#pragma once

#ifdef WIN32
    #include "bgapi2_ext/bgapi2_ext.h"
#else
    #include "bgapi2_genicam.hpp"
#endif

#include <string>
#include "BasicDef.h"

using namespace std;

class BCamera
{
public:
    BCamera(BGAPI2::Device * device);
    ~BCamera();

public:
    bool OpenCamera();
    bool GetTriggerMode(TriggerMode* model);
    bool SetTriggerMode(TriggerMode mode);
    bool GetIsMono8(bool* result);

    bool SetImageHandlerCallback(camera_image_handle_func func);
    bool StartCameraAcquisiton();

    //auto exps,gain,whiteblance
    bool GetAutoExposure(bool* enabled);
    bool GetAutoGain(bool* enabled);
    bool GetAutoWhite(bool* enabled);
    bool SetAutoExposure(const bool& enable);
    bool SetAutoGain(const bool& enable);
    bool SetAutoWhite(const bool& enable);

    //exps
    bool SetExposureTime(const double& exposureTime);
    bool GetExposureTime(double* exposureTime);
    bool GetExposureRange(double* min, double* max);

    //gain
    bool SetGain(const double& gain);
    bool GetGain(double* gain);
    bool GetGainRange(double* min, double* max);

    // gain
    bool SetGamma(const double& gamma);
    bool GetGamma(double* gamma);
    bool GetGammaRange(double* min, double* max);

    // Resolution
    bool GetCameraResolution(int* width, int* height);
    bool GetCameraResolutionRange(int* minWidth, int* maxWidth, int* minHeight, int* maxHeight);
    bool SetCameraResolution(const int& width, const int& height);

    //Set&Get camera output frame rate.
    bool GetOutputFrameRate(double *outputFrameRate) ;
    bool SetOutputFrameRate(const double &outputFrameRate) ;

    bool StopCameraAcquisiton();

    bool CloseCamera();

public:
    inline BGAPI2::Device* GetDevice()
    {
        return mDevice;
    }

    inline bool GetIsOpen()
    {
        return mIsOpen;
    }

private:
    bool handleImageEvent();
    bool unHandleImageEvent();

private:
    BGAPI2::Device* mDevice;
    BGAPI2::DataStream* mDataStream;
//    BGAPI2::Extended::AutoFunction* mAutoFunction;

    bool mIsOpen;
    bool mIsAcquisiton;
    bool mIsRegisterImageHandler;


public:
    BGAPI2::ImageProcessor* mImgProcessor;
    BGAPI2::Image * pImage;

public:
    camera_image_handle_func mImageHandlerFunc;
public:
    int64_t ImageNo;
    int64_t ImageTimestamp;
};

