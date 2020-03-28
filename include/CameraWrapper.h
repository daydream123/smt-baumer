#pragma once

#include "BasicDef.h"
#include "systems.h"

class BCamera;

typedef BCamera *SMART_CAMERA_LIB_DEV_HANDLE;

#ifdef __cplusplus
extern "C"
{
#endif
//1.
DECL_EXPORT ReturnCode InitSystem();

DECL_EXPORT ReturnCode GetIsSystemInit(bool *isInit);

DECL_EXPORT ReturnCode GetCameraCount(int *count);
DECL_EXPORT ReturnCode GetCameraDeviceIDByIndex(const unsigned int &index, char **deviceID);
DECL_EXPORT ReturnCode OpenCameraById(const char *deviceID, SMART_CAMERA_LIB_DEV_HANDLE *handle);
DECL_EXPORT ReturnCode OpenCameraByIndex(const int &index, SMART_CAMERA_LIB_DEV_HANDLE *handle);

DECL_EXPORT ReturnCode GetIsMono8(SMART_CAMERA_LIB_DEV_HANDLE handle, bool *result);

DECL_EXPORT ReturnCode LoadSettingFile(SMART_CAMERA_LIB_DEV_HANDLE handle, const char *fileName);
DECL_EXPORT ReturnCode SetSettingFile(SMART_CAMERA_LIB_DEV_HANDLE handle, const char *fileName);

DECL_EXPORT ReturnCode GetTriggerMode(SMART_CAMERA_LIB_DEV_HANDLE handle, TriggerMode *mode);
DECL_EXPORT ReturnCode SetTriggerMode(SMART_CAMERA_LIB_DEV_HANDLE handle, TriggerMode mode);

DECL_EXPORT ReturnCode SetImageHandlerCallback(SMART_CAMERA_LIB_DEV_HANDLE handle, camera_image_handle_func func);

DECL_EXPORT ReturnCode StartCameraAcquisiton(SMART_CAMERA_LIB_DEV_HANDLE handle);
DECL_EXPORT ReturnCode StopCameraAcquisiton(SMART_CAMERA_LIB_DEV_HANDLE handle);

DECL_EXPORT ReturnCode TakeOneShot(SMART_CAMERA_LIB_DEV_HANDLE handle);

//2.
//resolution
DECL_EXPORT ReturnCode GetCameraResolution(SMART_CAMERA_LIB_DEV_HANDLE handle, int *width, int *height);
DECL_EXPORT ReturnCode
GetCameraResolutionRange(SMART_CAMERA_LIB_DEV_HANDLE handle, int *minWidth, int *maxWidth, int *minHeight,
                         int *maxHeight);
DECL_EXPORT ReturnCode SetCameraResolution(SMART_CAMERA_LIB_DEV_HANDLE handle, const int &width, const int &height);

//expo gain game
DECL_EXPORT ReturnCode SetAutoExposure(SMART_CAMERA_LIB_DEV_HANDLE handle, const bool &enable);
DECL_EXPORT ReturnCode SetAutoGain(SMART_CAMERA_LIB_DEV_HANDLE handle, const bool &enable);
DECL_EXPORT ReturnCode SetAutoWhite(SMART_CAMERA_LIB_DEV_HANDLE handle, const bool &enable);
DECL_EXPORT ReturnCode GetAutoExposure(SMART_CAMERA_LIB_DEV_HANDLE handle, bool *enable);
DECL_EXPORT ReturnCode GetAutoGain(SMART_CAMERA_LIB_DEV_HANDLE handle, bool *enable);
DECL_EXPORT ReturnCode GetAutoWhite(SMART_CAMERA_LIB_DEV_HANDLE handle, bool *enable);

// exposure
DECL_EXPORT ReturnCode SetExposureTime(SMART_CAMERA_LIB_DEV_HANDLE handle, const double &exposureTime);
DECL_EXPORT ReturnCode GetExposureTime(SMART_CAMERA_LIB_DEV_HANDLE handle, double *exposureTime);
DECL_EXPORT ReturnCode GetExposureRange(SMART_CAMERA_LIB_DEV_HANDLE handle, double *min, double *max);

// gain
DECL_EXPORT ReturnCode SetGain(SMART_CAMERA_LIB_DEV_HANDLE handle, const double &gain);
DECL_EXPORT ReturnCode GetGain(SMART_CAMERA_LIB_DEV_HANDLE handle, double *gain);
DECL_EXPORT ReturnCode GetGainRange(SMART_CAMERA_LIB_DEV_HANDLE handle, double *min, double *max);

// gamma
DECL_EXPORT ReturnCode SetGamma(SMART_CAMERA_LIB_DEV_HANDLE handle, const double &gamma);
DECL_EXPORT ReturnCode GetGamma(SMART_CAMERA_LIB_DEV_HANDLE handle, double *gamma);
DECL_EXPORT ReturnCode GetGammaRange(SMART_CAMERA_LIB_DEV_HANDLE handle, double *min, double *max);

//white blance
DECL_EXPORT ReturnCode GetWhiteBalanceGainCount(SMART_CAMERA_LIB_DEV_HANDLE devPtr, int *count);
DECL_EXPORT ReturnCode
GetWhiteBalanceGainByIndex(SMART_CAMERA_LIB_DEV_HANDLE devPtr, const int &index, float *gain, char **name,
                           float *minGain, float *maxGain);
DECL_EXPORT ReturnCode
SetWhiteBalanceGainByIndex(SMART_CAMERA_LIB_DEV_HANDLE devPtr, const int &index, const float &gain);

//Set&Get camera output frame rate
DECL_EXPORT ReturnCode GetOutputFrameRate(SMART_CAMERA_LIB_DEV_HANDLE handle, double *outputFrameRate);
DECL_EXPORT ReturnCode SetOutputFrameRate(SMART_CAMERA_LIB_DEV_HANDLE handle, const double &outputFrameRate);

//3.
DECL_EXPORT ReturnCode CloseCamera(SMART_CAMERA_LIB_DEV_HANDLE handle);

//4.
DECL_EXPORT ReturnCode UnInitSystem();

#ifdef __cplusplus
}
#endif
