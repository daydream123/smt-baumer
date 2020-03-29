#include "CameraWrapper.h"
#ifdef WIN32
#include "bgapi2_ext/bgapi2_ext.h"
#else
#include "bgapi2_genicam/bgapi2_genicam.hpp"
#endif
#include <vector>
#include <cstring>
#include "BCamera.h"
#include <smt-logger/smt_logger.h>

using namespace BGAPI2;
using namespace std;

bool isInit = false;

vector<Interface*> gInterfaces;
vector<System*> gSystems;
vector<Device*> gDevices;
vector<BCamera*> gCameraList;

ReturnCode InitSystem()
{
    LoggerConfig config ={"Camera", LogLevel::TRACE, true, 1};
    setupLogger(config);
    LOGINFO("Start Init BaumerCamera System");
    if (isInit)
    {
        LOGINFO("Init BaumerCamera System Complete");
        return ReturnCode::SUCCESS;
    }
    SystemList* systemList;
    InterfaceList* interfaceList;
    DeviceList* deviceList;
    try {
        systemList = SystemList::CreateInstanceFromPath("./");
        systemList->Refresh();
        // no system available in system list
        if (systemList->size() == 0)
        {
            LOGWARN("Fail Found BaumerCamera System");
            return ReturnCode::FAIL_FOUND_SYSTEM;
        }
    }
    catch (Exceptions::IException & ex)
    {
        LOGERROR("ExceptionType: %s", (const char*)ex.GetType());
        LOGERROR("ErrorDescription: %s", (const char*)ex.GetErrorDescription());
        LOGERROR("in function: %s", (const char*)ex.GetFunctionName());
        return ReturnCode::FAIL_INITIALIZE_SYSTEMLIST;
    }

    for (SystemList::iterator sysIterator = systemList->begin(); sysIterator != systemList->end(); sysIterator++)
    {
        try
        {
            String systemID = sysIterator->first;
            System* system = sysIterator->second;
            String systemType = system->GetTLType();
            if (systemType == "U3V") // || systemType == "GEV"
            {
                system->Open();
                LOGINFO("Open next system:");
                LOGINFO("    System Name:     %s", (char*)system->GetFileName());
                LOGINFO("    System Type:     %s", (char*)system->GetTLType());
                LOGINFO("    System Version:  %s", (char*)system->GetVersion());
                //LOGINFO("System PathName: %s", system->GetPathName());
                LOGINFO("    Opened system - NodeList Information:");
                LOGINFO("    GenTL Version:   %s.%s", (char*)system->GetNode("GenTLVersionMajor")->GetValue(), (char*)system->GetNode("GenTLVersionMinor")->GetValue());
                gSystems.push_back(system);
                try
                {
                    interfaceList = system->GetInterfaces();
                    interfaceList->Refresh(100);
                    if (interfaceList->size() == 0)
                    {
                        LOGWARN("Fail Found BaumerCamera Interface");
                        return ReturnCode::FAIL_FOUND_INTERFACE;
                    }
                }
                catch (Exceptions::IException & ex)
                {
                    LOGERROR("ExceptionType: %s", (const char*)ex.GetType());
                    LOGERROR("ErrorDescription: %s", (const char*)ex.GetErrorDescription());
                    LOGERROR("in function: %s", (const char*)ex.GetFunctionName());
                    return ReturnCode::FAIL_INITIALIZE_INTERFACELIST;
                }
                for (InterfaceList::iterator ifIterator = interfaceList->begin(); ifIterator != interfaceList->end(); ifIterator++)
                {
                    try
                    {
                        String interfaceID = ifIterator->first;
                        Interface* iface = ifIterator->second;
                        iface->Open();
                        LOGINFO("Open interface:");
                        LOGINFO("    Interface ID:      %s", (char*)interfaceID);
                        LOGINFO("    Interface Type:    %s", (char*)iface->GetTLType());
                        LOGINFO("    Interface Name:    %s", (char*)iface->GetDisplayName());
                        gInterfaces.push_back(iface);
                        deviceList = iface->GetDevices();
                        deviceList->Refresh(100);
                        LOGINFO("Find devices:");
                        int devSize = deviceList->size();
                        if(!(devSize > 0))
                        {
                            LOGERROR("    %d cameras found.", devSize);
                            iface->Close();
                        }
                        else
                        {
                            LOGINFO("    Detected devices:    %d", devSize);
                            for (DeviceList::iterator devIterator = deviceList->begin(); devIterator != deviceList->end(); devIterator++)
                            {
                                LOGINFO("    Device DeviceID:        %s", (char*)devIterator->first);
                                LOGINFO("    Device Model:           %s", (char*)devIterator->second->GetModel());
                                LOGINFO("    Device SerialNumber:    %s", (char*)devIterator->second->GetSerialNumber());
                                LOGINFO("    Device Vendor:          %s", (char*)devIterator->second->GetVendor());
                                LOGINFO("    Device TLType:          %s", (char*)devIterator->second->GetTLType());
                                LOGINFO("    Device AccessStatus:    %s", (char*)devIterator->second->GetAccessStatus());
                                LOGINFO("    Device UserID:          %s", (char*)devIterator->second->GetDisplayName());
                                gDevices.push_back(devIterator->second);
                            }
                        }
                    }
                    catch (Exceptions::IException & ex)
                    {
                        LOGERROR("ExceptionType: %s", (const char*)ex.GetType());
                        LOGERROR("ErrorDescription: %s", (const char*)ex.GetErrorDescription());
                        LOGERROR("in function: %s", (const char*)ex.GetFunctionName());
                        return ReturnCode::FAIL_INITIALIZE_DEVICELIST;
                    }
                }
            }
        }
        catch (Exceptions::ResourceInUseException & ex)
        {
            LOGERROR("ExceptionType: %s", (const char*)ex.GetType());
            LOGERROR("ErrorDescription: %s", (const char*)ex.GetErrorDescription());
            LOGERROR("in function: %s", (const char*)ex.GetFunctionName());
            return ReturnCode::RESOURCE_IN_USE;
        }
    }
    isInit = true;
    LOGINFO("Init BaumerCamera System Complete");
    return ReturnCode::SUCCESS;
}

