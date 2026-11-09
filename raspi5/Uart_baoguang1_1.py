#!/usr/bin/env python3
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import time
import cv2
import numpy as np
import serial

# ==================== 0. 🔌 串口配置 (树莓派 GPIO 硬件串口) ====================
ENABLE_SERIAL = True

# 树莓派 GPIO 硬件串口推荐使用 '/dev/serial0'
# 系统会自动将其指向正确的硬件引脚 (GPIO 14 TXD / GPIO 15 RXD)
SERIAL_PORT_MCU = '/dev/ttyUSB0'
SERIAL_PORT_STM = '/dev/ttyUSB1'
BAUD_RATE = 115200

ser_stm = None
def init_serial_mcu():
    """初始化并打开下位机串口"""
    global ser_stm
    if not ENABLE_SERIAL:
        return None
    try:
        ser = serial.Serial(SERIAL_PORT_MCU, BAUD_RATE, timeout=0.01)
        print(f"✅ 下位机 GPIO 串口打开成功: {SERIAL_PORT_MCU} | 波特率: {BAUD_RATE}")
        return ser
    except Exception as e:
        print(f"⚠️ 下位机串口打开失败 ({SERIAL_PORT_MCU}): {e}")
        print("💡 提示1：如果报 Permission denied，请在终端运行: sudo chmod 666 " + SERIAL_PORT_MCU)
        print("💡 提示2：请确认已在 sudo raspi-config 中关闭了 Serial Console 终端登录。")
        print("💡 提示3：若无需通信，可设置 ENABLE_SERIAL = False 仅进行本地视觉调试。")
        return None

ser_stm = init_serial_mcu()

# ==================== 1. ROI 与去畸变参数 ====================
USE_UNDISTORT = True

TRACK_ROI_Y_MIN = 175
TRACK_ROI_Y_MAX = 208
TRACK_ROI_X_MIN = 61
TRACK_ROI_X_MAX = 520

# 基础（原始）中心点
BASE_CENTER_X = 328
BASE_CENTER_Y = (TRACK_ROI_Y_MIN + TRACK_ROI_Y_MAX) // 2

CAMERA_MATRIX = np.array([
    [420.0765, 0.0, 335.9628],
    [0.0, 560.8205, 227.2602],
    [0.0, 0.0, 1.0]
], dtype=np.float32)

DIST_COEFFS = np.array([-0.4478, 0.1601, 0.0, 0.0, 0.0], dtype=np.float32)

# ==================== 2. 卡尔曼滤波 + 多帧校验 ====================
class KalmanSmoothTracker:
    def __init__(self):
        self.kf = cv2.KalmanFilter(4, 2)
        self.kf.transitionMatrix = np.array([
            [1, 0, 1, 0],
            [0, 1, 0, 1],
            [0, 0, 1, 0],
            [0, 0, 0, 1]
        ], np.float32)
        self.kf.measurementMatrix = np.array([
            [1, 0, 0, 0],
            [0, 1, 0, 0]
        ], np.float32)
        self.kf.processNoiseCov = np.array([
            [1e-2, 0,    0,    0],
            [0,    1e-2, 0,    0],
            [0,    0,    2e-2, 0],
            [0,    0,    0,    2e-2]
        ], np.float32)
        self.kf.measurementNoiseCov = np.array([
            [1e-1, 0],
            [0,    1e-1]
        ], np.float32)

        self.initialized = False
        self.prev_radius = None
        self.missing_count = 0

        # 多帧校验
        self.consecutive_hits = 0
        self.CONFIRM_THRESHOLD = 3
        self.confirmed = False

    def predict_position(self):
        if not self.initialized:
            return None
        pred = self.kf.predict()
        return (int(pred[0][0]), int(pred[1][0]))

    def update(self, raw_center, raw_radius, valid=True):
        if not valid:
            self.mark_missing()
            return (None, None), raw_radius, False

        meas_x, meas_y = raw_center

        if not self.initialized:
            self.kf.statePost = np.array([
                [np.float32(meas_x)],
                [np.float32(meas_y)],
                [0],
                [0]
            ], np.float32)
            self.initialized = True
            self.prev_radius = raw_radius
            self.consecutive_hits = 1
            self.missing_count = 0
            self.confirmed = False
            return (meas_x, meas_y), raw_radius, False

        self.missing_count = 0
        measurement = np.array([[np.float32(meas_x)], [np.float32(meas_y)]], np.float32)
        corrected = self.kf.correct(measurement)

        smooth_x = int(corrected[0][0])
        smooth_y = int(corrected[1][0])
        smooth_r = int(self.prev_radius * 0.7 + raw_radius * 0.3) if self.prev_radius else raw_radius
        self.prev_radius = smooth_r

        self.consecutive_hits += 1
        if self.consecutive_hits >= self.CONFIRM_THRESHOLD:
            self.confirmed = True

        return (smooth_x, smooth_y), smooth_r, self.confirmed

    def mark_missing(self):
        self.missing_count += 1
        
        # 连续丢失 3 帧立刻解除锁定，强制下一次循环回到全局全轨道 ROI 搜索
        if self.missing_count > 3:
            self.confirmed = False
            self.consecutive_hits = 0

        # 丢失超过 8 帧，彻底重置卡尔曼状态等待重新捕获
        if self.missing_count > 8:
            self.initialized = False
            self.prev_radius = None

    def reset(self):
        self.initialized = False
        self.prev_radius = None
        self.missing_count = 0
        self.consecutive_hits = 0
        self.confirmed = False

