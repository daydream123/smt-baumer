#include "BCamera.h"
#include <smt-logger/smt_logger.h>
#include "time.h"

using namespace BGAPI2;

const int DATA_STREAM_BUFFER_COUNT = 4;

void NewBufferEventHandler(void* callbackOwner, BGAPI2::Buffer* pBufferFilled);

void loggerException(Exceptions::IException& ex)
{
    LOGERROR("ExceptionType: %s", (const char*)ex.GetType());
    LOGERROR("ErrorDescription: %s", (const char*)ex.GetErrorDescription());
    LOGERROR("in function: %s", (const char*)ex.GetFunctionName());
}

BCamera::BCamera(BGAPI2::Device* device)
    : mDevice(device),
      mDataStream(nullptr),
      mIsAcquisiton(false),
      mIsOpen(false),
      mIsRegisterImageHandler(false),
      mImageHandlerFunc(nullptr),
      //mAutoFunction(nullptr),
      ImageNo(0),
      ImageTimestamp(0),
      pImage(nullptr)
{
    LOGINFO("Camera structure:");
    try
    {
        mImgProcessor = new BGAPI2::ImageProcessor();
        LOGINFO("Camera structure Success");
    }
    catch (BGAPI2::Exceptions::IException & ex)
    {
        LOGERROR("Create mImgProcessor Failed");
        loggerException(ex);
    }
}

BCamera::~BCamera()
{
    CloseCamera();
    if (mImgProcessor)
    {
        delete mImgProcessor;
    }
}

bool BCamera::OpenCamera()
{
    LOGINFO("OpenCamera:");
    if (nullptr == mDevice)
    {
        return false;
    }
    try
    {
        if (mIsOpen)
        {
            return true;
        }
        LOGINFO("Start mDevice Open", (char*)mDevice->GetDisplayName());
        mDevice->Open();
        mDevice->GetRemoteNode(SFNC_ACQUISITION_STOP)->Execute();
        LOGINFO("mDevice:%s open Success", (char*)mDevice->GetDisplayName());
        //TODO open data stream (maybe one device suport multiple datastream so that can open multiple application at the same time)
        LOGINFO("Prepare to init mDataStream");
        DataStreamList* pDataStreamList = mDevice->GetDataStreams();
        pDataStreamList->Refresh();
        if (pDataStreamList->size() > 0)
        {
            mDataStream = pDataStreamList->begin()->second;
            if (!mDataStream->IsOpen())
            {
                mDataStream->Open();
                //TODO Prepare DataStream Buffer
                LOGINFO("Prepare to fill BufferList to %d", DATA_STREAM_BUFFER_COUNT);
                BufferList* pBufferList = mDataStream->GetBufferList();
                if (pBufferList->size() > DATA_STREAM_BUFFER_COUNT)
                {
                    // revoke redundant image buffer
                    pBufferList->DiscardAllBuffers();
                    while (pBufferList->size() > DATA_STREAM_BUFFER_COUNT)
                    {
                        BGAPI2::Buffer* pBuffer = pBufferList->begin()->second;
                        pBufferList->RevokeBuffer(pBuffer);
                        delete pBuffer;
                    }
                }
                else if (pBufferList->size() < DATA_STREAM_BUFFER_COUNT)
                {
                    // add more image buffer if not engouth
                    int num = 0;
                    while (pBufferList->size() < DATA_STREAM_BUFFER_COUNT)
                    {
                        pBufferList->Add(new BGAPI2::Buffer());
                    }
                }
                LOGINFO("Fill BufferList to %d Success", DATA_STREAM_BUFFER_COUNT);
                for (BufferList::iterator bufIterator = pBufferList->begin(); bufIterator != pBufferList->end(); bufIterator++)
                {
                    bufIterator->second->QueueBuffer();
                }
                LOGINFO("Init mDataStream Success");
                //                mAutoFunction = new BGAPI2::Extended::AutoFunction(mDevice);
                mIsOpen = true;
                LOGINFO("OpenCamera Success");
                return true;
            }
        }
        else
        {
            LOGERROR("pDataStreamList->size() can not less than zero");
        }
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("OpenCamera Failed");
        loggerException(ex);
        return false;
    }
    return false;
}