ReturnCode GetIsSystemInit(bool* state)
{
    if (nullptr == state)
    {
        return ReturnCode::PARAMETER_NULLPTR;
    }
    *state = isInit;
    return ReturnCode::SUCCESS;
}

ReturnCode GetCameraCount(int* count)
{
    if (!isInit)
    {
        return ReturnCode::SYSTEM_NOT_INIT;
    }
    if (nullptr == count)
    {
        return ReturnCode::PARAMETER_NULLPTR;
    }
    *count = gDevices.size();
    return ReturnCode::SUCCESS;
}

ReturnCode GetCameraDeviceIDByIndex(const unsigned int& index, char** deviceID)
{
    if (!isInit)
    {
        return ReturnCode::SYSTEM_NOT_INIT;
    }
    if (nullptr == deviceID)
    {
        return ReturnCode::PARAMETER_NULLPTR;
    }
    if (index >= gDevices.size() || index < 0)
    {
        return ReturnCode::INDEX_OUT_OF_RANGE;
    }
    Device* dev = gDevices.at(index);
    BGAPI2::String id = dev->GetID();
    *deviceID = (char*)malloc((size_t)id.size() + 1);
    strcpy(*deviceID, id.get());
    return ReturnCode::SUCCESS;
}

ReturnCode OpenCameraById(const char* deviceID, SMART_CAMERA_LIB_DEV_HANDLE* handle)
{
    if (nullptr == deviceID)
    {
        return ReturnCode::PARAMETER_NULLPTR;
    }
    Device* device = nullptr;
    for (auto dev: gDevices)
    {
        BGAPI2::String id = dev->GetID();
        if (id == deviceID)
        {
            device = dev;
        }
    }
    if (nullptr == device)
    {
        return ReturnCode::DEVICEID_INVALID;
    }
    for (auto d : gCameraList)
    {
        if (device == d->GetDevice())
        {
            *handle = d;
            if (!(*handle)->GetIsOpen())
            {
                if (!(*handle)->OpenCamera())
                {
                    delete *handle;
                    return ReturnCode::FAIL;
                }
            }
            return ReturnCode::SUCCESS;
        }
    }
    *handle = new BCamera(device);
    if (!(*handle)->OpenCamera())
    {
        delete *handle;
        return ReturnCode::FAIL;
    }
    gCameraList.push_back(*handle);
    return ReturnCode::SUCCESS;
}

