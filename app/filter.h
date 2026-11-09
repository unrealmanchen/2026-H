#ifndef FILTER
#define FILTER



#ifdef __cplusplus
extern "C" {
#endif
typedef void * Filter;
Filter LowPassFilter_Create_A(float a);
float LowPassFilter_Process(void* handle, float value);


#ifdef __cplusplus 
}
#endif


#endif