bool BCamera::GetTriggerMode(TriggerMode* model)
{
    LOGINFO("GetTriggerMode:");
    if (model == nullptr)
    {
        LOGERROR("Parameter is Null");
        return false;
    }
    if (nullptr == mDevice)
    {
        LOGERROR("Device is Null");
        return false;
    }
    if (mIsOpen != true)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    try
    {
        BGAPI2::String strTriggerMode = mDevice->GetRemoteNode(SFNC_TRIGGERMODE)->GetValue();
        if (strTriggerMode == "On")
        {
            BGAPI2::String strTriggerSource = mDevice->GetRemoteNode(SFNC_TRIGGERSOURCE)->GetValue();
            if (strTriggerSource =="Software")
            {
                *model = TriggerMode::SOFTWARE;
                LOGINFO("GetTriggerMode:%s", "TriggerMode::SOFTWARE");
            }
            else
            {
                *model = TriggerMode::HARDWARE;
                LOGINFO("GetTriggerMode:%s", "TriggerMode::HARDWARE");
            }
        }
        else if (strTriggerMode == "Off")
        {
            *model = TriggerMode::OFF;
            LOGINFO("GetTriggerMode:%s", "TriggerMode::OFF");
        }
        LOGINFO("GetTriggerMode Success");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("GetTriggerMode Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::SetTriggerMode(TriggerMode mode)
{
    LOGINFO("SetTriggerMode:");
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    TriggerMode curMode;
    bool result	= GetTriggerMode(&curMode);
    if (!result)
    {
        LOGERROR("GetTriggerMode Failed");
        return false;
    }
    if (curMode == mode)
    {
        LOGINFO("The trigger mode(%d) to set is current TriggerMode", (int)mode);
        return true;
    }
    try
    {
        switch (mode)
        {
        case TriggerMode::SOFTWARE:
            mDevice->GetRemoteNode(SFNC_TRIGGERSOURCE)->SetValue("Software");
            mDevice->GetRemoteNode(SFNC_TRIGGERMODE)->SetValue("On");
            LOGINFO("SetTriggerMode triggersource:%s triggermode:%s", "Software", "On");
            break;
        case TriggerMode::HARDWARE:
            mDevice->GetRemoteNode(SFNC_TRIGGERSOURCE)->SetValue("Line0");
            mDevice->GetRemoteNode(SFNC_TRIGGERMODE)->SetValue("On");
            LOGINFO("SetTriggerMode triggersource:%s triggermode:%s", "Line0", "On");
            break;
        case TriggerMode::OFF:
            mDevice->GetRemoteNode(SFNC_TRIGGERMODE)->SetValue("Off");
            LOGINFO("SetTriggerMode triggermode:%s", "Off");
            break;
        default:
            LOGERROR("Target mode invalid");
            return false;
            break;
        }
        LOGINFO("SetTriggerMode Success");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("SetTriggerMode Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::GetIsMono8(bool* result)
{
    LOGINFO("GetIsMono8:");
    if (result == nullptr)
    {
        LOGERROR("Parameter is Null");
        return false;
    }
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    try
    {
        BGAPI2::String format = mDevice->GetRemoteNode(SFNC_PIXELFORMAT)->GetValue();
        *result = (format == "Mono8");
        if (result)
        {
            LOGINFO("GetIsMono8: true");
        }
        else
        {
            LOGINFO("GetIsMono8: false");
        }
        LOGINFO("GetIsMono8 Success");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("GetIsMono8 Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::SetImageHandlerCallback(camera_image_handle_func func)
{
    LOGINFO("SetImageHandlerCallback:");
    mImageHandlerFunc = func;
    LOGINFO("SetImageHandlerCallback Success");
    return true;
}

bool BCamera::StartCameraAcquisiton()
{
    LOGINFO("StartCameraAcquisiton:");
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (mIsAcquisiton && !StopCameraAcquisiton())
    {
        LOGERROR("File to StopCameraAcquisiton");
        return false;
    }
    try
    {
        if (!handleImageEvent())
        {
            LOGERROR("File to handleImageEvent");
            return false;
        }

        mDataStream->StartAcquisitionContinuous();
        if (!SetTriggerMode(TriggerMode::OFF))
        {
            LOGERROR("File to SetTriggerMode to off");
            return false;
        }
        Node* node = mDevice->GetRemoteNode(SFNC_ACQUISITION_START);
        if (node->IsWriteable())
        {
            node->Execute();
            mIsAcquisiton = true;
            LOGINFO("StartCameraAcquisiton Success");
            return true;
        }
        else
        {
            LOGERROR("SFNC_ACQUISITION_START is not Writeable");
            return false;
        }
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("StartCameraAcquisiton Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::GetCameraResolution(int* width, int* height)
{
    LOGINFO("GetCameraResolution:");
    if (width == nullptr || height == nullptr)
    {
        LOGERROR("Parameter is Null");
        return false;
    }
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    try
    {
        *width = mDevice->GetRemoteNode(SFNC_WIDTH)->GetInt();
        *height = mDevice->GetRemoteNode(SFNC_HEIGHT)->GetInt();
        LOGINFO("GetCameraResolution width:%d height:%d", *width, *height);
        LOGINFO("GetCameraResolution Success");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("GetCameraResolution Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::GetCameraResolutionRange(int* minWidth, int* maxWidth, int* minHeight, int* maxHeight)
{
    LOGINFO("GetCameraResolutionRange:");
    if (nullptr == minWidth || nullptr == maxWidth || nullptr == minHeight || nullptr == maxHeight)
    {
        LOGERROR("Parameter is Null");
        return false;
    }
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    try
    {
        *minWidth = mDevice->GetRemoteNode(SFNC_WIDTH)->GetIntMin();
        *minHeight = mDevice->GetRemoteNode(SFNC_HEIGHT)->GetIntMin();
        *maxWidth = mDevice->GetRemoteNode(SFNC_WIDTH)->GetIntMax();
        *maxHeight = mDevice->GetRemoteNode(SFNC_HEIGHT)->GetIntMax();
        LOGINFO("GetCameraResolutionRange minWidth:%d minHeight:%d maxWidth:%d maxHeight:%d", *minWidth, *minHeight, *maxWidth, *maxHeight);
        LOGINFO("GetCameraResolutionRange Success");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("GetCameraResolutionRange Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::SetCameraResolution(const int& width, const int& height)
{
    LOGINFO("SetCameraResolution:");
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    try
    {
        if (mDevice->GetRemoteNode(SFNC_WIDTH)->IsWriteable())
        {
            if (mDevice->GetRemoteNode(SFNC_HEIGHT)->IsWriteable())
            {
                mDevice->GetRemoteNode(SFNC_WIDTH)->SetInt(width);
                mDevice->GetRemoteNode(SFNC_HEIGHT)->SetInt(height);
                LOGINFO("SetCameraResolution width:%d height:%d", width, height);
                LOGINFO("SetCameraResolution Success");
                return true;
            }
        }
        else
        {
            LOGERROR("SFNC_WIDTH is not Writeable");
            return false;
        }
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("SetCameraResolution Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::GetAutoExposure(bool* enabled)
{
    //    LOGINFO("GetAutoExposure:");
    //    if (enabled == nullptr)
    //    {
    //        LOGERROR("Parameter is Null");
    //        return false;
    //    }
    //    if (nullptr == mDevice)
    //    {
    //        LOGERROR("mDevice is Null");
    //        return false;
    //    }
    //    if (false == mIsOpen)
    //    {
    //        LOGERROR("Camera is not open");
    //        return false;
    //    }
    //    if (mAutoFunction == nullptr)
    //    {
    //        LOGERROR("mAutoFunction is Null");
    //        return false;
    //    }
    //    try
    //    {
    //        *enabled = RUN_MODE_CONTINUE == mAutoFunction->GetAutoExposureRun();
    //        LOGINFO("GetAutoExposure AutoExposure:%s", enabled ? "true" : "false");
    //        LOGINFO("GetAutoExposure Success");
    //        return true;
    //    }
    //    catch (Exceptions::IException & ex)
    //    {
    //        LOGERROR("GetAutoExposure Failed");
    //        loggerException(ex);
    //        return false;
    //    }
    return false;
}

bool BCamera::GetAutoGain(bool* enabled)
{
    //    LOGINFO("GetAutoGain:");
    //    if (enabled == nullptr)
    //    {
    //        LOGERROR("Parameter is Null");
    //        return false;
    //    }
    //    if (nullptr == mDevice)
    //    {
    //        LOGERROR("mDevice is Null");
    //        return false;
    //    }
    //    if (false == mIsOpen)
    //    {
    //        LOGERROR("Camera is not open");
    //        return false;
    //    }
    //    if (mAutoFunction == nullptr)
    //    {
    //        LOGERROR("mAutoFunction is Null");
    //        return false;
    //    }
    //    try
    //    {
    //        *enabled = RUN_MODE_CONTINUE == mAutoFunction->GetAutoGainRun();
    //        LOGINFO("GetAutoGain AutoGain:%s", enabled ? "true" : "false");
    //        LOGINFO("GetAutoGain Success");
    //        return true;
    //    }
    //    catch (Exceptions::IException & ex)
    //    {
    //        LOGERROR("GetAutoGain Failed");
    //        loggerException(ex);
    //        return false;
    //    }
    return false;
}

bool BCamera::GetAutoWhite(bool* enabled)
{
    LOGINFO("GetAutoWhite:");
    if (enabled == nullptr)
    {
        LOGERROR("Parameter is Null");
        return false;
    }

    if (mDevice == nullptr)
    {
        LOGERROR("mDevice is Null");
        return false;
    }

    if (mIsOpen == false)
    {
        LOGERROR("Camera is not open");
        return false;
    }

    try
    {
        if (!mDevice->GetRemoteNode(SFNC_BALANCEWHITEAUTO)->IsReadable())
        {
            LOGERROR("SFNC_BALANCEWHITEAUTO is not Readable");
            return false;
        }

        // options: Continuous|Once|Off
        *enabled = mDevice->GetRemoteNode(SFNC_BALANCEWHITEAUTO)->GetString() == "Continuous";
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("read SFNC_BALANCEWHITEAUTO Failed");
        loggerException(ex);
        return false;
    }

    return true;
}

bool BCamera::SetAutoExposure(const bool& enable)
{
    //    LOGINFO("SetAutoExposure:");
    //    if (nullptr == mDevice)
    //    {
    //        LOGERROR("mDevice is Null");
    //        return false;
    //    }
    //    if (false == mIsOpen)
    //    {
    //        LOGERROR("Camera is not open");
    //        return false;
    //    }
    //    if (mAutoFunction == nullptr)
    //    {
    //        LOGERROR("mAutoFunction is Null");
    //        return false;
    //    }
    //    try
    //    {
    //        if (enable == (RUN_MODE_CONTINUE == mAutoFunction->GetAutoExposureRun()))
    //        {
    //            LOGINFO("The AutoExposure(%d) to set is current AutoExposure", (int)enable);
    //            return true;
    //        }

    //        mAutoFunction->SetAutoExposureRun(enable ? RUN_MODE_CONTINUE : RUN_MODE_OFF);
    //        LOGINFO("SetAutoExposure AutoExposure:%s", enable ? "true" : "false");
    //        LOGINFO("SetAutoExposure Success");
    //        return true;
    //    }
    //    catch (Exceptions::IException & ex)
    //    {
    //        LOGERROR("SetAutoExposure Failed");
    //        loggerException(ex);
    //        return false;
    //    }
    return false;
}

bool BCamera::SetAutoGain(const bool& enable)
{
    //    LOGINFO("SetAutoGain:");
    //    if (nullptr == mDevice)
    //    {
    //        LOGERROR("mDevice is Null");
    //        return false;
    //    }
    //    if (false == mIsOpen)
    //    {
    //        LOGERROR("Camera is not open");
    //        return false;
    //    }
    //    if (mAutoFunction == nullptr)
    //    {
    //        LOGERROR("mAutoFunction is Null");
    //        return false;
    //    }
    //    try
    //    {
    //        if (enable == (RUN_MODE_CONTINUE == mAutoFunction->GetAutoGainRun()))
    //        {
    //            LOGINFO("The AutoGain(%d) to set is current AutoGain", (int)enable);
    //            return true;
    //        }

    //        mAutoFunction->SetAutoGainRun(enable ? RUN_MODE_CONTINUE : RUN_MODE_OFF);
    //        LOGINFO("SetAutoGain AutoGain:%s", enable ? "true" : "false");
    //        LOGINFO("SetAutoGain Success");
    //        return true;
    //    }
    //    catch (Exceptions::IException & ex)
    //    {
    //        LOGERROR("SetAutoGain Failed");
    //        loggerException(ex);
    //        return false;
    //    }
    return false;
}

bool BCamera::SetAutoWhite(const bool& enable)
{
    try
    {
        if (!mDevice->GetRemoteNode(SFNC_BALANCEWHITEAUTO)->IsWriteable())
        {
            LOGERROR("SFNC_BALANCEWHITEAUTO is not Writeable");
            return false;
        }

        // options: Continuous|Once|Off
        mDevice->GetRemoteNode(SFNC_BALANCEWHITEAUTO)->SetString(enable ? "Continuous" : "Off");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("disable or open SFNC_BALANCEWHITEAUTO Failed");
        loggerException(ex);
        return false;
    }
    return false;
}

bool BCamera::SetExposureTime(const double& exposureTime)
{
    LOGINFO("SetExposureTime:");
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    try
    {
        if (mDevice->GetRemoteNode(SFNC_EXPOSURETIME)->IsWriteable())
        {
            mDevice->GetRemoteNode(SFNC_EXPOSURETIME)->SetDouble(exposureTime);
            LOGINFO("SetExposureTime ExposureTime:%f", exposureTime);
            LOGINFO("SetExposureTime Success");
            return true;
        }
        else
        {
            LOGERROR("SFNC_EXPOSURETIME is not Writeable");
            return false;
        }
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("SetExposureTime Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::GetExposureTime(double* exposureTime)
{
    LOGINFO("GetExposureTime:");
    if (exposureTime == nullptr)
    {
        LOGERROR("Parameter is Null");
        return false;
    }
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    try
    {
        *exposureTime = mDevice->GetRemoteNode(SFNC_EXPOSURETIME)->GetDouble();
        LOGINFO("GetExposureTime ExposureTime:%f", *exposureTime);
        LOGINFO("GetExposureTime Success");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("GetExposureTime Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::GetExposureRange(double* min, double* max)
{
    LOGINFO("GetExposureRange:");
    if (min == nullptr || max == nullptr)
    {
        LOGERROR("Parameter is Null");
        return false;
    }
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    try
    {
        *min = mDevice->GetRemoteNode(SFNC_EXPOSURETIME)->GetDoubleMin();
        *max = mDevice->GetRemoteNode(SFNC_EXPOSURETIME)->GetDoubleMax();
        LOGINFO("GetExposureRange min:%f max:%f", *min, *max);
        LOGINFO("GetExposureRange Success");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("GetExposureRange Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::SetGain(const double& gain)
{
    LOGINFO("SetGain:");
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    try
    {
        if (mDevice->GetRemoteNode(SFNC_GAIN)->IsWriteable())
        {
            mDevice->GetRemoteNode(SFNC_GAIN)->SetDouble(gain);
            LOGINFO("SetGain Gain:%f", gain);
            LOGINFO("SetGain Success");
            return true;
        }
        else
        {
            LOGERROR("SFNC_GAIN is not Writeable");
            return false;
        }
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("SetGain Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::GetGain(double* gain)
{
    LOGINFO("GetGain:");
    if (gain == nullptr)
    {
        LOGERROR("Parameter is Null");
        return false;
    }
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    try
    {
        *gain = mDevice->GetRemoteNode(SFNC_GAIN)->GetDouble();
        LOGINFO("GetGain Gain:%f", *gain);
        LOGINFO("GetGain Success");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("GetGain Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::GetGainRange(double* min, double* max)
{
    LOGINFO("GetGainRange:");
    if (min == nullptr || max == nullptr)
    {
        LOGERROR("Parameter is Null");
        return false;
    }
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    try
    {
        *min = mDevice->GetRemoteNode(SFNC_GAIN)->GetDoubleMin();
        *max = mDevice->GetRemoteNode(SFNC_GAIN)->GetDoubleMax();
        LOGINFO("GetGainRange min:%f max:%f", *min, *max);
        LOGINFO("GetGainRange Success");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("GetGainRange Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::SetGamma(const double& gamma)
{
    //    LOGINFO("SetGamma:");
    //    if (nullptr == mDevice)
    //    {
    //        LOGERROR("mDevice is Null");
    //        return false;
    //    }
    //    if (false == mIsOpen)
    //    {
    //        LOGERROR("Camera is not open");
    //        return false;
    //    }
    //    if (nullptr == mImgProcessor)
    //    {
    //        LOGERROR("mImgProcessor is Null");
    //        return false;
    //    }
    //    try
    //    {
    //        BGAPI2::Extended::ImageTransformationSettings::SetDestinationLutGamma(mImgProcessor, static_cast<bo_double>(gamma));
    //        LOGINFO("SetGamma Gamma:%f", gamma);
    //        LOGINFO("SetGamma Success");
    //        return true;
    //    }
    //    catch (Exceptions::IException & ex)
    //    {
    //        LOGERROR("SetGamma Failed");
    //        loggerException(ex);
    //        return false;
    //    }
    return false;
}

bool BCamera::GetGamma(double* gamma)
{
    //    LOGINFO("GetGamma:");
    //    if (gamma == nullptr)
    //    {
    //        LOGERROR("Parameter is Null");
    //        return false;
    //    }
    //    if (nullptr == mDevice)
    //    {
    //        LOGERROR("mDevice is Null");
    //        return false;
    //    }
    //    if (false == mIsOpen)
    //    {
    //        LOGERROR("Camera is not open");
    //        return false;
    //    }
    //    if (nullptr == mImgProcessor)
    //    {
    //        LOGERROR("mImgProcessor is Null");
    //        return false;
    //    }
    //    try
    //    {
    //        double min, max;
    //        BGAPI2::Extended::ImageTransformationSettings::GetDestinationLutGamma(mImgProcessor, gamma, &min, &max);
    //        LOGINFO("GetGamma Gamma:%f", *gamma);
    //        LOGINFO("GetGamma Success");
    //        return true;
    //    }
    //    catch (Exceptions::IException & ex)
    //    {
    //        LOGERROR("GetGamma Failed");
    //        loggerException(ex);
    //        return false;
    //    }
    return false;
}

bool BCamera::GetGammaRange(double* min, double* max)
{
    //    LOGINFO("GetGammaRange:");
    //    if (min == nullptr || max == nullptr)
    //    {
    //        LOGERROR("Parameter is Null");
    //        return false;
    //    }
    //    if (nullptr == mDevice)
    //    {
    //        LOGERROR("mDevice is Null");
    //        return false;
    //    }
    //    if (false == mIsOpen)
    //    {
    //        LOGERROR("Camera is not open");
    //        return false;
    //    }
    //    if (nullptr == mImgProcessor)
    //    {
    //        LOGERROR("mImgProcessor is Null");
    //        return false;
    //    }
    //    try
    //    {
    //        double gamma;
    //        BGAPI2::Extended::ImageTransformationSettings::GetDestinationLutGamma(mImgProcessor, &gamma, min, max);
    //        LOGINFO("GetGammaRange min:%f max:%f", *min, *max);
    //        LOGINFO("GetGammaRange Success");
    //        return true;
    //    }
    //    catch (Exceptions::IException & ex)
    //    {
    //        LOGERROR("GetGammaRange Failed");
    //        loggerException(ex);
    //        return false;
    //    }
    return false;
}
///
/// \brief BCamera::GetOutputFrameRate provide this API for user to get the current camera output frame rate.
/// \param outputframerate
/// \return
///
bool BCamera::GetOutputFrameRate(double *outputFrameRate)
{
    LOGINFO("GetOutputFrameRate:");
    if (outputFrameRate == nullptr)
    {
        LOGERROR("GetOutputFrameRate Parameter is Null");
        return false;
    }
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    try
    {
        *outputFrameRate = mDevice->GetRemoteNode(SFNC_ACQUISITION_FRAMERATE)->GetDouble();
        LOGINFO("GetOutputFrameRate:%f", *outputFrameRate);
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("GetOutputFrameRate Failed");
        loggerException(ex);
        return false;
    }
}

///
/// \brief BCamera::SetOutputFrameRate, before setoutputframerate, must let AcquisitionFrameRateEnable=true, than can setoutframerate.
/// \param outputframerate
/// \return
///
bool BCamera::SetOutputFrameRate(const double &outputFrameRate)
{

    LOGINFO( "SetAcquisitionFrameRateEnable:");
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    try
    {
        bool acquisitionframerateenable = mDevice->GetRemoteNode("AcquisitionFrameRateEnable")->GetBool();

        if(false == acquisitionframerateenable)
        {
            mDevice->GetRemoteNode("AcquisitionFrameRateEnable")->SetBool(true);

            LOGINFO("AcquisitionFrameRateEnable is false, if need continue setoutputframe rate, need set acquisitionframerateenable true.");
        }
        else
        {
            LOGINFO("AcquisitionFrameRateEnable is true, can continue set the Acqusition Frame Rate.");

            if (mDevice->GetRemoteNode(SFNC_ACQUISITION_FRAMERATE)->IsWriteable())
            {
                mDevice->GetRemoteNode(SFNC_ACQUISITION_FRAMERATE)->SetDouble(outputFrameRate);
                LOGINFO( "SetOutputFrameRate :%lf", outputFrameRate);
                LOGINFO( "SetOutputFrameRate Success");
                return true;
            }
            else
            {
                LOGERROR("SFNC_ACQUISITION_FRAMERATE is not Writeable");
                return false;
            }
        }
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("SetOutputFrameRate Failed");
        loggerException(ex);
        return false;
    }
}


bool BCamera::StopCameraAcquisiton()
{
    LOGINFO("StopCameraAcquisiton:");
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGINFO("Camera is not open");
        return false;
    }
    if (false == mIsAcquisiton)
    {
        LOGINFO("Camera is not acquisition current");
        return true;
    }
    try
    {
        mDataStream->StopAcquisition();
        BufferList* pBufferList = mDataStream->GetBufferList();
        pBufferList->DiscardAllBuffers();
        LOGINFO("DiscardAllBuffers Success");

        if (mDevice->GetRemoteNodeList()->GetNodePresent(SFNC_ACQUISITION_ABORT))
        {
            mDevice->GetRemoteNode(SFNC_ACQUISITION_ABORT)->Execute();
            LOGINFO("Execute SFNC_ACQUISITION_ABORT Success");
        }

        mDevice->GetRemoteNode(SFNC_ACQUISITION_STOP)->Execute();
        LOGINFO("Execute SFNC_ACQUISITION_STOP Success");
        mIsAcquisiton = false;
        unHandleImageEvent();
        LOGINFO("unHandleImageEvent Success");
        LOGINFO("StopCameraAcquisiton Success");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("StopCameraAcquisiton Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::CloseCamera()
{
    LOGINFO("CloseCamera:");
    if (nullptr == mDevice)
    {
        LOGERROR("mDevice is Null");
        return false;
    }
    if (false == mIsOpen)
    {
        LOGINFO("Camera is not open");
        return true;
    }
    try
    {
        if (mIsAcquisiton)
        {
            if (!StopCameraAcquisiton())
            {
                LOGERROR("StopCameraAcquisiton Failed");
                return false;
            }
        }
        BufferList* bufferList = mDataStream->GetBufferList();
        bufferList->DiscardAllBuffers();
        LOGINFO("DiscardAllBuffers Success");
        while (bufferList->size() > 0)
        {
            BGAPI2::Buffer* buffer = bufferList->begin()->second;
            bufferList->RevokeBuffer(buffer);
            delete buffer;
        }
        mDataStream->Close();
        LOGINFO("mDataStream Close Success");
        //        if (mAutoFunction)
        //        {
        //            delete mAutoFunction;
        //        }
        mDevice->Close();
        LOGINFO("mDevice Close Success");
        mIsOpen = false;
        LOGINFO("CloseCamera Success");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("CloseCamera Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::handleImageEvent()
{
    LOGINFO("handleImageEvent:");
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    if (nullptr == mDataStream)
    {
        LOGERROR("mDataStream is Null");
        return false;
    }
    try
    {
        //register callback
        //        mDataStream->RegisterNewBufferEvent(Events::EVENTMODE_EVENT_HANDLER);
        mDataStream->RegisterNewBufferEventHandler(this, (BGAPI2::Events::NewBufferEventHandler)NewBufferEventHandler);
        mIsRegisterImageHandler = true;
        LOGINFO("handleImageEvent Success");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("handleImageEvent Failed");
        loggerException(ex);
        return false;
    }
}

bool BCamera::unHandleImageEvent()
{
    LOGINFO("unHandleImageEvent:");
    if (false == mIsOpen)
    {
        LOGERROR("Camera is not open");
        return false;
    }
    if (false == mIsRegisterImageHandler)
    {
        LOGINFO("No ImageEvent handle");
        return true;
    }
    try
    {
        mDataStream->UnregisterNewBufferEvent();
        mDataStream->RegisterNewBufferEvent(Events::EVENTMODE_UNREGISTERED);
        mIsRegisterImageHandler = false;
        LOGINFO("unHandleImageEvent Success");
        return true;
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("unHandleImageEvent Failed");
        loggerException(ex);
        return false;
    }
}

//#define FRAME_LIMIT
//#ifdef  FRAME_LIMIT
//    #define FRAME_RATE 24.
//    clock_t time_frame = 0l;
//    double time_interval = 1000. / FRAME_RATE;
//    double time_temp = 0.;
//#endif //  FRAME_LIMIT



void NewBufferEventHandler(void* callbackOwner, BGAPI2::Buffer* pBufferFilled)
{
#ifdef  FRAME_LIMIT
    if (time_frame == 0l)
    {
        time_frame = clock();
    }
    else
    {
        clock_t temp = clock();
        if ((double)(temp - time_frame) < time_interval)
        {
            pBufferFilled->QueueBuffer();
            return;
        }
        else
        {
            time_frame = clock();
        }
    }
#endif //  FRAME_LIMIT

    BCamera* camera = (BCamera*)callbackOwner;
    clock_t curTime = clock();

    camera->ImageNo++;
    if(curTime-camera->ImageTimestamp <= 0)
    {
        LOGINFO("FrameNo: %d, DurationSinceLastFrame：: %d", camera->ImageNo, (curTime-camera->ImageTimestamp));
    }
    else
    {
        LOGINFO("FrameNo: %d, DurationSinceLastFrame：: %d, fps: %d", camera->ImageNo, (curTime-camera->ImageTimestamp), 1000/(curTime-camera->ImageTimestamp));
    }

    try
    {
        if (pBufferFilled == nullptr)
        {
            LOGERROR("Buffer Timeout after 1000 msec");
            return;
        }
        else if (pBufferFilled->GetIsIncomplete() == true)
        {
            LOGERROR("Image is incomplete");
            // queue buffer again
            pBufferFilled->QueueBuffer();
            return;
        }
        if (nullptr == camera->mImageHandlerFunc)
        {
            pBufferFilled->QueueBuffer();
            return;
        }
        //TODO:all image transform to bgr24 and send to caller
        unsigned char* data = nullptr;
        int dataSize = 0;
        int width = 0;
        int height = 0;
        Image* pTransformImage;
        if (pBufferFilled->GetPixelFormat() == "Mono8" || pBufferFilled->GetPixelFormat() == "BGR8")
        {
            data = (unsigned char*)pBufferFilled->GetMemPtr();
            dataSize = pBufferFilled->GetMemSize();
            width = pBufferFilled->GetWidth();
            height = pBufferFilled->GetHeight();
            camera->mImageHandlerFunc(data, dataSize, width, height, camera->ImageNo, camera->ImageTimestamp);
            camera->ImageTimestamp = curTime;
            pBufferFilled->QueueBuffer();
        }
        else //convert to BGR8
        {
            Image * pImage = camera->mImgProcessor->CreateImage(
                        (bo_uint)pBufferFilled->GetWidth(),
                        (bo_uint)(int)pBufferFilled->GetHeight(),
                        pBufferFilled->GetPixelFormat(),
                        pBufferFilled->GetMemPtr(),
                        pBufferFilled->GetMemSize());

            pTransformImage = camera->mImgProcessor->CreateTransformedImage(pImage, "BGR8");
            data = (unsigned char*)pTransformImage->GetBuffer();
            dataSize = pImage->GetTransformBufferLength("BGR8");
            width = pTransformImage->GetWidth();
            height = pTransformImage->GetHeight();

            pBufferFilled->QueueBuffer();
            camera->mImageHandlerFunc(data, dataSize, width, height, camera->ImageNo, camera->ImageTimestamp);
            pImage->Release();
            pTransformImage->Release();
            camera->ImageTimestamp = curTime;
        }
    }
    catch (Exceptions::IException & ex)
    {
        loggerException(ex);
    }
}