ReturnCode OpenCameraByIndex(const int& index, SMART_CAMERA_LIB_DEV_HANDLE* handle)
{
    if (!(gDevices.size() > index) || index < 0)
    {
        return ReturnCode::DEVICEID_INVALID;
    }
    Device* device = gDevices.at(index);

    for (auto d : gCameraList)
    {
        if (device == d->GetDevice())
        {
            *handle = d;
            if (!d->GetIsOpen())
            {
                if (!d->OpenCamera())
                {
                    delete d;
                    return ReturnCode::FAIL;
                }
            }
            return ReturnCode::SUCCESS;
        }
    }
    *handle = new BCamera(device);
    if (!(*handle)->OpenCamera())
    {
        delete *handle;
        return ReturnCode::FAIL;
    }
    gCameraList.push_back(*handle);
    return ReturnCode::SUCCESS;
}

ReturnCode GetIsMono8(SMART_CAMERA_LIB_DEV_HANDLE handle, bool* result)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetIsMono8(result))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode LoadSettingFile(SMART_CAMERA_LIB_DEV_HANDLE handle, const char* fileName)
{
    return ReturnCode();
}

ReturnCode SetSettingFile(SMART_CAMERA_LIB_DEV_HANDLE handle, const char* fileName)
{
    return ReturnCode();
}

ReturnCode GetTriggerMode(SMART_CAMERA_LIB_DEV_HANDLE handle, TriggerMode* mode)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetTriggerMode(mode))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode SetTriggerMode(SMART_CAMERA_LIB_DEV_HANDLE handle, TriggerMode mode)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->SetTriggerMode(mode))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode SetImageHandlerCallback(SMART_CAMERA_LIB_DEV_HANDLE handle, camera_image_handle_func func)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->SetImageHandlerCallback(func))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode StartCameraAcquisiton(SMART_CAMERA_LIB_DEV_HANDLE handle)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->StartCameraAcquisiton())
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode StopCameraAcquisiton(SMART_CAMERA_LIB_DEV_HANDLE handle)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->StopCameraAcquisiton())
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode TakeOneShot(SMART_CAMERA_LIB_DEV_HANDLE handle)
{
    return ReturnCode();
}