tracker = KalmanSmoothTracker()

# ==================== 3. 🎯 任务与状态机系统 ====================
TASK_CALIBRATE_CENTER     = 1  # 任务一：标定中心（固定正中心）
TASK_DYNAMIC_SEQUENCE     = 2  # 任务二：动态顺序切换（正中 -> 左 -> 右保持）
TASK_LOCK_CURRENT_POSITION = 3  # 任务三：锁定手动指定的目标点

current_task = TASK_CALIBRATE_CENTER

STATE_CENTER = 0  # 状态0：正中间
STATE_LEFT   = 1  # 状态1：左移目标点 (-95px)
STATE_RIGHT  = 2  # 状态2：右移目标点 (+83px)，到达后终点保持

current_sub_state = STATE_CENTER
stable_start_time = None
STABILITY_DISTANCE_THRESHOLD = 15  # 判定稳定像素距离

TIME_STABLE_CENTER = 0.5  # 正中间稳定时间(s)
TIME_STABLE_LEFT   = 0.1  # 左侧稳定时间(s)

# 手动指定的目标点（用于任务三）
manual_target_x = BASE_CENTER_X
manual_target_y = BASE_CENTER_Y
manual_target_set = False

task3_locked_target_x = BASE_CENTER_X
task3_locked_target_y = BASE_CENTER_Y

def reset_task_state():
    """切换任务时重置内部计时和子状态"""
    global current_sub_state, stable_start_time
    current_sub_state = STATE_CENTER
    stable_start_time = None

def get_target_center(task, sub_state):
    """根据当前任务和子状态算出目标坐标"""
    if task == TASK_CALIBRATE_CENTER:
        return BASE_CENTER_X, BASE_CENTER_Y
    elif task == TASK_DYNAMIC_SEQUENCE:
        if sub_state == STATE_CENTER:
            return BASE_CENTER_X, BASE_CENTER_Y
        elif sub_state == STATE_LEFT:
            return BASE_CENTER_X - 95, BASE_CENTER_Y
        elif sub_state == STATE_RIGHT:
            return BASE_CENTER_X + 86, BASE_CENTER_Y
    elif task == TASK_LOCK_CURRENT_POSITION:
        return task3_locked_target_x, task3_locked_target_y
    return BASE_CENTER_X, BASE_CENTER_Y

# ==================== 4. 摄像头初始化与设备号获取 ====================
camera_idx_used = 0

def init_linux_camera():
    global camera_idx_used
    for idx in [0, 1, 2, 3, 4]:
        cap = cv2.VideoCapture(idx, cv2.CAP_V4L2)
        if cap.isOpened():
            ret, _ = cap.read()
            if ret:
                print(f"✅ 成功打开摄像头 /dev/video{idx}")
                camera_idx_used = idx
                cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))
                cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
                cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
                cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
                return cap
            cap.release()
    print("❌ 未找到可用摄像头")
    sys.exit(1)

cap = init_linux_camera()
actual_w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
actual_h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
print(f"分辨率: {actual_w}x{actual_h}")

