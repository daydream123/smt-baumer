#pragma once

#include <iostream>

//#define LOG_TAG "Camera_Baumer"

enum class ReturnCode
{
    SUCCESS,
    FAIL,
    SYSTEM_NOT_INIT,
    FAIL_INITIALIZE_SYSTEMLIST,
    FAIL_FOUND_SYSTEM,
    FAIL_INITIALIZE_INTERFACELIST,
    FAIL_FOUND_INTERFACE,
    FAIL_INITIALIZE_DEVICELIST,
    FAIL_FOUND_DEVICE,
    RESOURCE_IN_USE,
    INDEX_OUT_OF_RANGE,
    DEVICEID_INVALID,
    PARAMETER_NULLPTR,
    FAIL_UNINIT,
    NOT_IMPLEMENTED,
    UNKNOWN_ERROR,
    FUNC_NOT_EXIST
};

enum class TriggerMode
{
    SOFTWARE,
    HARDWARE,
    OFF
};
#ifdef WIN32
typedef void (_stdcall *camera_image_handle_func)(unsigned char* pixData, int dataSize, int width, int height, quint64 imageNo, qint64 timeStamp);
#else
typedef void ( *camera_image_handle_func)(unsigned char* pixData, int dataSize, int width, int height, int64_t imageNo, int64_t timeStamp);
#endif