ReturnCode GetCameraResolution(SMART_CAMERA_LIB_DEV_HANDLE handle, int* width, int* height)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetCameraResolution(width, height))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode GetCameraResolutionRange(SMART_CAMERA_LIB_DEV_HANDLE handle, int* minWidth, int* maxWidth, int* minHeight, int* maxHeight)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetCameraResolutionRange(minWidth, maxWidth, minHeight, maxHeight))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode SetCameraResolution(SMART_CAMERA_LIB_DEV_HANDLE handle, const int& width, const int& height)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->SetCameraResolution(width, height))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode SetAutoExposure(SMART_CAMERA_LIB_DEV_HANDLE handle, const bool& enable)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->SetAutoExposure(enable))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode SetAutoGain(SMART_CAMERA_LIB_DEV_HANDLE handle, const bool& enable)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->SetAutoGain(enable))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode SetAutoWhite(SMART_CAMERA_LIB_DEV_HANDLE handle, const bool& enable)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->SetAutoWhite(enable))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode GetAutoExposure(SMART_CAMERA_LIB_DEV_HANDLE handle, bool* enable)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetAutoExposure(enable))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode GetAutoGain(SMART_CAMERA_LIB_DEV_HANDLE handle, bool* enable)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetAutoGain(enable))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode GetAutoWhite(SMART_CAMERA_LIB_DEV_HANDLE handle, bool* enable)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetAutoWhite(enable))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode SetExposureTime(SMART_CAMERA_LIB_DEV_HANDLE handle, const double& exposureTime)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->SetExposureTime(exposureTime))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode GetExposureTime(SMART_CAMERA_LIB_DEV_HANDLE handle, double* exposureTime)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetExposureTime(exposureTime))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode GetExposureRange(SMART_CAMERA_LIB_DEV_HANDLE handle, double* min, double* max)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetExposureRange(min, max))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode SetGain(SMART_CAMERA_LIB_DEV_HANDLE handle, const double& gain)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->SetGain(gain))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode GetGain(SMART_CAMERA_LIB_DEV_HANDLE handle, double* gain)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetGain(gain))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode GetGainRange(SMART_CAMERA_LIB_DEV_HANDLE handle, double* min, double* max)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetGainRange(min, max))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode SetGamma(SMART_CAMERA_LIB_DEV_HANDLE handle, const double& gamma)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->SetGamma(gamma))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode GetGamma(SMART_CAMERA_LIB_DEV_HANDLE handle, double* gamma)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetGamma(gamma))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode GetGammaRange(SMART_CAMERA_LIB_DEV_HANDLE handle, double* min, double* max)
{
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetGammaRange(min, max))
    {
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode GetWhiteBalanceGainCount(SMART_CAMERA_LIB_DEV_HANDLE devPtr, int* count)
{
    return ReturnCode();
}

ReturnCode GetWhiteBalanceGainByIndex(SMART_CAMERA_LIB_DEV_HANDLE devPtr, const int& index, float* gain, char** name, float* minGain, float* maxGain)
{
    return ReturnCode();
}

ReturnCode SetWhiteBalanceGainByIndex(SMART_CAMERA_LIB_DEV_HANDLE devPtr, const int& index, const float& gain)
{
    return ReturnCode();
}

///
/// \brief GetOutputFrameRate camerawrapper calss provide get the camera output frame rate API
/// \param handle
/// \param outputframerate
/// \return
///
ReturnCode GetOutputFrameRate(SMART_CAMERA_LIB_DEV_HANDLE handle, double *outputframerate)
{
    if (nullptr == handle)
    {
        LOGERROR("GetOutputFrameRate PARAMETER_NULLPTR");
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->GetOutputFrameRate(outputframerate))
    {
        LOGERROR("GetOutputFrameRate Failed");
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

///
/// \brief SetOutputFrameRate CameraWrapper calss provide set the camera output frame rate API.
/// \param handle
/// \param outputframerate
/// \return
///
ReturnCode SetOutputFrameRate(SMART_CAMERA_LIB_DEV_HANDLE handle, const double &outputframerate)
{
    if (nullptr == handle)
    {
        LOGERROR("SetOutputFrameRate PARAMETER_NULLPTR");
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->SetOutputFrameRate(outputframerate))
    {
        LOGERROR("SetOutputFrameRate Failed");
        return ReturnCode::FAIL;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode CloseCamera(SMART_CAMERA_LIB_DEV_HANDLE handle)
{
    ReturnCode retcode = ReturnCode::FAIL ;
    if (nullptr == handle)
    {
        return ReturnCode::PARAMETER_NULLPTR;;
    }
    if (!handle->CloseCamera())
    {
        return ReturnCode::FAIL;
    }
    if(isInit)
    {
         retcode =  UnInitSystem();
    }
    return retcode;
}

ReturnCode UnInitSystem()
{
    if (!isInit)
    {
        return ReturnCode::SUCCESS;
    }
    try
    {
        while (gCameraList.size() > 0)
        {
            auto camera = gCameraList.front();
            delete camera;
            gCameraList.pop_back();
        }
        for (auto d : gDevices)
        {
            d->Close();
        }
        gDevices.clear();

        for (auto i : gInterfaces)
        {
            i->Close();
        }
        gInterfaces.clear();

        for (auto s : gSystems)
        {
            s->Close();
        }
        gSystems.clear();
        SystemList::ReleaseInstance();
        isInit = false;
        return ReturnCode::SUCCESS;
    }
    catch (Exceptions::IException & ex)
    {
        return ReturnCode::FAIL_UNINIT;
    }
}