# ==================== 5. 去畸变映射 ====================
map1, map2 = None, None
if USE_UNDISTORT:
    new_camera_matrix, _ = cv2.getOptimalNewCameraMatrix(
        CAMERA_MATRIX, DIST_COEFFS, (actual_w, actual_h), 1, (actual_w, actual_h)
    )
    map1, map2 = cv2.initUndistortRectifyMap(
        CAMERA_MATRIX, DIST_COEFFS, None, new_camera_matrix, (actual_w, actual_h), cv2.CV_32FC1
    )
    print("✅ 畸变校正已启用")

# ==================== 6. LAB 与 摄像头参数调参窗口 ====================
TUNING_WIN = "LAB & Camera Tuning Controller"
cv2.namedWindow(TUNING_WIN, cv2.WINDOW_NORMAL)
cv2.resizeWindow(TUNING_WIN, 450, 500)

# LAB 阈值 Trackbars
cv2.createTrackbar("L Min", TUNING_WIN, 0, 255, lambda x: None)
cv2.createTrackbar("L Max", TUNING_WIN, 196, 255, lambda x: None)
cv2.createTrackbar("A Min", TUNING_WIN, 0, 255, lambda x: None)
cv2.createTrackbar("A Max", TUNING_WIN, 162, 255, lambda x: None)
cv2.createTrackbar("B Min", TUNING_WIN, 79, 255, lambda x: None)
cv2.createTrackbar("B Max", TUNING_WIN, 138, 255, lambda x: None)

# 增强型摄像头参数回调 (API + v4l2 双重保障)
def set_hardware_param(v4l2_cmd, prop_enum, val):
    """同时尝试 OpenCV API 和 Linux v4l2-ctl 底层设置，确保生效"""
    cap.set(prop_enum, val)
    dev_path = f"/dev/video{camera_idx_used}"
    os.system(f"v4l2-ctl -d {dev_path} -c {v4l2_cmd}={val} >/dev/null 2>&1")

def on_contrast_change(val):
    set_hardware_param("contrast", cv2.CAP_PROP_CONTRAST, val)

def on_auto_exposure_change(val):
    mode_v4l2 = 3 if val == 1 else 1
    set_hardware_param("auto_exposure", cv2.CAP_PROP_AUTO_EXPOSURE, mode_v4l2)

def on_exposure_change(val):
    dev_path = f"/dev/video{camera_idx_used}"
    os.system(f"v4l2-ctl -d {dev_path} -c auto_exposure=1 >/dev/null 2>&1")
    cap.set(cv2.CAP_PROP_AUTO_EXPOSURE, 1)
    set_hardware_param("exposure_time_absolute", cv2.CAP_PROP_EXPOSURE, val)

# 创建摄像头硬件调节 Trackbars
cv2.createTrackbar("Contrast", TUNING_WIN, 155, 255, on_contrast_change)
cv2.createTrackbar("Auto Exp (0:M 1:A)", TUNING_WIN, 0, 1, on_auto_exposure_change)
cv2.createTrackbar("Exposure", TUNING_WIN, 110, 2000, on_exposure_change)

# 强制初始化写入一次
on_auto_exposure_change(0)  # 默认关闭自动曝光
on_contrast_change(155)
on_exposure_change(110)

# ==================== 7. 检测函数（针对长方形全轨道 ROI 提取） ====================
def detect_steel_ball_roi(image):
    l_min = cv2.getTrackbarPos("L Min", TUNING_WIN)
    l_max = cv2.getTrackbarPos("L Max", TUNING_WIN)
    a_min = cv2.getTrackbarPos("A Min", TUNING_WIN)
    a_max = cv2.getTrackbarPos("A Max", TUNING_WIN)
    b_min = cv2.getTrackbarPos("B Min", TUNING_WIN)
    b_max = cv2.getTrackbarPos("B Max", TUNING_WIN)
    lower_steel = np.array([l_min, a_min, b_min])
    upper_steel = np.array([l_max, a_max, b_max])

    # 1. 仅截取【黄色长方形全轨道区域】
    track_roi_img = image[TRACK_ROI_Y_MIN:TRACK_ROI_Y_MAX, TRACK_ROI_X_MIN:TRACK_ROI_X_MAX]
    if track_roi_img.size == 0:
        tracker.mark_missing()
        return [], None, None, lower_steel, upper_steel, False

    # 2. 对长方形全轨道转换 LAB 并生成【全轨道 Mask】
    track_lab = cv2.cvtColor(track_roi_img, cv2.COLOR_BGR2LAB)
    track_mask = cv2.inRange(track_lab, lower_steel, upper_steel)

    # ------------------- 🛠️ 组合形态学处理 -------------------
    close_kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5, 5))
    track_mask = cv2.morphologyEx(track_mask, cv2.MORPH_CLOSE, close_kernel, iterations=1)

    open_kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (3, 3))
    track_mask = cv2.morphologyEx(track_mask, cv2.MORPH_OPEN, open_kernel, iterations=1)
    # ---------------------------------------------------------

    # 3. 确定当前检索的动态框范围 (基于绝对原点)
    if tracker.confirmed and tracker.initialized and tracker.missing_count <= 2:
        pred = tracker.predict_position()
        if pred is not None:
            px, py = pred
            roi_x1 = max(TRACK_ROI_X_MIN, px - 60)
            roi_x2 = min(TRACK_ROI_X_MAX, px + 60)
            roi_y1 = max(TRACK_ROI_Y_MIN, py - 60)
            roi_y2 = min(TRACK_ROI_Y_MAX, py + 60)
        else:
            roi_x1, roi_x2 = TRACK_ROI_X_MIN, TRACK_ROI_X_MAX
            roi_y1, roi_y2 = TRACK_ROI_Y_MIN, TRACK_ROI_Y_MAX
    else:
        roi_x1, roi_x2 = TRACK_ROI_X_MIN, TRACK_ROI_X_MAX
        roi_y1, roi_y2 = TRACK_ROI_Y_MIN, TRACK_ROI_Y_MAX

    # 4. 从【全轨道 Mask】中截取【动态紫色框的局部 Mask】
    rel_x1 = roi_x1 - TRACK_ROI_X_MIN
    rel_x2 = roi_x2 - TRACK_ROI_X_MIN
    rel_y1 = roi_y1 - TRACK_ROI_Y_MIN
    rel_y2 = roi_y2 - TRACK_ROI_Y_MIN

    local_roi_mask = track_mask[rel_y1:rel_y2, rel_x1:rel_x2]

    if local_roi_mask.size == 0:
        tracker.mark_missing()
        return [], track_mask, local_roi_mask, lower_steel, upper_steel, False

    # 5. 在局部 Mask 中找轮廓
    contours, _ = cv2.findContours(local_roi_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    candidates = []
    for cnt in contours:
        if len(cnt) < 5:
            continue
        (cx, cy), radius = cv2.minEnclosingCircle(cnt)
        cx, cy, radius = int(cx), int(cy), int(radius)
        x, y, w, h = cv2.boundingRect(cnt)
        aspect_ratio = float(w) / h
        if aspect_ratio < 0.5 or aspect_ratio > 2.0:
            continue
        if radius < 8 or radius > 25:
            continue
            
        # 还原回全图绝对坐标
        abs_cx = cx + roi_x1
        abs_cy = cy + roi_y1
        area = np.pi * (radius ** 2)
        candidates.append(((abs_cx, abs_cy), radius, area))

    if not candidates:
        tracker.mark_missing()
        return [], track_mask, local_roi_mask, lower_steel, upper_steel, False

    # 候选目标筛选
    pred_pos = tracker.predict_position()
    if tracker.confirmed and tracker.initialized and pred_pos is not None:
        candidates.sort(key=lambda c: np.hypot(c[0][0] - pred_pos[0], c[0][1] - pred_pos[1]))
        best_center, best_radius, _ = candidates[0]
        dist = np.hypot(best_center[0] - pred_pos[0], best_center[1] - pred_pos[1])
        
        max_dist = 50 + tracker.missing_count * 15
        if dist > max_dist:
            tracker.mark_missing()
            return [], track_mask, local_roi_mask, lower_steel, upper_steel, False
    else:
        candidates.sort(key=lambda c: c[2], reverse=True)
        best_center, best_radius, _ = candidates[0]

    smooth_center, smooth_radius, confirmed = tracker.update(best_center, best_radius)
    if confirmed:
        return [{"center": smooth_center, "radius": smooth_radius}], track_mask, local_roi_mask, lower_steel, upper_steel, True
    else:
        return [{"center": smooth_center, "radius": smooth_radius}], track_mask, local_roi_mask, lower_steel, upper_steel, False

# ==================== 鼠标回调函数（用于手动指定目标点） ====================
def mouse_callback(event, x, y, flags, param):
    global manual_target_x, manual_target_y, manual_target_set
    if event == cv2.EVENT_LBUTTONDOWN:
        manual_target_x = x
        manual_target_y = y
        manual_target_set = True
        print(f"🖱️ 手动设定目标点: ({x}, {y})")

# ==================== 8. 主循环 ====================
prev_time = time.time()
frame_cnt = 0
real_fps = 0.0

cv2.namedWindow("Steel Ball Detector Window", cv2.WINDOW_NORMAL)
cv2.setMouseCallback("Steel Ball Detector Window", mouse_callback)

print("🚀 系统启动成功！")
print(f"📡 已关联树莓派 GPIO 通信串口: {SERIAL_PORT_MCU}")
print("📌 按键盘 ['1'] 键: 切换到【任务一：标定中心】")
print("📌 按键盘 ['2'] 键: 切换到【任务二：动态单向跟踪 (正中 -> 左 -> 右保持)】")
print("📌 按键盘 ['3'] 键: 切换到【任务三：使用鼠标指定的目标点】")
print("    （请先在图像窗口用鼠标左键点击任意位置设定目标点）")
print("📌 按键盘 ['q'] 键: 退出程序")
ball_x=0
ball_y=0
try: 
    while True:

        ret, frame = cap.read()
        if not ret:
            continue

        if USE_UNDISTORT and map1 is not None:
            processed_frame = cv2.remap(frame, map1, map2, cv2.INTER_LINEAR)
        else:
            processed_frame = frame

        # ==================== 📥 串口接收下位机指令 (GPIO 硬件串口) ====================
        if ENABLE_SERIAL:
            if ser_stm is None or not ser_stm.is_open:
                # 尝试自动重新连接串口
                ser_stm = init_serial_mcu()
            
            if ser_stm is not None and ser_stm.is_open:
                try:
                    if ser_stm.in_waiting > 0:
                        raw_data = ser_stm.read(ser_stm.in_waiting)
                        cmd_str = raw_data.decode('utf-8', errors='ignore').strip()
                        
                        # 下位机 TASK_MODE_2 -> Python 任务二
                        if "2" in cmd_str:
                            if current_task != TASK_DYNAMIC_SEQUENCE:
                                current_task = TASK_DYNAMIC_SEQUENCE
                            reset_task_state()
                            print("📡 [GPIO串口] 收到下位机指令 -> 切换至【任务二：动态顺序跟踪】")

                        # 下位机 TASK_MODE_3 / TASK_MODE_4 -> Python 任务一
                        elif "3" in cmd_str or "4" in cmd_str:
                            if current_task != TASK_CALIBRATE_CENTER:
                                current_task = TASK_CALIBRATE_CENTER
                            reset_task_state()
                            print("📡 [GPIO串口] 收到下位机指令 -> 切换至【任务一：标定中心】")

                        # 下位机 TASK_MODE_5 -> Python 任务三
                        elif "5" in cmd_str:
                            if current_task != TASK_LOCK_CURRENT_POSITION:
                                current_task = TASK_LOCK_CURRENT_POSITION
                            reset_task_state()
                            if not manual_target_set:
                                task3_locked_target_x = BASE_CENTER_X
                                task3_locked_target_y = BASE_CENTER_Y
                            else:
                                task3_locked_target_x = manual_target_x
                                task3_locked_target_y = manual_target_y

                            #send_str = f"#b999 999 999!\n"
                            # if ser_stm is not None and ser_stm.is_open:
                            #     try:
                            #         ser_stm.write(send_str.encode('utf-8'))
                            #     except Exception as e:
                            #         print(f"⚠️ 串口发送异常: {e}")
                            print("📡 [GPIO串口] 收到下位机指令 -> 切换至【任务三：锁定目标点】")
                        elif "9" in cmd_str:
                            if current_task == TASK_LOCK_CURRENT_POSITION:
                                # send_str = f"#b1999 1999 1999!\n"
                                # if ser_stm is not None and ser_stm.is_open:
                                #     try:
                                #         ser_stm.write(send_str.encode('utf-8'))
                                #     except Exception as e:
                                #         print(f"⚠️ 串口发送异常: {e}")
                                task3_locked_target_x = ball_x
                                task3_locked_target_y = ball_y
                        elif "8" in cmd_str:
                            BASE_CENTER_X = ball_x
                                
                                

                except Exception as e:
                    print(f"⚠️ 串口读取异常，重置串口状态: {e}")
                    try:
                        ser_stm.close()
                    except:
                        pass
                    ser_stm = None

        # FPS 统计
        frame_cnt += 1
        curr_time = time.time()
        if curr_time - prev_time >= 1.0:
            real_fps = frame_cnt / (curr_time - prev_time)
            frame_cnt = 0
            prev_time = curr_time

        balls, track_mask, local_roi_mask, lower_s, upper_s, valid = detect_steel_ball_roi(processed_frame)
        disp_frame = processed_frame.copy()

        # -------------------- 逻辑控制：任务二 --------------------
        if current_task == TASK_DYNAMIC_SEQUENCE:
            target_center_x, target_center_y = get_target_center(current_task, current_sub_state)
            if valid and balls:
                cx, cy = balls[0]["center"]
                dist_to_target = np.hypot(cx - target_center_x, cy - target_center_y)

                if dist_to_target < STABILITY_DISTANCE_THRESHOLD:
                    if stable_start_time is None:
                        stable_start_time = time.time()
                    else:
                        duration = time.time() - stable_start_time
                        
                        # 状态 0 (正中间) 稳定 -> 切到 状态 1 (左侧 -95px)
                        if current_sub_state == STATE_CENTER and duration >= TIME_STABLE_CENTER:
                            current_sub_state = STATE_LEFT
                            stable_start_time = None
                            print(f"⚡ [任务二] 切至【左侧 (-95px)】 (正中间稳定 {TIME_STABLE_CENTER}s)")
                        
                        # 状态 1 (左侧) 稳定 -> 切到 状态 2 (右侧 +83px)
                        elif current_sub_state == STATE_LEFT and duration >= TIME_STABLE_LEFT:
                            current_sub_state = STATE_RIGHT
                            stable_start_time = None
                            print(f"⚡ [任务二] 切至【右侧 (+83px)】 (左侧稳定 {TIME_STABLE_LEFT}s，保持在此位置)")

                        # 状态 2 (右侧) 稳定后保持在此状态，不再切回
                        elif current_sub_state == STATE_RIGHT:
                            stable_start_time = None
                else:
                    stable_start_time = None
            else:
                stable_start_time = None

        # -------------------- 获取当前真正的目标点坐标 --------------------
        target_center_x, target_center_y = get_target_center(current_task, current_sub_state)

        # 1. 绘制全轨道 ROI（黄色框）
        cv2.rectangle(disp_frame, (TRACK_ROI_X_MIN, TRACK_ROI_Y_MIN),
                      (TRACK_ROI_X_MAX, TRACK_ROI_Y_MAX), (0, 255, 255), 1)

        # 2. 绘制当前目标中心十字（绿色）
        cv2.line(disp_frame, (target_center_x - 12, target_center_y), (target_center_x + 12, target_center_y), (0, 255, 0), 2)
        cv2.line(disp_frame, (target_center_x, target_center_y - 12), (target_center_x, target_center_y + 12), (0, 255, 0), 2)
        cv2.putText(disp_frame, f"Target ({target_center_x},{target_center_y})", (target_center_x - 40, target_center_y + 20),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 255, 0), 1)

        # 如果当前是任务三且手动设定过，显示 "Manual" 标记
        if current_task == TASK_LOCK_CURRENT_POSITION and manual_target_set:
            cv2.putText(disp_frame, "Manual", (target_center_x - 20, target_center_y - 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)

        # 3. 绘制动态搜索窗口（紫色框，锁定且未丢帧时显示）
        if tracker.confirmed and tracker.initialized and tracker.missing_count <= 2:
            pred = tracker.predict_position()
            if pred is not None:
                px, py = pred
                cv2.rectangle(disp_frame, (px - 60, py - 60), (px + 60, py + 60), (255, 0, 255), 2)
                cv2.putText(disp_frame, "Dynamic Window", (px - 50, py - 65),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 0, 255), 1)

        # 4. 📤 发送数据给下位机 (GPIO 硬件串口) 与绘制识别结果

        if valid and balls:
            for ball in balls:
                cx, cy = ball["center"]
                ball_x = cx
                ball_y = cy
                r = ball["radius"]
                
                offset_x = cx - target_center_x
                offset_y = cy - target_center_y
                
                send_str = f"#b{offset_x} {offset_y} {cx}!\n"
                if ser_stm is not None and ser_stm.is_open:
                    try:
                        ser_stm.write(send_str.encode('utf-8'))
                    except Exception as e:
                        print(f"⚠️ 串口发送异常: {e}")

                cv2.circle(disp_frame, (cx, cy), r, (0, 255, 0), 2)
                cv2.circle(disp_frame, (cx, cy), 2, (0, 0, 255), 3)
                cv2.line(disp_frame, (target_center_x, target_center_y), (cx, cy), (255, 0, 0), 1)
                cv2.putText(disp_frame, f"Abs:({cx},{cy})", (cx - 30, cy - r - 20),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.45, (0, 255, 0), 1)
                cv2.putText(disp_frame, f"dTarget:({offset_x:+d},{offset_y:+d})", (cx - 30, cy - r - 5),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.45, (0, 255, 255), 1)

        # -------------------- UI 面板展示 --------------------
        if current_task == TASK_CALIBRATE_CENTER:
            task_str = "TASK 1: Calibrate Base Center"
            sub_info = "Fixed Base Center"
        elif current_task == TASK_DYNAMIC_SEQUENCE:
            task_str = "TASK 2: Dynamic Sequence"
            sub_names = ["0: CENTER", "1: LEFT (-95px)", "2: RIGHT (+83px, Final)"]
            time_text = f" | Stable: {time.time() - stable_start_time:.1f}s" if stable_start_time else ""
            sub_info = f"SubState: {sub_names[current_sub_state]}{time_text}"
        elif current_task == TASK_LOCK_CURRENT_POSITION:
            task_str = "TASK 3: Manual Target"
            if manual_target_set:
                sub_info = f"Manual Point: ({task3_locked_target_x}, {task3_locked_target_y})"
            else:
                sub_info = "No manual point set (using base center)"

        cv2.putText(disp_frame, f"[{task_str}]", (20, 65),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.65, (0, 255, 255), 2)
        cv2.putText(disp_frame, sub_info, (20, 90),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.55, (255, 255, 0), 1)

        # 显示阈值和 FPS
        param_text = f"L:[{lower_s[0]},{upper_s[0]}] A:[{lower_s[1]},{upper_s[1]}] B:[{lower_s[2]},{upper_s[2]}]"
        cv2.putText(disp_frame, param_text, (10, actual_h - 15),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 1)
        cv2.putText(disp_frame, f"FPS: {real_fps:.1f} | Count: {len(balls)}", (20, 35),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)

        # 主 RGB 画面显示
        cv2.imshow("Steel Ball Detector Window", disp_frame)

        # 显示轨道长方形 Mask 和 动态框 Mask
        if track_mask is not None:
            cv2.imshow("Track ROI LAB Mask Debug", track_mask)
        if local_roi_mask is not None and local_roi_mask.size > 0:
            cv2.imshow("Dynamic Local LAB Mask Debug", local_roi_mask)

        # -------------------- 键盘交互 (保留键盘调试) --------------------
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        elif key == ord('1'):
            current_task = TASK_CALIBRATE_CENTER
            reset_task_state()
            print("🔄 键盘切换 -> 【任务一：标定中心】")
        elif key == ord('2'):
            current_task = TASK_DYNAMIC_SEQUENCE
            reset_task_state()
            print("🔄 键盘切换 -> 【任务二：动态顺序跟踪】")
        elif key == ord('3'):
            current_task = TASK_LOCK_CURRENT_POSITION
            reset_task_state()
            if manual_target_set:
                task3_locked_target_x = manual_target_x
                task3_locked_target_y = manual_target_y
                print(f"🔒 键盘切换 -> 【任务三】使用手动指定目标: ({task3_locked_target_x}, {task3_locked_target_y})")
            else:
                task3_locked_target_x = BASE_CENTER_X
                task3_locked_target_y = BASE_CENTER_Y
                print("⚠️ 尚未手动设定目标点，请先在图像窗口左键点击设定目标点。")
                print(f"🔒 键盘切换 -> 【任务三】使用默认基准点: ({task3_locked_target_x}, {task3_locked_target_y})")
        elif key == ord('c'):
            manual_target_set = False
            print("🧹 已清除手动目标点，恢复到基准中心。")

except KeyboardInterrupt:
    pass

finally:
    cap.release()
    if ser_stm and ser_stm.is_open:
        ser_stm.close()
    if ser_stm and ser_stm.is_open:
        ser_stm.close()
    cv2.destroyAllWindows()
    print("✅ 资源已释放")