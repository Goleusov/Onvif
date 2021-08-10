//
// Created by allforgot on 2020/3/28.
//
#include <sstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <stdsoap2.h>
#include "wsseapi.h"
#include "soapPTZBindingProxy.h"
#include "soapImagingBindingProxy.h"
#include "soapDeviceBindingProxy.h"
#include "soapMediaBindingProxy.h"
#include "soapPullPointSubscriptionBindingProxy.h"
#include "wsaapi.h"

#include "common.h"
#include <thread>
using namespace std;

//const string username = "admin";
const string username = "peeklio";

const string password = "Gdhuemk";
//const string password = "admin";
//const string password = "Admin1234";
//const string password = "adm12345";

const string hostname = "http://10.24.72.22:8080/onvif/media";

typedef struct
{
    float pan;
    float tilt;
    float zoom;

    /*enum tt__MoveStatus {
        tt__MoveStatus__IDLE = 0,
        tt__MoveStatus__MOVING = 1,
        tt__MoveStatus__UNKNOWN = 2
    };*/
    tt__MoveStatus move_status_pan_tilt;
    tt__MoveStatus move_status_zoom;
} _ocp_PTZStatus;

float panLimitsMin = -1;
float panLimitsMax = 1;
float tiltLimitsMin = -1;
float tiltLimitsMax = 1;
float zoomLimitsMin = 0;
float zoomLimitsMax = 1;

bool getConfiguraions()
{

    //    std::cout << "====================== PTZBinding Configurations ======================" << std::endl;

    PTZBindingProxy ptzBindingProxy;
    ptzBindingProxy.soap_endpoint = hostname.c_str();
    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(ptzBindingProxy.soap, nullptr, username.c_str(), password.c_str()))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
        report_error(ptzBindingProxy.soap);
        return false;
    }

    struct soap *soap = soap_new();
    _tptz__GetConfigurations *GetConfigurations = soap_new__tptz__GetConfigurations(soap);
    _tptz__GetConfigurationsResponse *GetConfigurationsResponse = soap_new__tptz__GetConfigurationsResponse(soap);

    bool executeResult = false;
    if (SOAP_OK == ptzBindingProxy.GetConfigurations(GetConfigurations, *GetConfigurationsResponse))
    {
        for (auto PTZConfiguration : GetConfigurationsResponse->PTZConfiguration)
        {
            cout << "NodeToken: " << PTZConfiguration->NodeToken << endl;
            cout << "DefaultAbsolutePantTiltPositionSpace: " << *PTZConfiguration->DefaultAbsolutePantTiltPositionSpace << endl;
            cout << "DefaultAbsoluteZoomPositionSpace: " << *PTZConfiguration->DefaultAbsoluteZoomPositionSpace << endl;
            cout << "DefaultRelativePanTiltTranslationSpace: " << *PTZConfiguration->DefaultRelativePanTiltTranslationSpace << endl;
            cout << "DefaultRelativeZoomTranslationSpace: " << *PTZConfiguration->DefaultRelativeZoomTranslationSpace << endl;
            cout << "DefaultContinuousPanTiltVelocitySpace: " << *PTZConfiguration->DefaultContinuousPanTiltVelocitySpace << endl;
            cout << "DefaultContinuousZoomVelocitySpace: " << *PTZConfiguration->DefaultContinuousZoomVelocitySpace << endl;
            cout << "DefaultPTZSpeed Pan:     " << PTZConfiguration->DefaultPTZSpeed->PanTilt->x << endl;
            cout << "DefaultPTZSpeed Tilt:    " << PTZConfiguration->DefaultPTZSpeed->PanTilt->y << endl;
            cout << "DefaultPTZSpeed Zoom:    " << PTZConfiguration->DefaultPTZSpeed->Zoom->x << endl;
            cout << "DefaultPTZTimeout:       " << *PTZConfiguration->DefaultPTZTimeout << endl;
            cout << "PanTiltLimits Pan Min:   " << PTZConfiguration->PanTiltLimits->Range->XRange->Min << endl;
            panLimitsMin = PTZConfiguration->PanTiltLimits->Range->XRange->Min;
            cout << "PanTiltLimits Pan Max:   " << PTZConfiguration->PanTiltLimits->Range->XRange->Max << endl;
            panLimitsMax = PTZConfiguration->PanTiltLimits->Range->XRange->Max;
            cout << "PanTiltLimits Tilt Min:  " << PTZConfiguration->PanTiltLimits->Range->YRange->Min << endl;
            tiltLimitsMin = PTZConfiguration->PanTiltLimits->Range->YRange->Min;
            cout << "PanTiltLimits Tilt Max:  " << PTZConfiguration->PanTiltLimits->Range->YRange->Max << endl;
            tiltLimitsMax = PTZConfiguration->PanTiltLimits->Range->YRange->Max;
            cout << "ZoomLimits Min:          " << PTZConfiguration->ZoomLimits->Range->XRange->Min << endl;
            zoomLimitsMin = PTZConfiguration->ZoomLimits->Range->XRange->Min;
            cout << "ZoomLimits Max:          " << PTZConfiguration->ZoomLimits->Range->XRange->Max << endl;
            zoomLimitsMax = PTZConfiguration->ZoomLimits->Range->XRange->Max;
            cout << "Extension:               " << PTZConfiguration->Extension << endl;
            cout << "MoveRamp:                " << PTZConfiguration->MoveRamp << endl;
            cout << "PresetRamp:              " << PTZConfiguration->PresetRamp << endl;
            cout << "PresetTourRamp:          " << PTZConfiguration->PresetTourRamp << endl;
        }
        executeResult = true;
    }
    else
    {
        std::cerr << "Error: getConfiguraions" << endl;
        report_error(ptzBindingProxy.soap);
    }

    return executeResult;
}

bool getStatus(const string &profileToken, _ocp_PTZStatus &ptzStatus)
{

    //    std::cout << "====================== PTZBinding Status ======================" << std::endl;

    PTZBindingProxy ptzBindingProxy;
    ptzBindingProxy.soap_endpoint = hostname.c_str();
    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(ptzBindingProxy.soap, nullptr, username.c_str(), password.c_str()))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
        report_error(ptzBindingProxy.soap);
        return false;
    }

    struct soap *soap = soap_new();
    _tptz__GetStatus *GetStatus = soap_new__tptz__GetStatus(soap);
    _tptz__GetStatusResponse *GetStatusResponse = soap_new__tptz__GetStatusResponse(soap);

    const tt__ReferenceToken &token = profileToken;
    GetStatus->ProfileToken = token;

    bool executeResult = false;
    if (SOAP_OK == ptzBindingProxy.GetStatus(GetStatus, *GetStatusResponse))
    {
        if (GetStatusResponse->PTZStatus)
        {

            if (GetStatusResponse->PTZStatus->Position)
            {
                ptzStatus.pan = GetStatusResponse->PTZStatus->Position->PanTilt->x;
                // std::cout << "x" << GetStatusResponse->PTZStatus->Position->PanTilt->x << std::endl;
                ptzStatus.tilt = GetStatusResponse->PTZStatus->Position->PanTilt->y;
                //std::cout << "y" << GetStatusResponse->PTZStatus->Position->PanTilt->y << std::endl;

                ptzStatus.zoom = GetStatusResponse->PTZStatus->Position->Zoom->x;
            }
            else
            {
                cerr << "Error get ptz position" << endl;
            }
            if (GetStatusResponse->PTZStatus->MoveStatus)
            {
                if (GetStatusResponse->PTZStatus->MoveStatus->PanTilt)
                {
                    ptzStatus.move_status_pan_tilt = *(GetStatusResponse->PTZStatus->MoveStatus->PanTilt);
                }
                else
                {
                    ptzStatus.move_status_pan_tilt = tt__MoveStatus::tt__MoveStatus__UNKNOWN;
                }
                if (GetStatusResponse->PTZStatus->MoveStatus->Zoom)
                {
                    ptzStatus.move_status_zoom = *(GetStatusResponse->PTZStatus->MoveStatus->Zoom);
                }
                else
                {
                    ptzStatus.move_status_zoom = tt__MoveStatus::tt__MoveStatus__UNKNOWN;
                }
            }
            else
            {
                ptzStatus.move_status_pan_tilt = tt__MoveStatus::tt__MoveStatus__UNKNOWN;
                ptzStatus.move_status_zoom = tt__MoveStatus::tt__MoveStatus__UNKNOWN;
            }
            executeResult = true;
        }
        else
        {
            std::cout << "вылет" << std::endl;

            std::cerr << "Error: getStatus" << endl;
            report_error(ptzBindingProxy.soap);
        }
    }
    else
    {
        std::cout << "вылет" << std::endl;

        std::cerr << "Error: getStatus" << endl;
        report_error(ptzBindingProxy.soap);
    }

    CLEANUP_SOAP(soap);

    return executeResult;
}

bool continuousMove(const string &profileToken, const float &speedPTX, const float &speedPTY, const float &speed_zoom,
                    const long long &timeout)
{
    //    std::cout << "====================== PTZBinding ContinuousMove ======================" << std::endl;

    PTZBindingProxy ptzBindingProxy;
    ptzBindingProxy.soap_endpoint = hostname.c_str();
    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(ptzBindingProxy.soap, nullptr, username.c_str(), password.c_str()))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
        report_error(ptzBindingProxy.soap);
        return false;
    }

    struct soap *soap = soap_new();
    _tptz__ContinuousMove *ContinuousMove = soap_new__tptz__ContinuousMove(soap);
    _tptz__ContinuousMoveResponse *ContinuousMoveResponse = soap_new__tptz__ContinuousMoveResponse(soap);

    const tt__ReferenceToken &token = profileToken;
    ContinuousMove->ProfileToken = token;
    ContinuousMove->Velocity = soap_new_tt__PTZSpeed(soap);
    ContinuousMove->Velocity->PanTilt = soap_new_tt__Vector2D(soap);
    ContinuousMove->Velocity->PanTilt->x = speedPTX;
    ContinuousMove->Velocity->PanTilt->y = speedPTY;
    ContinuousMove->Velocity->Zoom = soap_new_tt__Vector1D(soap);
    ContinuousMove->Velocity->Zoom->x = speed_zoom;
    ContinuousMove->Timeout = (xsd__duration *)soap_malloc(soap, sizeof(xsd__duration *));
    *ContinuousMove->Timeout = timeout;

    bool executeResult = false;
    if (SOAP_OK == ptzBindingProxy.ContinuousMove(ContinuousMove, *ContinuousMoveResponse))
    {
        executeResult = true;
    }
    else
    {
        std::cerr << "Error: continuousMove" << endl;
        report_error(ptzBindingProxy.soap);
    }

    CLEANUP_SOAP(soap);

    return executeResult;
}

bool stop(const string &profileToken, const bool &stopPanTilt, const bool &stopZoom)
{
    //    std::cout << "====================== PTZBinding Stop ======================" << std::endl;

    PTZBindingProxy ptzBindingProxy;
    ptzBindingProxy.soap_endpoint = hostname.c_str();
    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(ptzBindingProxy.soap, nullptr, username.c_str(), password.c_str()))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
        report_error(ptzBindingProxy.soap);
        return false;
    }

    struct soap *soap = soap_new();
    _tptz__Stop *Stop = soap_new__tptz__Stop(soap);
    _tptz__StopResponse *StopResponse = soap_new__tptz__StopResponse(soap);

    const tt__ReferenceToken &token = profileToken;
    Stop->ProfileToken = token;
    Stop->PanTilt = (bool *)soap_malloc(soap, sizeof(bool *));
    Stop->Zoom = (bool *)soap_malloc(soap, sizeof(bool *));
    *Stop->PanTilt = stopPanTilt;
    *Stop->Zoom = stopZoom;

    bool executeResult = false;
    if (SOAP_OK == ptzBindingProxy.Stop(Stop, *StopResponse))
    {
        executeResult = true;
    }
    else
    {
        std::cerr << "Error: stop" << endl;
        report_error(ptzBindingProxy.soap);
    }

    CLEANUP_SOAP(soap);

    return executeResult;
}

bool absoluteMove(const std::string &profileToken, const float &pantiltX, const float &pantiltY, const float &zoom,
                  const float &speedPTX, const float &speedPTY, const float &speedZoom)
{
    //    std::cout << "====================== PTZBinding AbsoluteMove ======================" << std::endl;

    if (pantiltX < panLimitsMin || pantiltX > panLimitsMax || pantiltY < tiltLimitsMin || pantiltY > tiltLimitsMax || zoom < zoomLimitsMin || zoom > zoomLimitsMax)
    {
        cerr << "Destination out of bounds" << endl;
        return false;
    }

    PTZBindingProxy ptzBindingProxy;
    ptzBindingProxy.soap_endpoint = hostname.c_str();
    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(ptzBindingProxy.soap, nullptr, username.c_str(), password.c_str()))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
        report_error(ptzBindingProxy.soap);
        return false;
    }

    struct soap *soap = soap_new();
    _tptz__AbsoluteMove *AbsoluteMove = soap_new__tptz__AbsoluteMove(soap);
    _tptz__AbsoluteMoveResponse *AbsoluteMoveResponse = soap_new__tptz__AbsoluteMoveResponse(soap);

    const tt__ReferenceToken &token = profileToken;
    AbsoluteMove->ProfileToken = token;

    AbsoluteMove->Position = soap_new_tt__PTZVector(soap);
    AbsoluteMove->Position->PanTilt = soap_new_tt__Vector2D(soap);
    AbsoluteMove->Position->PanTilt->x = pantiltX;
    AbsoluteMove->Position->PanTilt->y = pantiltY;
    AbsoluteMove->Position->Zoom = soap_new_tt__Vector1D(soap);
    AbsoluteMove->Position->Zoom->x = zoom;
    AbsoluteMove->Speed = soap_new_tt__PTZSpeed(soap);
    AbsoluteMove->Speed->PanTilt = soap_new_tt__Vector2D(soap);
    AbsoluteMove->Speed->PanTilt->x = speedPTX;
    AbsoluteMove->Speed->PanTilt->y = speedPTY;
    AbsoluteMove->Speed->Zoom = soap_new_tt__Vector1D(soap);
    AbsoluteMove->Speed->Zoom->x = speedZoom;

    bool executeResult = false;
    if (SOAP_OK == ptzBindingProxy.AbsoluteMove(AbsoluteMove, *AbsoluteMoveResponse))
    {
        executeResult = true;
    }
    else
    {
        std::cerr << "Error: absoluteMove" << endl;
        report_error(ptzBindingProxy.soap);
    }

    CLEANUP_SOAP(soap);

    return executeResult;
}

template <typename T>
std::string printRangePtr(T *ptr)
{
    std::ostringstream os;
    if (ptr)
    {
        os << "[" << ptr->Min << "," << ptr->Max << "]";
    }
    else
    {
        os << "(null)";
    }
    return os.str();
}

template <typename T>
std::string printPtr(T *ptr)
{
    std::ostringstream os;
    if (ptr)
    {
        os << *ptr;
    }
    else
    {
        os << "(null)";
    }
    return os.str();
}

template <typename T>
std::string printMode(T *ptr)
{
    std::ostringstream os;
    if (ptr)
    {
        os << ptr->Mode;
    }
    else
    {
        os << "(null)";
    }
    return os.str();
}

bool absoluteMove1()
{

    PTZBindingProxy ptzBindingProxy;

    ptzBindingProxy.soap_endpoint = hostname.c_str();

    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(ptzBindingProxy.soap, nullptr, username.c_str(), password.c_str()))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
        report_error(ptzBindingProxy.soap);
        return false;
    }

    _tptz__GetNodes tptz__GetNodes;
    _tptz__GetNodesResponse tptz__GetNodesResponse;

    std::cout << "====================== GetNodes ======================" << std::endl;

    /* if (ptzBindingProxy.GetNodes(&tptz__GetNodes, tptz__GetNodesResponse) == SOAP_OK)
    {
        cout << "kz " << tptz__GetNodesResponse.PTZNode.size() << endl;
        cout<<"token "<<tptz__GetNodesResponse.PTZNode[0]->token<<endl;
        cout<<"AuxiliaryCommands "<<tptz__GetNodesResponse.PTZNode[0]->AuxiliaryCommands.size()<<endl;
        cout<<"FixedHomePosition "<<tptz__GetNodesResponse.PTZNode[0]->FixedHomePosition<<endl;
        cout<<"GeoMove "<<tptz__GetNodesResponse.PTZNode[0]->GeoMove<<endl;
        cout<<"HomeSupported "<<tptz__GetNodesResponse.PTZNode[0]->HomeSupported<<endl;
        cout<<"MaximumNumberOfPresets "<<tptz__GetNodesResponse.PTZNode[0]->MaximumNumberOfPresets<<endl;

        std::cout << "\tAbsolutePanTiltPositionSpace XRange: " << printRangePtr(tptz__GetNodesResponse.PTZNode[0]->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[0]->XRange) << std::endl;
        std::cout << "\tAbsolutePanTiltPositionSpace YRange: " << printRangePtr(tptz__GetNodesResponse.PTZNode[0]->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[0]->YRange) << std::endl;


        std::cout << "\tAbsoluteZoomPositionSpace XRange: " << printRangePtr(tptz__GetNodesResponse.PTZNode[0]->SupportedPTZSpaces->AbsoluteZoomPositionSpace[0]->XRange) << std::endl;
        
        std::cout << "\tContinuousPanTiltVelocitySpace XRange: " << printRangePtr(tptz__GetNodesResponse.PTZNode[0]->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[0]->XRange) << std::endl;
        std::cout << "\tContinuousPanTiltVelocitySpace YRange: " << printRangePtr(tptz__GetNodesResponse.PTZNode[0]->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[0]->YRange) << std::endl;

        std::cout << "\tContinuousZoomVelocitySpace XRange: " << printRangePtr(tptz__GetNodesResponse.PTZNode[0]->SupportedPTZSpaces->ContinuousZoomVelocitySpace[0]->XRange) << std::endl;

        std::cout << "\tPanTiltSpeedSpace XRange: " << printRangePtr(tptz__GetNodesResponse.PTZNode[0]->SupportedPTZSpaces->PanTiltSpeedSpace[0]->XRange) << std::endl;


        std::cout << "\tRelativePanTiltTranslationSpace XRange: " << printRangePtr(tptz__GetNodesResponse.PTZNode[0]->SupportedPTZSpaces->RelativePanTiltTranslationSpace[0]->XRange) << std::endl;
        std::cout << "\tRelativePanTiltTranslationSpace YRange: " << printRangePtr(tptz__GetNodesResponse.PTZNode[0]->SupportedPTZSpaces->RelativePanTiltTranslationSpace[0]->YRange) << std::endl;

        std::cout << "\tRelativeZoomTranslationSpace XRange: " << printRangePtr(tptz__GetNodesResponse.PTZNode[0]->SupportedPTZSpaces->RelativeZoomTranslationSpace[0]->XRange) << std::endl;

        std::cout << "\t ZoomSpeedSpace XRange: " << printRangePtr(tptz__GetNodesResponse.PTZNode[0]->SupportedPTZSpaces->ZoomSpeedSpace[0]->XRange) << std::endl;

        std::cout << "\tContinuousZoomVelocitySpace XRange: " << printRangePtr(tptz__GetNodesResponse.PTZNode[0]->SupportedPTZSpaces->ContinuousZoomVelocitySpace[0]->XRange) << std::endl;
        cout<<"Name "<<tptz__GetNodesResponse.PTZNode[0]->Name->c_str()<<endl;
    }*/

    std::cout << "====================== PTZBinding Configurations ======================" << std::endl;

    _tptz__GetConfigurations tptz__GetConfigurations;
    _tptz__GetConfigurationsResponse tptz__GetConfigurationsResponse;

    if (ptzBindingProxy.GetConfigurations(&tptz__GetConfigurations, tptz__GetConfigurationsResponse) == SOAP_OK)
    {
        cout << "kz " << tptz__GetConfigurationsResponse.PTZConfiguration.size() << endl;
        std::cout << "DefaultAbsolutePantTiltPositionSpace " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->DefaultAbsolutePantTiltPositionSpace->c_str() << endl;
        std::cout << "DefaultAbsoluteZoomPositionSpace " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->DefaultAbsoluteZoomPositionSpace->c_str() << endl;
        std::cout << "DefaultContinuousPanTiltVelocitySpace " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->DefaultContinuousPanTiltVelocitySpace->c_str() << endl;
        std::cout << "DefaultContinuousZoomVelocitySpace " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->DefaultContinuousZoomVelocitySpace->c_str() << endl;
        std::cout << "DefaultPTZSpeed PanTilt x " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->DefaultPTZSpeed->PanTilt->x << endl;
        std::cout << "DefaultPTZSpeed PanTilt y " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->DefaultPTZSpeed->PanTilt->y << endl;
        std::cout << "DefaultPTZSpeed Zoom x " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->DefaultPTZSpeed->Zoom->x << endl;
        std::cout << "DefaultPTZSpeed Zoom space " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->DefaultPTZSpeed->Zoom->space->c_str() << endl;

        std::cout << "DefaultPTZTimeout " << *tptz__GetConfigurationsResponse.PTZConfiguration[0]->DefaultPTZTimeout << endl;
        std::cout << "DefaultRelativePanTiltTranslationSpace " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->DefaultRelativePanTiltTranslationSpace->c_str() << endl;
        std::cout << "DefaultRelativeZoomTranslationSpace " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->DefaultRelativeZoomTranslationSpace->c_str() << endl;
        std::cout << "MoveRamp " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->MoveRamp << endl;
        std::cout << "Name " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->Name.c_str() << endl;
        std::cout << "NodeToken " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->NodeToken.c_str() << endl;
        std::cout << "\t PanTiltLimits XRange: " << printRangePtr(tptz__GetConfigurationsResponse.PTZConfiguration[0]->PanTiltLimits->Range->XRange) << std::endl;
        std::cout << "\t PanTiltLimits YRange: " << printRangePtr(tptz__GetConfigurationsResponse.PTZConfiguration[0]->PanTiltLimits->Range->YRange) << std::endl;
        std::cout << "PresetRamp " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->PresetRamp << endl;
        std::cout << "PresetTourRamp " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->PresetTourRamp << endl;
        std::cout << "token " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->token.c_str() << endl;
        std::cout << "UseCount " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->UseCount << endl;
        std::cout << "\t PanTiltLimits XRange: " << printRangePtr(tptz__GetConfigurationsResponse.PTZConfiguration[0]->ZoomLimits->Range->XRange) << std::endl;
        std::cout << "PanTiltLimits XRange: " << tptz__GetConfigurationsResponse.PTZConfiguration[0]->ZoomLimits->Range->URI.c_str() << std::endl;
    }
}

bool Soap_wsse_add_UsernameTokenDigest(soap *soap) //подключение к прокси
{
    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(soap, nullptr, username.c_str(), password.c_str()))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
        report_error(soap);
        return false;
    }
    return true;
}

/*bool changesettingsImages()
{
    MediaBindingProxy mediaBindingProxy;
    mediaBindingProxy.soap_endpoint = "http://10.11.200.7/onvif/Media";
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

    _trt__GetVideoSources trt__GetVideoSources;
    _trt__GetVideoSourcesResponse trt__GetVideoSourcesResponse;

    if (SOAP_OK == mediaBindingProxy.GetVideoSources(&trt__GetVideoSources, trt__GetVideoSourcesResponse))
    {

        for (int i = 0; i < trt__GetVideoSourcesResponse.VideoSources.size(); i++)
        {

            ImagingBindingProxy imagingBindingProxy;
            imagingBindingProxy.soap_endpoint = "http://10.11.200.7/onvif/Imaging";

            _timg__GetOptions timg__GetOptions;
            timg__GetOptions.VideoSourceToken = trt__GetVideoSourcesResponse.VideoSources[i]->token;
            _timg__GetOptionsResponse timg__GetOptionsResponse;
            if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(imagingBindingProxy.soap, nullptr, username.c_str(), password.c_str()))
            {
                std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
                report_error(imagingBindingProxy.soap);
                return false;
            }
            std::cout << "====================== range of values ======================" << std::endl;
            if (imagingBindingProxy.GetOptions(&timg__GetOptions, timg__GetOptionsResponse) == SOAP_OK)
            {
                std::cout << "\tBrightness: " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Brightness) << std::endl;
                std::cout << "\tContrast  : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Contrast) << std::endl;
                std::cout << "\tSaturation: " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->ColorSaturation) << std::endl;
                std::cout << "\tSharpness : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Sharpness) << std::endl;

                if (timg__GetOptionsResponse.ImagingOptions->Exposure != NULL)
                {
                    std::cout << "\t WhiteBalance:" << std::endl;

                    if (timg__GetOptionsResponse.ImagingOptions->Exposure->ExposureTime != NULL)
                        std::cout << "\t MaxExposureTime : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->ExposureTime) << std::endl;

                    if (timg__GetOptionsResponse.ImagingOptions->Exposure->Gain != NULL)
                        std::cout << "\t Gain : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->Gain) << std::endl;

                    if (timg__GetOptionsResponse.ImagingOptions->Exposure->Iris != NULL)
                        std::cout << "\t Iris : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->Iris) << std::endl;

                    if (timg__GetOptionsResponse.ImagingOptions->Exposure->MaxExposureTime != NULL)
                        std::cout << "\t MaxExposureTime : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->MaxExposureTime) << std::endl;

                    if (timg__GetOptionsResponse.ImagingOptions->Exposure->MaxGain != NULL)
                        std::cout << "\t MaxGain : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->MaxGain) << std::endl;

                    if (timg__GetOptionsResponse.ImagingOptions->Exposure->MaxIris != NULL)
                        std::cout << "\t MaxIris : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->MaxIris) << std::endl;

                    if (timg__GetOptionsResponse.ImagingOptions->Exposure->MinExposureTime != NULL)
                        std::cout << "\t MinExposureTime : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->MinExposureTime) << std::endl;

                    if (timg__GetOptionsResponse.ImagingOptions->Exposure->MinGain != NULL)
                        std::cout << "\t MinGain : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->MinGain) << std::endl;

                    if (timg__GetOptionsResponse.ImagingOptions->Exposure->MinIris != NULL)
                        std::cout << "\t MinIris : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->MinIris) << std::endl;

                    for (int i = 0; i < timg__GetOptionsResponse.ImagingOptions->Exposure->Mode.size(); i++)
                    {
                        if (timg__GetOptionsResponse.ImagingOptions->Exposure->Mode[i] == tt__ExposureMode__AUTO)
                            cout << "\t\t Exposure Mode :"
                                 << "tt__ExposureMode__AUTO" << endl;
                        if (timg__GetOptionsResponse.ImagingOptions->Exposure->Mode[i] == tt__ExposureMode__MANUAL)
                            cout << "\t\t Exposure Mode :"
                                 << "tt__ExposureMode__MANUAL" << endl;
                    }

                    for (int i = 0; i < timg__GetOptionsResponse.ImagingOptions->Exposure->Priority.size(); i++)
                    {
                        std::cout << "\t WhiteBalance:" << std::endl;

                        if (timg__GetOptionsResponse.ImagingOptions->Exposure->Priority[i] == tt__ExposurePriority__LowNoise)
                            cout << "\t\t Exposure Priority : tt__ExposurePriority__LowNoise" << endl;
                        if (timg__GetOptionsResponse.ImagingOptions->Exposure->Priority[i] == tt__ExposurePriority__FrameRate)
                            cout << "\t\t Exposure Priority : tt__ExposurePriority__FrameRate" << endl;
                    }
                    // std::cout << "\t Priority : " << timg__GetOptionsResponse.ImagingOptions->Exposure->Priority[i] << std::endl;
                }

                if (timg__GetOptionsResponse.ImagingOptions->BacklightCompensation != NULL)
                {
                    std::cout << "\t BacklightCompensation:" << std::endl;
                    if (timg__GetOptionsResponse.ImagingOptions->BacklightCompensation->Level != NULL)
                        std::cout << "\tBacklightCompensation Level: " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->BacklightCompensation->Level) << std::endl;
                }

                if (timg__GetOptionsResponse.ImagingOptions->WhiteBalance != NULL)
                {
                    std::cout << "\t WhiteBalance:" << std::endl;
                    if (timg__GetOptionsResponse.ImagingOptions->WhiteBalance->YbGain != NULL)
                        std::cout << "\t WhiteBalance YbGain : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->WhiteBalance->YbGain) << std::endl;
                    if (timg__GetOptionsResponse.ImagingOptions->WhiteBalance->YrGain != NULL)
                        std::cout << "\t WhiteBalance YrGain : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->WhiteBalance->YrGain) << std::endl;
                }
            }
            else
            {
                std::cerr << "Error: GetVideoSourceConfiguration" << endl;
                report_error(mediaBindingProxy.soap);
            }
            Soap_wsse_add_UsernameTokenDigest(imagingBindingProxy.soap);

            _timg__GetImagingSettings timg__GetImagingSettings;
            timg__GetImagingSettings.VideoSourceToken = trt__GetVideoSourcesResponse.VideoSources[i]->token;
            _timg__GetImagingSettingsResponse timg__GetImagingSettingsResponse;

            std::cout << "====================== camera values ======================" << std::endl;

            if (imagingBindingProxy.GetImagingSettings(&timg__GetImagingSettings, timg__GetImagingSettingsResponse) == SOAP_OK)
            {
                std::cout << "\tBrightness  :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Brightness) << std::endl;
                std::cout << "\tContrast    :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Contrast) << std::endl;
                std::cout << "\tSaturation  :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->ColorSaturation) << std::endl;
                std::cout << "\tSharpness   :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Sharpness) << std::endl;

                if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation != NULL)
                {
                    std::cout << "\t BacklightCompensation:" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Mode == tt__BacklightCompensationMode__OFF)
                        std::cout << "\t\t Backlight Mode  : tt__BacklightCompensationMode__OFF" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Mode == tt__BacklightCompensationMode__ON)
                        std::cout << "\t\t Backlight Mode  : tt__BacklightCompensationMode__ON" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Level != NULL)
                        std::cout << "\t\t BacklightCompensation Level  :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Level) << std::endl;
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange != NULL)
                {
                    std::cout << "\t WideDynamicRange:" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Mode == tt__WideDynamicMode__OFF)
                        std::cout << "\t\t WideDynamicRange Mode  : tt__WideDynamicMode__OFF" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Mode == tt__WideDynamicMode__ON)
                        std::cout << "\t\t WideDynamicRange Mode  : tt__WideDynamicMode__ON" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Level != NULL)
                        //std::cout << "\t\t WideDynamicRange Level  :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Level) << std::endl;
                        std::cout << "\t\t WideDynamicRange Level  :" << *timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Level << std::endl;
                }

                if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance != NULL)
                {
                    std::cout << "\t WhiteBalance:" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->Mode == tt__WhiteBalanceMode__AUTO)
                        std::cout << "\t\t WhiteBalance Mode  : tt__WhiteBalanceMode__AUTO" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->Mode == tt__WhiteBalanceMode__MANUAL)
                        std::cout << "\t\t WhiteBalance Mode  : tt__WhiteBalanceMode__MANUAL" << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CbGain != NULL)
                        std::cout << "\t\t CbGain :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CbGain) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CrGain != NULL)
                        std::cout << "\t\t CbGain :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CbGain) << std::endl;
                }

                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure != NULL)
                {
                    std::cout << "\t Exposure:" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->ExposureTime != NULL)
                        std::cout << "\t\tExposureTime :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->ExposureTime) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Gain != NULL)
                        std::cout << "\t\tGain :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Gain) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Iris != NULL)
                        std::cout << "\t\tIris :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Iris) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxExposureTime != NULL)
                        std::cout << "\t\tMaxExposureTime :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxExposureTime) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxGain != NULL)
                        std::cout << "\t\tMaxGain :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxGain) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxIris != NULL)
                        std::cout << "\t\tMaxIris :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxIris) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinExposureTime != NULL)
                        std::cout << "\t\tMinExposureTime :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinExposureTime) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinGain != NULL)
                        std::cout << "\t\tMinGain :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinGain) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinIris != NULL)
                        std::cout << "\t\tMinIris :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinIris) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Mode == tt__ExposureMode__AUTO)
                        cout << "\t\t Exposure Mode :"
                             << "tt__ExposureMode__AUTO" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Mode == tt__ExposureMode__MANUAL)
                        cout << "\t\t Exposure Mode :"
                             << "tt__ExposureMode__MANUAL" << endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Priority != NULL)
                        std::cout << "\t\tPriority :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Priority) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window != 0)
                    {
                        std::cout << "\t\tWindow bottom :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->bottom) << std::endl;
                        std::cout << "\t\tWindow left :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->left) << std::endl;
                        std::cout << "\t\tWindow right :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->right) << std::endl;
                        std::cout << "\t\tWindow top :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->top) << std::endl;
                    }
                }
            }
            else
            {
                std::cerr << "Error: GetImagingSettings" << endl;
                report_error(mediaBindingProxy.soap);
            }

            tt__ImagingSettings20 ImagingSettings20;

            struct soap *soap = soap_new();
            std::cout << "====================== insertion values ======================" << std::endl;

            float temp = 50.0;
            float tempSharpness = 50.0;

            float tempExposure = 20.0;
            float tempWhiteBalance = 10.0;
            float maxExposureTime = 40000.0;
            ImagingSettings20.Brightness = &temp;
            ImagingSettings20.Contrast = &temp;
            ImagingSettings20.ColorSaturation = &temp;
            ImagingSettings20.Sharpness = &tempSharpness;

            ImagingSettings20.BacklightCompensation = soap_new_tt__BacklightCompensation20(soap, -1);
            ImagingSettings20.BacklightCompensation->Mode = tt__BacklightCompensationMode__OFF;
            if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Level != NULL)
            {
                ImagingSettings20.BacklightCompensation->Level = &tempSharpness;
                std::cout << "\tBacklightCompensation Level: " << printPtr(ImagingSettings20.BacklightCompensation->Level) << std::endl;
            }
            ImagingSettings20.WideDynamicRange = soap_new_tt__WideDynamicRange20(soap, -1);
            ImagingSettings20.WideDynamicRange->Mode = tt__WideDynamicMode__OFF;

            ImagingSettings20.Exposure = soap_new_tt__Exposure20(soap, -1);
            /*ImagingSettings20.Exposure->Mode = tt__ExposureMode__MANUAL;
            //tt__ExposureMode__AUTO
            ImagingSettings20.Exposure->ExposureTime = &tempExposure;
            ImagingSettings20.Exposure->Gain = &tempExposure;
            ImagingSettings20.Exposure->Iris = &tempExposure;
            ImagingSettings20.Exposure->Priority = soap_new_tt__ExposurePriority(soap, -1);
            *ImagingSettings20.Exposure->Priority= tt__ExposurePriority__LowNoise;
            ImagingSettings20.Exposure->MaxExposureTime = &maxExposureTime;
            ImagingSettings20.Exposure->MaxGain = &maxExposureTime;

            ImagingSettings20.WhiteBalance = soap_new_tt__WhiteBalance20(soap, -1);
            ImagingSettings20.WhiteBalance->Mode = tt__WhiteBalanceMode__AUTO;
            ImagingSettings20.WhiteBalance->CbGain = &tempWhiteBalance;
            ImagingSettings20.WhiteBalance->CrGain = &tempWhiteBalance;

            std::cout << "\tBrightness  :" << printPtr(ImagingSettings20.Brightness) << std::endl;
            std::cout << "\tContrast    :" << printPtr(ImagingSettings20.Contrast) << std::endl;
            std::cout << "\tSaturation  :" << printPtr(ImagingSettings20.ColorSaturation) << std::endl;
            std::cout << "\tSharpness   :" << printPtr(ImagingSettings20.Sharpness) << std::endl;
            std::cout << "\tBacklight Mode  :" << printMode(ImagingSettings20.BacklightCompensation) << std::endl;

            std::cout << "\tWideDynamic :" << printMode(ImagingSettings20.WideDynamicRange) << std::endl;
            std::cout << "\tExposure    :" << printMode(ImagingSettings20.Exposure) << std::endl;
            if (ImagingSettings20.Exposure != NULL)
            {
                std::cout << "\t\t MaxExposureTime :" << printPtr(ImagingSettings20.Exposure->MaxExposureTime) << std::endl;

                /* std::cout << "\t\tExposureTime :" << printPtr(ImagingSettings20.Exposure->ExposureTime) << std::endl;
                std::cout << "\t\tExposure Gain :" << printPtr(ImagingSettings20.Exposure->Gain) << std::endl;
                std::cout << "\t\tExposure Iris :" << printPtr(ImagingSettings20.Exposure->Iris) << std::endl;
                std::cout << "\t\tPriority :" << printPtr(ImagingSettings20.Exposure->Priority) << std::endl;
            }

            std::cout << "\tWhiteBalance:" << printMode(ImagingSettings20.WhiteBalance) << std::endl;
            std::cout << "\t\tWhiteBalance->CbGain :" << printPtr(ImagingSettings20.WhiteBalance->CbGain) << std::endl;
            std::cout << "\t\tWhiteBalance->CrGain :" << printPtr(ImagingSettings20.WhiteBalance->CrGain) << std::endl;

            Soap_wsse_add_UsernameTokenDigest(imagingBindingProxy.soap);

            _timg__SetImagingSettings timg__SetImagingSettings;
            timg__SetImagingSettings.VideoSourceToken = trt__GetVideoSourcesResponse.VideoSources[i]->token;
            timg__SetImagingSettings.ImagingSettings = &ImagingSettings20;

            _timg__SetImagingSettingsResponse timg__SetImagingSettingsResponse;

            if (SOAP_OK != imagingBindingProxy.SetImagingSettings(&timg__SetImagingSettings, timg__SetImagingSettingsResponse))
            {
                std::cerr << "Error: SetImagingSettings" << std::endl;
                report_error(imagingBindingProxy.soap);
                //return false;
            }

            std::cout << "====================== new camera values ======================" << std::endl;

            Soap_wsse_add_UsernameTokenDigest(imagingBindingProxy.soap);

            timg__GetImagingSettings.VideoSourceToken = trt__GetVideoSourcesResponse.VideoSources[i]->token;
            if (imagingBindingProxy.GetImagingSettings(&timg__GetImagingSettings, timg__GetImagingSettingsResponse) == SOAP_OK)
            {
                std::cout << "\tBrightness  :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Brightness) << std::endl;
                std::cout << "\tContrast    :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Contrast) << std::endl;
                std::cout << "\tSaturation  :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->ColorSaturation) << std::endl;
                std::cout << "\tSharpness   :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Sharpness) << std::endl;

                if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation != NULL)
                {
                    std::cout << "\t BacklightCompensation:" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Mode == tt__BacklightCompensationMode__OFF)
                        std::cout << "\t\t Backlight Mode  : tt__BacklightCompensationMode__OFF" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Mode == tt__BacklightCompensationMode__ON)
                        std::cout << "\t\t Backlight Mode  : tt__BacklightCompensationMode__ON" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Level != NULL)
                        std::cout << "\t\t BacklightCompensation Level  :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Level) << std::endl;
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange != NULL)
                {
                    std::cout << "\t WideDynamicRange:" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Mode == tt__WideDynamicMode__OFF)
                        std::cout << "\t\t WideDynamicRange Mode  : tt__WideDynamicMode__OFF" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Mode == tt__WideDynamicMode__ON)
                        std::cout << "\t\t WideDynamicRange Mode  : tt__WideDynamicMode__ON" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Level != NULL)
                        // std::cout << "\t\t WideDynamicRange Level  :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Level) << std::endl;
                        std::cout << "\t\t WideDynamicRange Level  :" << *timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Level << std::endl;
                }

                if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance != NULL)
                {
                    std::cout << "\t WhiteBalance:" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->Mode == tt__WhiteBalanceMode__AUTO)
                        std::cout << "\t\t WhiteBalance Mode  : tt__WhiteBalanceMode__AUTO" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->Mode == tt__WhiteBalanceMode__MANUAL)
                        std::cout << "\t\t WhiteBalance Mode  : tt__WhiteBalanceMode__MANUAL" << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CbGain != NULL)
                        std::cout << "\t\t CbGain :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CbGain) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CrGain != NULL)
                        std::cout << "\t\t CbGain :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CbGain) << std::endl;
                }

                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure != NULL)
                {
                    std::cout << "\t Exposure:" << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->ExposureTime != NULL)
                        std::cout << "\t\tExposureTime :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->ExposureTime) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Gain != NULL)
                        std::cout << "\t\tGain :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Gain) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Iris != NULL)
                        std::cout << "\t\tIris :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Iris) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxExposureTime != NULL)
                        std::cout << "\t\tMaxExposureTime :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxExposureTime) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxGain != NULL)
                        std::cout << "\t\tMaxGain :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxGain) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxIris != NULL)
                        std::cout << "\t\tMaxIris :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxIris) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinExposureTime != NULL)
                        std::cout << "\t\tMinExposureTime :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinExposureTime) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinGain != NULL)
                        std::cout << "\t\tMinGain :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinGain) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinIris != NULL)
                        std::cout << "\t\tMinIris :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinIris) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Mode == tt__ExposureMode__AUTO)
                        cout << "\t\t Exposure Mode :"
                             << "tt__ExposureMode__AUTO" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Mode == tt__ExposureMode__MANUAL)
                        cout << "\t\t Exposure Mode :"
                             << "tt__ExposureMode__MANUAL" << endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Priority != NULL)
                        std::cout << "\t\tPriority :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Priority) << std::endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window != 0)
                    {
                        std::cout << "\t\tWindow bottom :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->bottom) << std::endl;
                        std::cout << "\t\tWindow left :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->left) << std::endl;
                        std::cout << "\t\tWindow right :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->right) << std::endl;
                        std::cout << "\t\tWindow top :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->top) << std::endl;
                    }
                }
            }
            else
            {
                std::cerr << "Error: GetImagingSettings" << endl;
                report_error(mediaBindingProxy.soap);
            }
        }
    }
    else
    {
        std::cerr << "Error: GetVideoSourceConfiguration" << endl;
        report_error(mediaBindingProxy.soap);
    }
}*/

template <typename T>
void printRangePtr(T *ptr, string str)
{
    if (ptr)
    {
        if (ptr->Min || ptr->Max)
        {
            cout << str << ": [" << ptr->Min << "," << ptr->Max << "]" << endl;
        }
    }
    return;
}

template <typename T>
void printPtr(T *ptr, string str)
{
    if (ptr)
    {
        cout << str << ": " << *ptr << endl;
    }
    return;
}

template <typename T>
void printMode(T *ptr, string str)
{
    if (ptr)
    {
        cout << str << ": " << ptr->Mode << endl;
    }
    return;
}

bool changesettingsImages()
{
    MediaBindingProxy mediaBindingProxy;
    mediaBindingProxy.soap_endpoint = "http://10.24.72.33:8899/onvif/Media";
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

    _trt__GetVideoSources trt__GetVideoSources;
    _trt__GetVideoSourcesResponse trt__GetVideoSourcesResponse;

    if (SOAP_OK == mediaBindingProxy.GetVideoSources(&trt__GetVideoSources, trt__GetVideoSourcesResponse))
    {

        for (int i = 0; i < trt__GetVideoSourcesResponse.VideoSources.size(); i++)
        {
            ImagingBindingProxy imagingBindingProxy;
            imagingBindingProxy.soap_endpoint = "http://10.24.72.33:8899/onvif/imaging";

            _timg__GetOptions timg__GetOptions;
            timg__GetOptions.VideoSourceToken = trt__GetVideoSourcesResponse.VideoSources[i]->token;
            _timg__GetOptionsResponse timg__GetOptionsResponse;
            if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(imagingBindingProxy.soap, nullptr, username.c_str(), password.c_str()))
            {
                std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
                report_error(imagingBindingProxy.soap);
                return false;
            }

            std::cout << "====================== range of values ======================" << std::endl;
            if (imagingBindingProxy.GetOptions(&timg__GetOptions, timg__GetOptionsResponse) == SOAP_OK)
            {
                printRangePtr(timg__GetOptionsResponse.ImagingOptions->Brightness, "\tBrightness");
                printRangePtr(timg__GetOptionsResponse.ImagingOptions->Contrast, "\tContrast");
                printRangePtr(timg__GetOptionsResponse.ImagingOptions->ColorSaturation, "\tSaturation");
                printRangePtr(timg__GetOptionsResponse.ImagingOptions->Sharpness, "\tSharpness");

                if (timg__GetOptionsResponse.ImagingOptions->BacklightCompensation != NULL)
                {
                    std::cout << "\tBacklightCompensation:" << std::endl;
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->BacklightCompensation->Level, "\t\tBacklightCompensation Level");
                    for (int i = 0; i < timg__GetOptionsResponse.ImagingOptions->BacklightCompensation->Mode.size(); i++)
                    {
                        if (timg__GetOptionsResponse.ImagingOptions->BacklightCompensation->Mode[i] == tt__BacklightCompensationMode__OFF)
                            cout << "\t\tMode : tt__BacklightCompensationMode__OFF" << endl;
                        if (timg__GetOptionsResponse.ImagingOptions->BacklightCompensation->Mode[i] == tt__BacklightCompensationMode__ON)
                            cout << "\t\tMode : tt__BacklightCompensationMode__ON" << endl;
                    }
                }

                if (timg__GetOptionsResponse.ImagingOptions->Exposure != NULL)
                {
                    std::cout << "\tExposure:" << std::endl;
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->ExposureTime, "\t\tMaxExposureTime");
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->Gain, "\t\tGain");
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->Iris, "\t\tIris");
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->MaxExposureTime, "\t\tMaxExposureTime");
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->MaxGain, "\t\tMaxGain");
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->MaxIris, "\t\tMaxIris");
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->MinExposureTime, "\t\tMinExposureTime");
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->MinGain, "\t\tMinGain");
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->Exposure->MinIris, "\t\tMinIris");

                    for (int i = 0; i < timg__GetOptionsResponse.ImagingOptions->Exposure->Mode.size(); i++)
                    {
                        if (timg__GetOptionsResponse.ImagingOptions->Exposure->Mode[i] == tt__ExposureMode__AUTO)
                            cout << "\t\tExposure Mode :"
                                 << "tt__ExposureMode__AUTO" << endl;
                        if (timg__GetOptionsResponse.ImagingOptions->Exposure->Mode[i] == tt__ExposureMode__MANUAL)
                            cout << "\t\tExposure Mode :"
                                 << "tt__ExposureMode__MANUAL" << endl;
                    }

                    for (int i = 0; i < timg__GetOptionsResponse.ImagingOptions->Exposure->Priority.size(); i++)
                    {
                        if (timg__GetOptionsResponse.ImagingOptions->Exposure->Priority[i] == tt__ExposurePriority__LowNoise)
                            cout << "\t\tExposure Priority : tt__ExposurePriority__LowNoise" << endl;
                        if (timg__GetOptionsResponse.ImagingOptions->Exposure->Priority[i] == tt__ExposurePriority__FrameRate)
                            cout << "\t\tExposure Priority : tt__ExposurePriority__FrameRate" << endl;
                    }
                }

                if (timg__GetOptionsResponse.ImagingOptions->Focus != NULL)
                {
                    std::cout << "\t Focus:" << std::endl;
                    for (int i = 0; i < timg__GetOptionsResponse.ImagingOptions->Focus->AutoFocusModes.size(); i++)
                    {
                        if (timg__GetOptionsResponse.ImagingOptions->Focus->AutoFocusModes[i] == tt__AutoFocusMode__AUTO)
                            cout << "\t\tAutoFocusMode :"
                                 << "tt__AutoFocusMode__AUTO" << endl;
                        if (timg__GetOptionsResponse.ImagingOptions->Focus->AutoFocusModes[i] == tt__AutoFocusMode__MANUAL)
                            cout << "\t\tAutoFocusMode :"
                                 << "tt__AutoFocusMode__MANUAL" << endl;
                    }
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->Focus->DefaultSpeed, "\t\tDefaultSpeed");
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->Focus->NearLimit, "\t\tNearLimit");
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->Focus->FarLimit, "\t\tFarLimit");
                }

                for (int i = 0; i < timg__GetOptionsResponse.ImagingOptions->IrCutFilterModes.size(); i++)
                {
                    if (timg__GetOptionsResponse.ImagingOptions->IrCutFilterModes[i] == tt__IrCutFilterMode__ON)
                        cout << "\tIrCutFilterModes :"
                             << "tt__IrCutFilterMode__ON" << endl;
                    if (timg__GetOptionsResponse.ImagingOptions->IrCutFilterModes[i] == tt__IrCutFilterMode__OFF)
                        cout << "\tIrCutFilterModes :"
                             << "tt__IrCutFilterMode__OFF" << endl;
                    if (timg__GetOptionsResponse.ImagingOptions->IrCutFilterModes[i] == tt__IrCutFilterMode__AUTO)
                        cout << "\tIrCutFilterModes :"
                             << "tt__IrCutFilterMode__AUTO" << endl;
                }

                if (timg__GetOptionsResponse.ImagingOptions->WideDynamicRange != NULL)
                {
                    std::cout << "\tWideDynamicRange:" << std::endl;
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->WideDynamicRange->Level, "\t\tLevel");
                    for (int i = 0; i < timg__GetOptionsResponse.ImagingOptions->WideDynamicRange->Mode.size(); i++)
                    {
                        if (timg__GetOptionsResponse.ImagingOptions->WideDynamicRange->Mode[i] == tt__WideDynamicMode__OFF)
                            cout << "\t\tMode : tt__WideDynamicMode__OFF" << endl;
                        if (timg__GetOptionsResponse.ImagingOptions->WideDynamicRange->Mode[i] == tt__WideDynamicMode__ON)
                            cout << "\t\tMode : tt__WideDynamicMode__ON" << endl;
                    }
                }

                if (timg__GetOptionsResponse.ImagingOptions->WhiteBalance != NULL)
                {
                    std::cout << "\tWhiteBalance:" << std::endl;
                    for (int i = 0; i < timg__GetOptionsResponse.ImagingOptions->WhiteBalance->Mode.size(); i++)
                    {
                        if (timg__GetOptionsResponse.ImagingOptions->WhiteBalance->Mode[i] == tt__WhiteBalanceMode__AUTO)
                            cout << "\t\tWhiteBalanceMode :"
                                 << "tt__WhiteBalanceMode__AUTO" << endl;
                        if (timg__GetOptionsResponse.ImagingOptions->WhiteBalance->Mode[i] == tt__WhiteBalanceMode__MANUAL)
                            cout << "\t\tWhiteBalanceMode :"
                                 << "tt__WhiteBalanceMode__MANUAL" << endl;
                    }
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->WhiteBalance->YbGain, "\t\tYbGain");
                    printRangePtr(timg__GetOptionsResponse.ImagingOptions->WhiteBalance->YrGain, "\t\tYrGain");
                }
                if (timg__GetOptionsResponse.ImagingOptions->Extension != nullptr)
                {
                    std::cout << "\tExtension:" << std::endl;
                    if (timg__GetOptionsResponse.ImagingOptions->Extension->ImageStabilization != nullptr)
                    {
                        std::cout << "\t\tImageStabilization:" << std::endl;
                        printRangePtr(timg__GetOptionsResponse.ImagingOptions->Extension->ImageStabilization->Level, "\t\tLevel");
                        for (int i = 0; i < timg__GetOptionsResponse.ImagingOptions->Extension->ImageStabilization->Mode.size(); i++)
                        {
                            if (timg__GetOptionsResponse.ImagingOptions->Extension->ImageStabilization->Mode[i] == tt__ImageStabilizationMode__OFF)
                                cout << "\t\t\tMode : tt__ImageStabilizationMode__OFF" << endl;
                            if (timg__GetOptionsResponse.ImagingOptions->Extension->ImageStabilization->Mode[i] == tt__ImageStabilizationMode__ON)
                                cout << "\t\t\tMode : tt__ImageStabilizationMode__ON" << endl;
                            if (timg__GetOptionsResponse.ImagingOptions->Extension->ImageStabilization->Mode[i] == tt__ImageStabilizationMode__AUTO)
                                cout << "\t\t\tMode : tt__ImageStabilizationMode__AUTO" << endl;
                            if (timg__GetOptionsResponse.ImagingOptions->Extension->ImageStabilization->Mode[i] == tt__ImageStabilizationMode__Extended)
                                cout << "\t\t\tMode : tt__ImageStabilizationMode__Extended" << endl;
                        }
                    }
                    if (timg__GetOptionsResponse.ImagingOptions->Extension->Extension != nullptr)
                    {
                        if (timg__GetOptionsResponse.ImagingOptions->Extension->Extension->IrCutFilterAutoAdjustment != nullptr)
                        {
                            std::cout << "\t\tIrCutFilterAutoAdjustment:" << std::endl;
                            for (int i = 0; i < timg__GetOptionsResponse.ImagingOptions->Extension->Extension->IrCutFilterAutoAdjustment->BoundaryType.size(); i++)
                            {
                                std::cout << "\t\tBoundaryType:" << timg__GetOptionsResponse.ImagingOptions->Extension->Extension->IrCutFilterAutoAdjustment->BoundaryType[i] << endl;
                            }
                            printPtr(timg__GetOptionsResponse.ImagingOptions->Extension->Extension->IrCutFilterAutoAdjustment->BoundaryOffset, "\t\tBoundaryOffset");
                            printRangePtr(timg__GetOptionsResponse.ImagingOptions->Extension->Extension->IrCutFilterAutoAdjustment->ResponseTimeRange, "\t\tResponseTimeRange");
                            cout << "!!!" << endl;
                            cout << "\t\t\tLevel" << timg__GetOptionsResponse.ImagingOptions->Extension->Extension->Extension->NoiseReductionOptions->Level << endl;
                        }
                        if (timg__GetOptionsResponse.ImagingOptions->Extension->Extension->Extension != nullptr)
                        {
                            if (timg__GetOptionsResponse.ImagingOptions->Extension->Extension->Extension->ToneCompensationOptions != nullptr)
                            {
                                std::cout << "\t\tToneCompensationOptions:" << std::endl;
                                for (int i = 0; i < timg__GetOptionsResponse.ImagingOptions->Extension->Extension->Extension->ToneCompensationOptions->Mode.size(); i++)
                                {
                                    std::cout << "\t\t\tMode:" << timg__GetOptionsResponse.ImagingOptions->Extension->Extension->Extension->ToneCompensationOptions->Mode[i] << endl;
                                }
                                cout << "\t\t\tLevel" << timg__GetOptionsResponse.ImagingOptions->Extension->Extension->Extension->ToneCompensationOptions->Level << endl;
                            }
                            if (timg__GetOptionsResponse.ImagingOptions->Extension->Extension->Extension->DefoggingOptions != nullptr)
                            {
                                std::cout << "\t\tDefoggingOptions:" << std::endl;
                                for (int i = 0; i < timg__GetOptionsResponse.ImagingOptions->Extension->Extension->Extension->DefoggingOptions->Mode.size(); i++)
                                {
                                    std::cout << "\t\t\tMode:" << timg__GetOptionsResponse.ImagingOptions->Extension->Extension->Extension->DefoggingOptions->Mode[i] << endl;
                                }
                                cout << "\t\t\tLevel" << timg__GetOptionsResponse.ImagingOptions->Extension->Extension->Extension->DefoggingOptions->Level << endl;
                            }
                            if (timg__GetOptionsResponse.ImagingOptions->Extension->Extension->Extension->NoiseReductionOptions != nullptr)
                            {
                                std::cout << "\t\tNoiseReductionOptions:" << std::endl;
                                cout << "\t\t\tLevel: " << timg__GetOptionsResponse.ImagingOptions->Extension->Extension->Extension->NoiseReductionOptions->Level << endl;
                            }
                        }
                    }
                }
            }
            else
            {
                std::cerr << "Error: GetVideoSourceConfiguration" << endl;
                report_error(mediaBindingProxy.soap);
            }

            std::cout << "====================== camera values ======================" << std::endl;

            Soap_wsse_add_UsernameTokenDigest(imagingBindingProxy.soap);

            _timg__GetImagingSettings timg__GetImagingSettings;
            timg__GetImagingSettings.VideoSourceToken = trt__GetVideoSourcesResponse.VideoSources[i]->token;
            _timg__GetImagingSettingsResponse timg__GetImagingSettingsResponse;

            if (imagingBindingProxy.GetImagingSettings(&timg__GetImagingSettings, timg__GetImagingSettingsResponse) == SOAP_OK)
            {
                //Яркость
                printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Brightness, "\tBrightness");
                printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Contrast, "\tContrast");
                //Насышенность
                printPtr(timg__GetImagingSettingsResponse.ImagingSettings->ColorSaturation, "\tSaturation");
                //Резкость
                printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Sharpness, "\tSharpness");
                //Компенсация подсветки
                if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation != NULL)
                {
                    if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Mode == tt__BacklightCompensationMode__OFF)
                        cout << "\tBacklightCompensationMode:"
                             << "tt__BacklightCompensationMode__OFF" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Mode == tt__BacklightCompensationMode__ON)
                        cout << "\tBacklightCompensationMode:"
                             << "tt__BacklightCompensationMode__ON" << endl;
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Level, "\tBacklightCompensationLevel");
                }
                //Режим широкого динамического диапазона
                if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange != NULL)
                {
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Mode == tt__WideDynamicMode__ON)
                        cout << "\tWideDynamicRange:"
                             << "tt__WideDynamicMode__ON" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Mode == tt__WideDynamicMode__OFF)
                        cout << "\tWideDynamicRange:"
                             << "tt__WideDynamicMode__OFF" << endl;
                }
                //Экспозиция
                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure)
                {
                    cout << "\tExposure:" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Mode == tt__ExposureMode__AUTO)
                        cout << "\t\tExposureMode:"
                             << "tt__ExposureMode__AUTO" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Mode == tt__ExposureMode__MANUAL)
                        cout << "\t\tExposureMode:"
                             << "tt__ExposureMode__MANUAL" << endl;
                    //Приоритет
                    //LowNoise, FrameRate - низкий уровень шума и частота кадров
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Priority, "\t\tExposurePriority");
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window != nullptr)
                    {
                        //Прямоугольник
                        cout << "\t\tExposureWindow :" << endl;
                        printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->bottom, "\t\t\tExposureWindowBottom");
                        printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->left, "\t\t\tExposureWindowLeft");
                        printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->right, "\t\t\tExposureWindowRight");
                        printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->top, "\t\t\tExposureWindowTop");
                    }
                    //Время выдержки в мс.
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->ExposureTime, "\t\tExposureTime");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinExposureTime, "\t\tMinExposureTime");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxExposureTime, "\t\tMaxExposureTime");
                    //Усиление
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Gain, "\t\tGain");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinGain, "\t\tMinGain");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxGain, "\t\tMaxGain");
                    //Диафрагма
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Iris, "\t\tIris");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinIris, "\t\tMinIris");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxIris, "\t\tMaxIris");
                }

                if (timg__GetImagingSettingsResponse.ImagingSettings->Focus != nullptr)
                {
                    cout << "\tFocus :" << endl;
                    //0 или более режимов
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Focus->AFMode, "\t\tFocusAFMode");
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Focus->AutoFocusMode == tt__AutoFocusMode__MANUAL)
                        cout << "\t\tFocusAutoFocusMode :"
                             << "tt__AutoFocusMode__MANUAL" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Focus->AutoFocusMode == tt__AutoFocusMode__AUTO)
                        cout << "\t\tFocusAutoFocusMode :"
                             << "tt__AutoFocusMode__AUTO" << endl;
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Focus->DefaultSpeed, "\t\tFocusDefaultSpeed");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Focus->NearLimit, "\t\tFocusNearLimit");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Focus->FarLimit, "\t\tFocusFarLimit");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->IrCutFilter != nullptr)
                {
                    //Настройки фильтра среза инфракрасного излучения
                    if (*timg__GetImagingSettingsResponse.ImagingSettings->IrCutFilter == tt__IrCutFilterMode__AUTO)
                        cout << "\tIrCutFilter :"
                             << "tt__IrCutFilterMode__AUTO" << endl;
                    if (*timg__GetImagingSettingsResponse.ImagingSettings->IrCutFilter == tt__IrCutFilterMode__ON)
                        cout << "\tIrCutFilter :"
                             << "tt__IrCutFilterMode__ON" << endl;
                    if (*timg__GetImagingSettingsResponse.ImagingSettings->IrCutFilter == tt__IrCutFilterMode__OFF)
                        cout << "\tIrCutFilter :"
                             << "tt__IrCutFilterMode__OFF" << endl;
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance != nullptr)
                {
                    //Баланс белого
                    cout << "\tWhiteBalance" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->Mode == tt__WhiteBalanceMode__AUTO)
                        cout << "\t\tWhiteBalanceMode :"
                             << "tt__WhiteBalanceMode__AUTO" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->Mode == tt__WhiteBalanceMode__MANUAL)
                        cout << "\t\tWhiteBalanceMode :"
                             << "tt__WhiteBalanceMode__MANUAL" << endl;
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CbGain, "\t\tWhiteBalanceCbGain");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CrGain, "\t\tWhiteBalanceCrGain");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->Extension != nullptr)
                {
                    //Стабилизация
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->ImageStabilization != nullptr)
                    {
                        if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->ImageStabilization->Mode == tt__ImageStabilizationMode__ON)
                            cout << "\tExtensionImageStabilizationMode :"
                                 << "tt__ImageStabilizationMode__ON" << endl;
                        if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->ImageStabilization->Mode == tt__ImageStabilizationMode__OFF)
                            cout << "\tExtensionImageStabilizationMode :"
                                 << "tt__ImageStabilizationMode__OFF" << endl;
                        printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Extension->ImageStabilization->Level, "\tExtensionImageStabilizationLevel");
                    }
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension != nullptr)
                    {
                        for (int i = 0; i < timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->IrCutFilterAutoAdjustment.size(); i++)
                        {
                            std::cout << "\tIrCutFilterAutoAdjustment:" << std::endl;
                            if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->IrCutFilterAutoAdjustment[i]->BoundaryType.size() != 0)
                                std::cout << "\t\tBoundaryType:" << timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->IrCutFilterAutoAdjustment[i]->BoundaryType << endl;
                            printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->IrCutFilterAutoAdjustment[i]->BoundaryOffset, "\t\tBoundaryOffset");
                            printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->IrCutFilterAutoAdjustment[i]->ResponseTime, "\t\tResponseTime");
                        }
                        if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension != nullptr)
                        {
                            if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->ToneCompensation != nullptr)
                            {
                                std::cout << "\tToneCompensationOptions:" << std::endl;
                                printMode(timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->ToneCompensation, "\t\tMode");
                                printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->ToneCompensation->Level, "\t\tLevel");
                            }
                            if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->Defogging != nullptr)
                            {
                                std::cout << "\tDefoggingOptions:" << std::endl;
                                printMode(timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->Defogging, "\t\tMode");
                                printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->Defogging->Level, "\t\tLevel");
                            }
                            if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->NoiseReduction != nullptr)
                            {
                                std::cout << "\tNoiseReductionOptions:" << std::endl;
                                cout << "\t\tLevel: " << timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->NoiseReduction->Level << endl;
                            }
                        }
                    }
                }
            }
            else
            {
                std::cerr << "Error: GetImagingSettings" << endl;
                report_error(mediaBindingProxy.soap);
            }

            std::cout << "====================== insertion values ======================" << std::endl;

            struct soap *soap = soap_new();
            _timg__SetImagingSettings *timg__SetImagingSettings = soap_new__timg__SetImagingSettings(soap, -1);
            timg__SetImagingSettings->VideoSourceToken = trt__GetVideoSourcesResponse.VideoSources[i]->token;
            _timg__SetImagingSettingsResponse timg__SetImagingSettingsResponse;
            Soap_wsse_add_UsernameTokenDigest(imagingBindingProxy.soap);

            timg__SetImagingSettings->ImagingSettings = soap_new_tt__ImagingSettings20(soap, -1);

            if (timg__GetImagingSettingsResponse.ImagingSettings->Brightness)
            {
                timg__SetImagingSettings->ImagingSettings->Brightness = new float(20.0);
                printPtr(timg__SetImagingSettings->ImagingSettings->Brightness, "\tBrightness");
            }

            if (timg__GetImagingSettingsResponse.ImagingSettings->Contrast)
            {
                timg__SetImagingSettings->ImagingSettings->Contrast = new float(20.0);
                printPtr(timg__SetImagingSettings->ImagingSettings->Contrast, "\tContrast");
            }

            if (timg__GetImagingSettingsResponse.ImagingSettings->ColorSaturation)
            {
                timg__SetImagingSettings->ImagingSettings->ColorSaturation = new float(20.0);
                printPtr(timg__SetImagingSettings->ImagingSettings->ColorSaturation, "\tSaturation");
            }

            if (timg__GetImagingSettingsResponse.ImagingSettings->Sharpness)
            {
                timg__SetImagingSettings->ImagingSettings->Sharpness = new float(20.0);
                printPtr(timg__SetImagingSettings->ImagingSettings->Sharpness, "\tSharpness");
            }

            if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation)
            {
                timg__SetImagingSettings->ImagingSettings->BacklightCompensation = soap_new_tt__BacklightCompensation20(soap, -1);
                timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Mode = tt__BacklightCompensationMode__ON;
                //timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Mode = tt__BacklightCompensationMode__OFF;
                if (timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Mode == tt__BacklightCompensationMode__OFF)
                    cout << "\tBacklightCompensationMode:"
                         << "tt__BacklightCompensationMode__OFF" << endl;
                if (timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Mode == tt__BacklightCompensationMode__ON)
                    cout << "\tBacklightCompensationMode:"
                         << "tt__BacklightCompensationMode__ON" << endl;
                if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Level)
                {
                    timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Level = new float(50.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->BacklightCompensation->Level, "\tBacklightCompensationLevel");
                }
            }

            if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange)
            {
                timg__SetImagingSettings->ImagingSettings->WideDynamicRange = soap_new_tt__WideDynamicRange20(soap, -1);
                if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Mode)
                {
                    //timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Mode = tt__WideDynamicMode__ON;
                    timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Mode =  tt__WideDynamicMode__OFF;
                    cout << "\tWideDynamicRange" << timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Mode << endl;
                }

                if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Level)
                {
                    timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Level = new float(50.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Level, "\tWideDynamicRangeLevel");
                }
            }

            if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure)
            {
                cout << "\tExposure:" << endl;
                timg__SetImagingSettings->ImagingSettings->Exposure = soap_new_tt__Exposure20(soap, -1);

                timg__SetImagingSettings->ImagingSettings->Exposure->Mode = tt__ExposureMode__AUTO;

                //timg__SetImagingSettings->ImagingSettings->Exposure->Mode =  tt__ExposureMode__MANUAL;
                if (timg__SetImagingSettings->ImagingSettings->Exposure->Mode == tt__ExposureMode__AUTO)
                    cout << "\t\tExposureMode:"
                         << "tt__ExposureMode__AUTO" << endl;
                if (timg__SetImagingSettings->ImagingSettings->Exposure->Mode == tt__ExposureMode__MANUAL)
                    cout << "\t\tExposureMode:"
                         << "tt__ExposureMode__MANUAL" << endl;

                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Priority)
                {
                    tt__ExposurePriority tt__ExposurePriority = tt__ExposurePriority__LowNoise;
                    timg__SetImagingSettings->ImagingSettings->Exposure->Priority = &tt__ExposurePriority;
                    //*timg__SetImagingSettings->ImagingSettings->Exposure->Priority = tt__ExposurePriority__FrameRate;
                    printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->Priority, "\t\tExposurePriority");
                }


              /*  if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window!=NULL)
                {

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->bottom)
                    {
                        timg__SetImagingSettings->ImagingSettings->Exposure->Window->bottom = new float(0);
                        printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->Window->bottom, "\t\t\tExposureWindowBottom");
                    }
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->left)
                    {
                        timg__SetImagingSettings->ImagingSettings->Exposure->Window->left = new float(0);
                        printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->Window->left, "\t\t\tExposureWindowLeft");
                    }
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->right)
                    {
                        timg__SetImagingSettings->ImagingSettings->Exposure->Window->right = new float(0);
                        printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->Window->right, "\t\t\tExposureWindowRight");
                    }
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->top)
                    {
                        timg__SetImagingSettings->ImagingSettings->Exposure->Window->top = new float(0);
                        printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->Window->top, "\t\t\tExposureWindowTop");
                    }
                }*/

                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinExposureTime)
                {
                    timg__SetImagingSettings->ImagingSettings->Exposure->MinExposureTime = new float(10.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->MinExposureTime, "\t\tMinExposureTime");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxExposureTime)
                {
                    timg__SetImagingSettings->ImagingSettings->Exposure->MaxExposureTime = new float(40000.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->MaxExposureTime, "\t\tMaxExposureTime");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinGain)
                {
                    timg__SetImagingSettings->ImagingSettings->Exposure->MinGain = new float(0.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->MinGain, "\t\tMinGain");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxGain)
                {
                    timg__SetImagingSettings->ImagingSettings->Exposure->MaxGain = new float(100.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->MaxGain, "\t\tMaxGain");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinIris)
                {
                    timg__SetImagingSettings->ImagingSettings->Exposure->MinIris = new float(0.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->MinIris, "\t\tMinIris");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxIris)
                {
                    timg__SetImagingSettings->ImagingSettings->Exposure->MaxIris = new float(100.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->MaxIris, "\t\tMaxIris");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->ExposureTime)
                {
                    timg__SetImagingSettings->ImagingSettings->Exposure->ExposureTime = new float(40000.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->ExposureTime, "\t\tExposureTime");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Gain)
                {
                    timg__SetImagingSettings->ImagingSettings->Exposure->Gain = new float(0.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->Gain, "\t\tGain");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Iris)
                {
                    timg__SetImagingSettings->ImagingSettings->Exposure->Iris = new float(0.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->Exposure->Iris, "\t\tIris");
                }
            }

            if (timg__GetImagingSettingsResponse.ImagingSettings->Focus)
            {
                timg__SetImagingSettings->ImagingSettings->Focus = soap_new_tt__FocusConfiguration20(soap, -1);
                if (timg__GetImagingSettingsResponse.ImagingSettings->Focus->AFMode)
                {
                    timg__SetImagingSettings->ImagingSettings->Focus->AFMode = new string("");
                    printPtr(timg__SetImagingSettings->ImagingSettings->Focus->AFMode, "\t\tFocusAFMode");
                }

                timg__SetImagingSettings->ImagingSettings->Focus->AutoFocusMode = tt__AutoFocusMode__AUTO;
                //timg__SetImagingSettings->ImagingSettings->Focus->AutoFocusMode = tt__AutoFocusMode__MANUAL;
                if (timg__SetImagingSettings->ImagingSettings->Focus->AutoFocusMode == tt__AutoFocusMode__MANUAL)
                    cout << "\t\tFocusAutoFocusMode :"
                         << "tt__AutoFocusMode__MANUAL" << endl;
                if (timg__SetImagingSettings->ImagingSettings->Focus->AutoFocusMode == tt__AutoFocusMode__AUTO)
                    cout << "\t\tFocusAutoFocusMode :"
                         << "tt__AutoFocusMode__AUTO" << endl;

                if (timg__GetImagingSettingsResponse.ImagingSettings->Focus->DefaultSpeed)
                {
                    timg__SetImagingSettings->ImagingSettings->Focus->DefaultSpeed = new float(0.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->Focus->DefaultSpeed, "\t\tFocusDefaultSpeed");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->Focus->NearLimit)
                {
                    timg__SetImagingSettings->ImagingSettings->Focus->NearLimit = new float(0.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->Focus->NearLimit, "\t\tFocusNearLimit");
                }

                if (timg__GetImagingSettingsResponse.ImagingSettings->Focus->FarLimit)
                {
                    timg__SetImagingSettings->ImagingSettings->Focus->FarLimit = new float(0.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->Focus->FarLimit, "\t\tFocusFarLimit");
                }
            }

            if (timg__GetImagingSettingsResponse.ImagingSettings->IrCutFilter != nullptr)
            {
                tt__IrCutFilterMode tt__IrCutFilterMode = tt__IrCutFilterMode__ON;
                timg__SetImagingSettings->ImagingSettings->IrCutFilter = &tt__IrCutFilterMode;
                //timg__SetImagingSettings->ImagingSettings->IrCutFilter = tt__IrCutFilterMode__OFF;
                //timg__SetImagingSettings->ImagingSettings->IrCutFilter = tt__IrCutFilterMode__AUTO;
                if (*timg__SetImagingSettings->ImagingSettings->IrCutFilter == tt__IrCutFilterMode__AUTO)
                    cout << "\tIrCutFilter :"
                         << "tt__IrCutFilterMode__AUTO" << endl;
                if (*timg__SetImagingSettings->ImagingSettings->IrCutFilter == tt__IrCutFilterMode__ON)
                    cout << "\tIrCutFilter :"
                         << "tt__IrCutFilterMode__ON" << endl;
                if (*timg__SetImagingSettings->ImagingSettings->IrCutFilter == tt__IrCutFilterMode__OFF)
                    cout << "\tIrCutFilter :"
                         << "tt__IrCutFilterMode__OFF" << endl;
            }
            if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance)
            {
                timg__SetImagingSettings->ImagingSettings->WhiteBalance = soap_new_tt__WhiteBalance20(soap, -1);
                cout << "\tWhiteBalance" << endl;

                timg__SetImagingSettings->ImagingSettings->WhiteBalance->Mode = tt__WhiteBalanceMode__AUTO;
                //timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->Mode = tt__WhiteBalanceMode__MANUAL;
                if (timg__SetImagingSettings->ImagingSettings->WhiteBalance->Mode == tt__WhiteBalanceMode__AUTO)
                    cout << "\t\tWhiteBalanceMode :"
                         << "tt__WhiteBalanceMode__AUTO" << endl;
                if (timg__SetImagingSettings->ImagingSettings->WhiteBalance->Mode == tt__WhiteBalanceMode__MANUAL)
                    cout << "\t\tWhiteBalanceMode :"
                         << "tt__WhiteBalanceMode__MANUAL" << endl;

                if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CbGain)
                {
                    timg__SetImagingSettings->ImagingSettings->WhiteBalance->CbGain = new float(0.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->WhiteBalance->CbGain, "\t\tWhiteBalanceCbGain");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CrGain)
                {
                    timg__SetImagingSettings->ImagingSettings->WhiteBalance->CrGain = new float(0.0);
                    printPtr(timg__SetImagingSettings->ImagingSettings->WhiteBalance->CrGain, "\t\tWhiteBalanceCrGain");
                }
            }

            if (timg__GetImagingSettingsResponse.ImagingSettings->Extension)
            {
                timg__SetImagingSettings->ImagingSettings->Extension = soap_new_tt__ImagingSettingsExtension20(soap, -1);
                if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->ImageStabilization)
                {
                    timg__GetImagingSettingsResponse.ImagingSettings->Extension->ImageStabilization = soap_new_tt__ImageStabilization(soap, -1);

                    timg__SetImagingSettings->ImagingSettings->Extension->ImageStabilization->Mode = tt__ImageStabilizationMode__OFF;
                    //timg__SetImagingSettings->ImagingSettings->Extension->ImageStabilization->Mode = tt__ImageStabilizationMode__ON;
                    //timg__SetImagingSettings->ImagingSettings->Extension->ImageStabilization->Mode = tt__ImageStabilizationMode__AUTO;
                    //timg__SetImagingSettings->ImagingSettings->Extension->ImageStabilization->Mode = tt__ImageStabilizationMode__Extended;
                    if (timg__SetImagingSettings->ImagingSettings->Extension->ImageStabilization->Mode == tt__ImageStabilizationMode__ON)
                        cout << "\tExtensionImageStabilizationMode :"
                             << "tt__ImageStabilizationMode__ON" << endl;
                    if (timg__SetImagingSettings->ImagingSettings->Extension->ImageStabilization->Mode == tt__ImageStabilizationMode__OFF)
                        cout << "\tExtensionImageStabilizationMode :"
                             << "tt__ImageStabilizationMode__OFF" << endl;

                    if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->ImageStabilization->Level)
                    {
                        timg__SetImagingSettings->ImagingSettings->Extension->ImageStabilization->Level = new float(50.0);
                        printPtr(timg__SetImagingSettings->ImagingSettings->Extension->ImageStabilization->Level, "\tExtensionImageStabilizationLevel");
                    }
                }

                if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension)
                {
                    timg__SetImagingSettings->ImagingSettings->Extension->Extension = soap_new_tt__ImagingSettingsExtension202(soap, -1);
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->IrCutFilterAutoAdjustment.size() > 0)
                    {
                        std::cout << "\tIrCutFilterAutoAdjustment:" << std::endl;
                        tt__IrCutFilterAutoAdjustment *tt__IrCutFilterAutoAdjustment = soap_new_tt__IrCutFilterAutoAdjustment(soap, -1);
                        tt__IrCutFilterAutoAdjustment->BoundaryOffset = new float(0.0);
                        tt__IrCutFilterAutoAdjustment->ResponseTime = new int64_t(0.0);
                        tt__IrCutFilterAutoAdjustment->BoundaryType = "";
                        timg__SetImagingSettings->ImagingSettings->Extension->Extension->IrCutFilterAutoAdjustment.push_back(tt__IrCutFilterAutoAdjustment);
                        cout << "\t\tBoundaryOffset:" << tt__IrCutFilterAutoAdjustment->BoundaryOffset << endl;
                        cout << "\t\tResponseTime:" << tt__IrCutFilterAutoAdjustment->ResponseTime << endl;
                        cout << "\t\tBoundaryType:" << tt__IrCutFilterAutoAdjustment->BoundaryType << endl;
                    }
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension)
                    {
                        timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension = soap_new_tt__ImagingSettingsExtension203(soap, -1);
                        if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->ToneCompensation)
                        {
                            timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension->ToneCompensation = soap_new_tt__ToneCompensation(soap, -1);
                            std::cout << "\tToneCompensationOptions:" << std::endl;
                            if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->ToneCompensation->Mode.size() > 0)
                            {
                                timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension->ToneCompensation->Mode = "";
                                cout << "\t\tMode" << timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension->ToneCompensation->Mode << endl;
                            }
                            if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->ToneCompensation->Level)
                            {
                                timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension->ToneCompensation->Level = new float(50.0);
                                printPtr(timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension->ToneCompensation->Level, "\t\tLevel");
                            }
                        }
                        if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->Defogging)
                        {
                            timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension->Defogging = soap_new_tt__Defogging(soap, -1);
                            std::cout << "\tDefoggingOptions:" << std::endl;
                            if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->Defogging->Mode.size() > 0)
                            {
                                timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension->Defogging->Mode = "";
                                cout << "\t\tMode" << timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension->Defogging->Mode << endl;
                            }
                            if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->Defogging->Level)
                            {
                                timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension->Defogging->Level = new float(50.0);
                                printPtr(timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension->Defogging->Level, "\t\tLevel");
                            }
                        }
                        if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->NoiseReduction)
                        {
                            std::cout << "\tNoiseReductionOptions:" << std::endl;
                            timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension->NoiseReduction = soap_new_tt__NoiseReduction(soap, -1);
                            timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension->NoiseReduction->Level = 1;
                            cout << "\t\tLevel: " << timg__SetImagingSettings->ImagingSettings->Extension->Extension->Extension->NoiseReduction->Level << endl;
                        }
                    }
                }
            }

            if (imagingBindingProxy.SetImagingSettings(timg__SetImagingSettings, timg__SetImagingSettingsResponse) == SOAP_OK)
            {
                cout << "!!!" << endl;
            }
            else
            {
                report_error(imagingBindingProxy.soap);
            }

            std::cout << "====================== new camera values ======================" << std::endl;

            Soap_wsse_add_UsernameTokenDigest(imagingBindingProxy.soap);

            timg__GetImagingSettings.VideoSourceToken = trt__GetVideoSourcesResponse.VideoSources[i]->token;

            if (imagingBindingProxy.GetImagingSettings(&timg__GetImagingSettings, timg__GetImagingSettingsResponse) == SOAP_OK)
            {
                //Яркость
                printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Brightness, "\tBrightness");
                printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Contrast, "\tContrast");
                //Насышенность
                printPtr(timg__GetImagingSettingsResponse.ImagingSettings->ColorSaturation, "\tSaturation");
                //Резкость
                printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Sharpness, "\tSharpness");
                //Компенсация подсветки
                if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation != NULL)
                {
                    if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Mode == tt__BacklightCompensationMode__OFF)
                        cout << "\tBacklightCompensationMode:"
                             << "tt__BacklightCompensationMode__OFF" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Mode == tt__BacklightCompensationMode__ON)
                        cout << "\tBacklightCompensationMode:"
                             << "tt__BacklightCompensationMode__ON" << endl;
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation->Level, "\tBacklightCompensationLevel");
                }
                //Режим широкого динамического диапазона
                if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange != NULL)
                {
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Mode == tt__WideDynamicMode__ON)
                        cout << "\tWideDynamicRange:"
                             << "tt__WideDynamicMode__ON" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange->Mode == tt__WideDynamicMode__OFF)
                        cout << "\tWideDynamicRange:"
                             << "tt__WideDynamicMode__OFF" << endl;
                }
                //Экспозиция
                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure)
                {
                    cout << "\tExposure:" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Mode == tt__ExposureMode__AUTO)
                        cout << "\t\tExposureMode:"
                             << "tt__ExposureMode__AUTO" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Mode == tt__ExposureMode__MANUAL)
                        cout << "\t\tExposureMode:"
                             << "tt__ExposureMode__MANUAL" << endl;
                    //Приоритет
                    //LowNoise, FrameRate - низкий уровень шума и частота кадров
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Priority, "\t\tExposurePriority");
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window != nullptr)
                    {
                        //Прямоугольник
                        cout << "\t\tExposureWindow :" << endl;
                        printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->bottom, "\t\t\tExposureWindowBottom");
                        printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->left, "\t\t\tExposureWindowLeft");
                        printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->right, "\t\t\tExposureWindowRight");
                        printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->top, "\t\t\tExposureWindowTop");
                    }
                    //Время выдержки в мс.
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->ExposureTime, "\t\tExposureTime");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinExposureTime, "\t\tMinExposureTime");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxExposureTime, "\t\tMaxExposureTime");
                    //Усиление
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Gain, "\t\tGain");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinGain, "\t\tMinGain");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxGain, "\t\tMaxGain");
                    //Диафрагма
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Iris, "\t\tIris");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinIris, "\t\tMinIris");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxIris, "\t\tMaxIris");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->Focus != nullptr)
                {
                    cout << "\tFocus :" << endl;
                    //0 или более режимов
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Focus->AFMode, "\t\tFocusAFMode");
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Focus->AutoFocusMode == tt__AutoFocusMode__MANUAL)
                        cout << "\t\tFocusAutoFocusMode :"
                             << "tt__AutoFocusMode__MANUAL" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Focus->AutoFocusMode == tt__AutoFocusMode__AUTO)
                        cout << "\t\tFocusAutoFocusMode :"
                             << "tt__AutoFocusMode__AUTO" << endl;
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Focus->DefaultSpeed, "\t\tFocusDefaultSpeed");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Focus->NearLimit, "\t\tFocusNearLimit");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Focus->FarLimit, "\t\tFocusFarLimit");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->IrCutFilter != nullptr)
                {
                    //Настройки фильтра среза инфракрасного излучения
                    if (*timg__GetImagingSettingsResponse.ImagingSettings->IrCutFilter == tt__IrCutFilterMode__AUTO)
                        cout << "\tIrCutFilter :"
                             << "tt__IrCutFilterMode__AUTO" << endl;
                    if (*timg__GetImagingSettingsResponse.ImagingSettings->IrCutFilter == tt__IrCutFilterMode__ON)
                        cout << "\tIrCutFilter :"
                             << "tt__IrCutFilterMode__ON" << endl;
                    if (*timg__GetImagingSettingsResponse.ImagingSettings->IrCutFilter == tt__IrCutFilterMode__OFF)
                        cout << "\tIrCutFilter :"
                             << "tt__IrCutFilterMode__OFF" << endl;
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance != nullptr)
                {
                    //Баланс белого
                    cout << "\tWhiteBalance" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->Mode == tt__WhiteBalanceMode__AUTO)
                        cout << "\t\tWhiteBalanceMode :"
                             << "tt__WhiteBalanceMode__AUTO" << endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->Mode == tt__WhiteBalanceMode__MANUAL)
                        cout << "\t\tWhiteBalanceMode :"
                             << "tt__WhiteBalanceMode__MANUAL" << endl;
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CbGain, "\t\tWhiteBalanceCbGain");
                    printPtr(timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance->CrGain, "\t\tWhiteBalanceCrGain");
                }
                if (timg__GetImagingSettingsResponse.ImagingSettings->Extension != nullptr)
                {
                    //Стабилизация
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->ImageStabilization != nullptr)
                    {
                        if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->ImageStabilization->Mode == tt__ImageStabilizationMode__ON)
                            cout << "\tExtensionImageStabilizationMode :"
                                 << "tt__ImageStabilizationMode__ON" << endl;
                        if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->ImageStabilization->Mode == tt__ImageStabilizationMode__OFF)
                            cout << "\tExtensionImageStabilizationMode :"
                                 << "tt__ImageStabilizationMode__OFF" << endl;
                        printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Extension->ImageStabilization->Level, "\tExtensionImageStabilizationLevel");
                    }
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension != nullptr)
                    {
                        for (int i = 0; i < timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->IrCutFilterAutoAdjustment.size(); i++)
                        {
                            std::cout << "\tIrCutFilterAutoAdjustment:" << std::endl;
                            if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->IrCutFilterAutoAdjustment[i]->BoundaryType.size() != 0)
                                std::cout << "\t\tBoundaryType:" << timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->IrCutFilterAutoAdjustment[i]->BoundaryType << endl;
                            printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->IrCutFilterAutoAdjustment[i]->BoundaryOffset, "\t\tBoundaryOffset");
                            printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->IrCutFilterAutoAdjustment[i]->ResponseTime, "\t\tResponseTime");
                        }
                        if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension != nullptr)
                        {
                            if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->ToneCompensation != nullptr)
                            {
                                std::cout << "\tToneCompensationOptions:" << std::endl;
                                printMode(timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->ToneCompensation, "\t\tMode");
                                printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->ToneCompensation->Level, "\t\tLevel");
                            }
                            if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->Defogging != nullptr)
                            {
                                std::cout << "\tDefoggingOptions:" << std::endl;
                                printMode(timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->Defogging, "\t\tMode");
                                printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->Defogging->Level, "\t\tLevel");
                            }
                            if (timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->NoiseReduction != nullptr)
                            {
                                std::cout << "\tNoiseReductionOptions:" << std::endl;
                                cout << "\t\tLevel: " << timg__GetImagingSettingsResponse.ImagingSettings->Extension->Extension->Extension->NoiseReduction->Level << endl;
                            }
                        }
                    }
                }
            }
            else
            {
                std::cerr << "Error: GetImagingSettings" << endl;
                report_error(mediaBindingProxy.soap);
            }
        }
    }
    else
    {
        std::cerr << "Error: GetVideoSourceConfiguration" << endl;
        report_error(mediaBindingProxy.soap);
    }
}

bool checkImaging()
{
    MediaBindingProxy mediaBindingProxy;
    mediaBindingProxy.soap_endpoint = "http://10.11.200.7/onvif/Media";
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

    _trt__GetVideoSources trt__GetVideoSources;
    _trt__GetVideoSourcesResponse trt__GetVideoSourcesResponse;

    if (SOAP_OK == mediaBindingProxy.GetVideoSources(&trt__GetVideoSources, trt__GetVideoSourcesResponse))
    {

        for (int i = 0; i < trt__GetVideoSourcesResponse.VideoSources.size(); i++)
        {
            ImagingBindingProxy imagingBindingProxy;
            imagingBindingProxy.soap_endpoint = "http://10.11.200.7/onvif/Imaging";

            _timg__GetOptions timg__GetOptions;
            timg__GetOptions.VideoSourceToken = trt__GetVideoSourcesResponse.VideoSources[i]->token;
            _timg__GetOptionsResponse timg__GetOptionsResponse;
            cout << timg__GetOptions.VideoSourceToken << endl;

            Soap_wsse_add_UsernameTokenDigest(imagingBindingProxy.soap);

            std::cout << "====================== range of values ======================" << std::endl;

            if (imagingBindingProxy.GetOptions(&timg__GetOptions, timg__GetOptionsResponse) == SOAP_OK)
            {
                std::cout << "\tBrightness: " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Brightness) << std::endl;
                std::cout << "\tContrast  : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Contrast) << std::endl;
                std::cout << "\tSaturation: " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->ColorSaturation) << std::endl;
                std::cout << "\tSharpness : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->Sharpness) << std::endl;
                if (timg__GetOptionsResponse.ImagingOptions->BacklightCompensation)
                {
                    std::cout << "\tBacklightCompensation Level: " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->BacklightCompensation->Level) << std::endl;
                }
                if (timg__GetOptionsResponse.ImagingOptions->WhiteBalance)
                {
                    std::cout << "\t WhiteBalance YbGain : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->WhiteBalance->YbGain) << std::endl;
                    std::cout << "\t WhiteBalance YrGain : " << printRangePtr(timg__GetOptionsResponse.ImagingOptions->WhiteBalance->YrGain) << std::endl;
                }
            }
            else
            {
                std::cerr << "Error: GetOptions" << std::endl;
                report_error(imagingBindingProxy.soap);
            }
            Soap_wsse_add_UsernameTokenDigest(imagingBindingProxy.soap);

            _timg__GetImagingSettings timg__GetImagingSettings;
            timg__GetImagingSettings.VideoSourceToken = trt__GetVideoSourcesResponse.VideoSources[i]->token;
            _timg__GetImagingSettingsResponse timg__GetImagingSettingsResponse;
            std::cout << "====================== camera values ======================" << std::endl;

            if (imagingBindingProxy.GetImagingSettings(&timg__GetImagingSettings, timg__GetImagingSettingsResponse) == SOAP_OK)
            {
                std::cout << "\tBrightness  :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Brightness) << std::endl;
                std::cout << "\tContrast    :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Contrast) << std::endl;
                std::cout << "\tSaturation  :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->ColorSaturation) << std::endl;
                std::cout << "\tSharpness   :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Sharpness) << std::endl;
                std::cout << "\tBacklight Mode  :" << printMode(timg__GetImagingSettingsResponse.ImagingSettings->BacklightCompensation) << std::endl;
                std::cout << "\tWideDynamic :" << printMode(timg__GetImagingSettingsResponse.ImagingSettings->WideDynamicRange) << std::endl;
                std::cout << "\tExposure    :" << printMode(timg__GetImagingSettingsResponse.ImagingSettings->Exposure) << std::endl;
                if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure != 0)
                {
                    std::cout << "\t\tExposureTime :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->ExposureTime) << std::endl;
                    std::cout << "\t\tGain :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Gain) << std::endl;
                    std::cout << "\t\tIris :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Iris) << std::endl;
                    std::cout << "\t\tMaxExposureTime :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxExposureTime) << std::endl;
                    std::cout << "\t\tMaxGain :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxGain) << std::endl;
                    std::cout << "\t\tMaxIris :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MaxIris) << std::endl;
                    std::cout << "\t\tMinExposureTime :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinExposureTime) << std::endl;
                    std::cout << "\t\tMinGain :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinGain) << std::endl;
                    std::cout << "\t\tMinIris :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->MinIris) << std::endl;
                    std::cout << "\t\tExposure Mode :" << printMode(timg__GetImagingSettingsResponse.ImagingSettings->Exposure) << std::endl;
                    std::cout << "\t\tPriority :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Priority) << std::endl;
                    if (timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window != 0)
                    {
                        std::cout << "\t\tWindow bottom :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->bottom) << std::endl;
                        std::cout << "\t\tWindow left :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->left) << std::endl;
                        std::cout << "\t\tWindow right :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->right) << std::endl;
                        std::cout << "\t\tWindow top :" << printPtr(timg__GetImagingSettingsResponse.ImagingSettings->Exposure->Window->top) << std::endl;
                    }
                }
                std::cout << "\tWhiteBalance:" << printMode(timg__GetImagingSettingsResponse.ImagingSettings->WhiteBalance) << std::endl;
            }
            else
            {
                std::cerr << "Error: GetImagingSettings" << std::endl;
                report_error(imagingBindingProxy.soap);
            }
        }
    }
    else
    {
        std::cerr << "Error: GetVideoSourceConfiguration" << endl;
        report_error(mediaBindingProxy.soap);
    }
}

bool getCapabilites()
{
    /* DeviceBindingProxy proxyDevice;
    proxyDevice.soap_endpoint = hostname.c_str();

    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyDevice.soap, nullptr, username.c_str(), password.c_str()))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
        report_error(proxyDevice.soap);
        return false;
    }

    struct soap *soap = soap_new();
    _tds__GetCapabilities *tds__GetCapabilities = soap_new__tds__GetCapabilities(soap, -1);
    tds__GetCapabilities->Category.push_back(tt__CapabilityCategory__All);

    _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse = soap_new__tds__GetCapabilitiesResponse(soap, -1);

    std::cout << "====================== Get Capabilities ======================" << std::endl;

    if (SOAP_OK != proxyDevice.GetCapabilities(tds__GetCapabilities, *tds__GetCapabilitiesResponse))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
        report_error(proxyDevice.soap);
        //return false;
    }
    else
    {
        std::cout << "====================== Analytics ======================" << std::endl;

        cout << "Analytics AnalyticsModuleSupport - " << tds__GetCapabilitiesResponse->Capabilities->Analytics->AnalyticsModuleSupport << endl;
        cout << "Analytics RuleSupport - " << tds__GetCapabilitiesResponse->Capabilities->Analytics->RuleSupport << endl;
        cout << "Analytics XAddr - " << tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr.c_str() << endl;

        std::cout << "====================== Device ======================" << std::endl;

        cout << "Device Extension - " << tds__GetCapabilitiesResponse->Capabilities->Device->Extension << endl;

        if (tds__GetCapabilitiesResponse->Capabilities->Device->IO != NULL)
        {
            std::cout << "====================== IO ======================" << std::endl;

            //cout << "IO AuxiliaryCommands - " << tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->AuxiliaryCommands.capacity() << endl;
            //cout << "IO Auxiliary - " << *tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->Auxiliary << endl;
            //cout << "IO Extension - " << *tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->Extension << endl;
            cout << "IO InputConnectors - " << *tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors << endl;
            cout << "IO RelayOutputs - " << *tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs << endl;
        }
        std::cout << "====================== Network ======================" << std::endl;

        cout << "Network DynDNS - " << *tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS << endl;
        cout << "Network Dot11Configuration - " << *tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->Dot11Configuration << endl;
        cout << "Network Extension - " << tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->Extension << endl;
        cout << "Network IPFilter - " << *tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter << endl;
        cout << "Network IPVersion6 - " << *tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6 << endl;
        cout << "Network ZeroConfiguration - " << *tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration << endl;

        std::cout << "====================== Security ======================" << std::endl;

        cout << "Security AccessPolicyConfig - " << tds__GetCapabilitiesResponse->Capabilities->Device->Security->AccessPolicyConfig << endl;
        //cout << "Security Extension - " << *tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension->Extension << endl;
        cout << "Security KerberosToken - " << tds__GetCapabilitiesResponse->Capabilities->Device->Security->KerberosToken << endl;
        cout << "Security OnboardKeyGeneration - " << tds__GetCapabilitiesResponse->Capabilities->Device->Security->OnboardKeyGeneration << endl;
        cout << "Security RELToken - " << tds__GetCapabilitiesResponse->Capabilities->Device->Security->RELToken << endl;
        cout << "Security SAMLToken - " << tds__GetCapabilitiesResponse->Capabilities->Device->Security->SAMLToken << endl;

        std::cout << "====================== System ======================" << std::endl;

        cout << "System DiscoveryBye - " << tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryBye << endl;
        cout << "System DiscoveryResolve - " << tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryResolve << endl;
        cout << "System Extension - " << tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->Extension << endl;
        cout << "System HttpFirmwareUpgrade - " << *tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpFirmwareUpgrade << endl;
        cout << "System HttpSupportInformation - " << *tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSupportInformation << endl;
        cout << "System HttpSystemBackup - " << *tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemBackup << endl;
        cout << "System HttpSystemLogging - " << *tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemLogging << endl;
        cout << "System FirmwareUpgrade - " << tds__GetCapabilitiesResponse->Capabilities->Device->System->FirmwareUpgrade << endl;
        cout << "System RemoteDiscovery - " << tds__GetCapabilitiesResponse->Capabilities->Device->System->RemoteDiscovery << endl;
        //cout << "AnalyticsModuleSupport - " << tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions << endl;
        cout << "System SystemBackup - " << tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemBackup << endl;
        cout << "System SystemLogging - " << tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemLogging << endl;

        cout << "Device XAddr - " << tds__GetCapabilitiesResponse->Capabilities->Device->XAddr.c_str() << endl;

        std::cout << "====================== Events ======================" << std::endl;

        cout << "WSPausableSubscriptionManagerInterfaceSupport - " << tds__GetCapabilitiesResponse->Capabilities->Events->WSPausableSubscriptionManagerInterfaceSupport << endl;
        cout << "WSPullPointSupport - " << tds__GetCapabilitiesResponse->Capabilities->Events->WSPullPointSupport << endl;
        cout << "WSSubscriptionPolicySupport - " << tds__GetCapabilitiesResponse->Capabilities->Events->WSSubscriptionPolicySupport << endl;

        std::cout << "====================== EXTENSION ======================" << std::endl;

        std::cout << "====================== AnalyticsDevice ======================" << std::endl;

        //cout << "Extension - " << tds__GetCapabilitiesResponse->Capabilities->Extension->AnalyticsDevice->Extension << endl;
        //cout << "RuleSupport - " << tds__GetCapabilitiesResponse->Capabilities->Extension->AnalyticsDevice->RuleSupport << endl;
        //cout << "XAddr - " << tds__GetCapabilitiesResponse->Capabilities->Extension->AnalyticsDevice->XAddr.c_str() << endl;

        std::cout << "====================== DeviceIO ======================" << std::endl;
        //cout << "AudioOutputs - " << tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->AudioOutputs << endl;
        /* cout << "AudioSources - " << tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->AudioSources << endl;
        cout << "RelayOutputs - " << tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->RelayOutputs << endl;
        cout << "VideoOutputs - " << tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->VideoOutputs << endl;
        cout << "VideoSources - " << tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->VideoSources << endl;*/
    //cout << "XAddr - " << tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->XAddr.c_str() << endl;

    /* std::cout << "====================== Display ======================" << std::endl;
        cout << "FixedLayout - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Display->FixedLayout << endl;
        cout << "XAddr - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Display->XAddr.c_str() << endl;

        std::cout << "====================== Extensions ======================" << std::endl;
        cout << "Device Extension - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Extensions << endl;

        std::cout << "====================== Receiver ======================" << std::endl;

        cout << "MaximumRTSPURILength - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Receiver->MaximumRTSPURILength << endl;
        cout << "RTP_USCOREMulticast - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Receiver->RTP_USCOREMulticast << endl;
        cout << "RTP_USCORERTSP_USCORETCP - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Receiver->RTP_USCORERTSP_USCORETCP << endl;
        cout << "RTP_USCORETCP - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Receiver->RTP_USCORETCP << endl;
        cout << "SupportedReceivers - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Receiver->SupportedReceivers << endl;
        cout << "XAddr - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Receiver->XAddr.c_str() << endl;

        std::cout << "====================== Recording ======================" << std::endl;

        cout << "DynamicRecordings - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Recording->DynamicRecordings << endl;
        cout << "DynamicTracks - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Recording->DynamicTracks << endl;
        cout << "MaxStringLength - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Recording->MaxStringLength << endl;
        cout << "MediaProfileSource - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Recording->MediaProfileSource << endl;
        cout << "ReceiverSource - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Recording->ReceiverSource << endl;
        cout << "XAddr - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Recording->XAddr.c_str() << endl;

        std::cout << "====================== Replay ======================" << std::endl;

        cout << "XAddr - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Replay->XAddr.c_str() << endl;

        std::cout << "====================== Search ======================" << std::endl;

        cout << "MetadataSearch - " << tds__GetCapabilitiesResponse->Capabilities->Extension->Search->MetadataSearch << endl;*/

    /*std::cout << "====================== IMAGING ======================" << std::endl;
        cout << "XAddr - " << tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr.c_str() << endl;

        std::cout << "====================== MEDIA ======================" << std::endl;
        std::cout << "====================== Extension ======================" << std::endl;
        //cout << "MaximumNumberOfProfiles - " << tds__GetCapabilitiesResponse->Capabilities->Media->Extension->ProfileCapabilities->MaximumNumberOfProfiles << endl;

        std::cout << "====================== StreamingCapabilities ======================" << std::endl;

        cout << "Extension - " << tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->Extension << endl;
        cout << "RTP_USCORERTSP_USCORETCP - " << *tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP << endl;
        cout << "RTP_USCORETCP - " << *tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP << endl;
        cout << "RTPMulticast - " << *tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast << endl;
        cout << "XAddr - " << tds__GetCapabilitiesResponse->Capabilities->Media->XAddr.c_str() << endl;

        std::cout << "====================== PTZ ======================" << std::endl;
        cout << "XAddr - " << tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr.c_str() << endl;

        return true;
    }

    std::cout << "====================== Get Service Capabilities ======================" << std::endl;

    _tds__GetServiceCapabilities *tds__GetServiceCapabilities = soap_new__tds__GetServiceCapabilities(soap, -1);
    _tds__GetServiceCapabilitiesResponse *tds__GetServiceCapabilitiesResponse = soap_new__tds__GetServiceCapabilitiesResponse(soap, -1);

    if (SOAP_OK != proxyDevice.GetServiceCapabilities(tds__GetServiceCapabilities, *tds__GetServiceCapabilitiesResponse))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
        report_error(proxyDevice.soap);
        //return false;
    }
    else
    {

        cout << "ZeroConfiguration - " << *tds__GetServiceCapabilitiesResponse->Capabilities->Network->ZeroConfiguration << endl;
        cout << "IPVersion6 - " << *tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPVersion6 << endl;
        cout << "DynDNS - " << *tds__GetServiceCapabilitiesResponse->Capabilities->Network->DynDNS << endl;
        cout << "IPFilter - " << *tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPFilter << endl;
        cout << "DHCPv6 - " << tds__GetServiceCapabilitiesResponse->Capabilities->Network->DHCPv6 << endl;
        cout << "UsernameToken - " << *tds__GetServiceCapabilitiesResponse->Capabilities->Security->UsernameToken << endl;
        cout << "HttpDigest - " << *tds__GetServiceCapabilitiesResponse->Capabilities->Security->HttpDigest << endl;
        cout << "MaxUsers - " << tds__GetServiceCapabilitiesResponse->Capabilities->Security->MaxUsers << endl;
        cout << "DefaultAccessPolicy - " << tds__GetServiceCapabilitiesResponse->Capabilities->Security->DefaultAccessPolicy << endl;
        cout << "RemoteUserHandling - " << *tds__GetServiceCapabilitiesResponse->Capabilities->Security->RemoteUserHandling << endl;
        cout << "MaxUserNameLength - " << tds__GetServiceCapabilitiesResponse->Capabilities->Security->MaxUserNameLength << endl;
        cout << "MaxPasswordLength - " << tds__GetServiceCapabilitiesResponse->Capabilities->Security->MaxPasswordLength << endl;
        cout << "DiscoveryBye - " << *tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryBye << endl;
        cout << "SystemLogging - " << *tds__GetServiceCapabilitiesResponse->Capabilities->System->SystemLogging << endl;
        cout << "HttpFirmwareUpgrade - " << *tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpFirmwareUpgrade << endl;
        cout << "HttpSystemBackup - " << *tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSystemBackup << endl;
        cout << "HttpSystemLogging - " << *tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSystemLogging << endl;
        cout << "HttpSupportInformation - " << *tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSupportInformation << endl;

        //if (tds__GetServiceCapabilitiesResponse->Capabilities->Security->MaxUsers != NULL)

        //if (*tds__GetServiceCapabilitiesResponse->Capabilities->Security->RemoteUserHandling != false)

        //if (tds__GetServiceCapabilitiesResponse->Capabilities->Security->MaxUserNameLength != NULL)

        //if (*tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryBye == false)

        return true;
    }*/
}

bool testPtz(const string &profileToken, _ocp_PTZStatus &ptzStatus, const float &pantiltX, const float &pantiltY, const float &zoom, const float &speedPTX, const float &speedPTY, const float &speedZoom)
{

    PTZBindingProxy ptzBindingProxy;
    ptzBindingProxy.soap_endpoint = hostname.c_str();
    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(ptzBindingProxy.soap, nullptr, username.c_str(), password.c_str()))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
        report_error(ptzBindingProxy.soap);
        return false;
    }

    std::cout << "====================== tptz__RelativeMove ======================" << std::endl;

    struct soap *soap = soap_new();
    _tptz__RelativeMove tptz__RelativeMove;
    _tptz__RelativeMoveResponse tptz__RelativeMoveResponse;
    tptz__RelativeMove.ProfileToken = profileToken;

    tt__PTZVector Translation;
    Translation.PanTilt = soap_new_tt__Vector2D(soap, -1);
    Translation.PanTilt->x = pantiltX;
    Translation.PanTilt->y = pantiltY;
    Translation.Zoom = soap_new_tt__Vector1D(soap, -1);
    Translation.Zoom->x = zoom;
    tptz__RelativeMove.Translation = &Translation;

    tt__PTZSpeed Speed;
    Speed.PanTilt = soap_new_tt__Vector2D(soap, -1);
    Speed.PanTilt->x = speedPTX;
    Speed.PanTilt->y = speedPTY;
    Speed.Zoom = soap_new_tt__Vector1D(soap, -1);
    Speed.Zoom->x = speedZoom;
    tptz__RelativeMove.Speed = &Speed;

    bool executeResult = false;
    if (SOAP_OK == ptzBindingProxy.RelativeMove(&tptz__RelativeMove, tptz__RelativeMoveResponse))
    {
        executeResult = true;
    }
    else
    {
        std::cerr << "Error: absoluteMove" << endl;
        report_error(ptzBindingProxy.soap);
    }

    CLEANUP_SOAP(soap);
    return true;
}

bool ptz(const std::string &profileToken)
{
    PTZBindingProxy ptzBindingProxy;
    ptzBindingProxy.soap_endpoint = hostname.c_str();
    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(ptzBindingProxy.soap, nullptr, username.c_str(), password.c_str()))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
        report_error(ptzBindingProxy.soap);
        return false;
    }

    _tptz__GetPresets tptz__GetPresets;
    tptz__GetPresets.ProfileToken = profileToken;
    _tptz__GetPresetsResponse tptz__GetPresetsResponse;
    if (SOAP_OK == ptzBindingProxy.GetPresets(&tptz__GetPresets, tptz__GetPresetsResponse))
    {
        cout << "tptz__GetPresetsResponse.Preset.size " << tptz__GetPresetsResponse.Preset.size() << endl;
        cout << "tptz__GetPresetsResponse.Preset[0]->Name " << tptz__GetPresetsResponse.Preset[0]->Name->c_str() << endl;
        cout << "PTZPosition->PanTilt->x " << tptz__GetPresetsResponse.Preset[0]->PTZPosition->PanTilt->x << endl;
        cout << "PTZPosition->PanTilt->y " << tptz__GetPresetsResponse.Preset[0]->PTZPosition->PanTilt->y << endl;
        cout << "Zoom->x " << tptz__GetPresetsResponse.Preset[0]->PTZPosition->Zoom->x << endl;
        cout << "token " << tptz__GetPresetsResponse.Preset[0]->token->c_str() << endl;
        cout << "tptz__GetPresetsResponse.Preset[0]->Name " << tptz__GetPresetsResponse.Preset[1]->Name->c_str() << endl;
        cout << "PTZPosition->PanTilt->x " << tptz__GetPresetsResponse.Preset[1]->PTZPosition->PanTilt->x << endl;
        cout << "PTZPosition->PanTilt->y " << tptz__GetPresetsResponse.Preset[1]->PTZPosition->PanTilt->y << endl;
        cout << "Zoom->x " << tptz__GetPresetsResponse.Preset[1]->PTZPosition->Zoom->x << endl;
        cout << "token " << tptz__GetPresetsResponse.Preset[1]->token->c_str() << endl;
    }
    else
    {
        std::cerr << "Error: GetPresets" << endl;
        report_error(ptzBindingProxy.soap);
    }

    std::string str = tptz__GetPresetsResponse.Preset[0]->token->c_str();
    _tptz__SetPreset tptz__SetPreset;
    _tptz__SetPresetResponse tptz__SetPresetResponse;
    tptz__SetPreset.ProfileToken = profileToken;
    cout << "str " << str << &str << endl;

    tptz__SetPreset.PresetToken = &str;
    cout << "tptz__SetPreset.PresetToken " << *tptz__SetPreset.PresetToken << endl;

    std::cout << "============================================" << std::endl;

    if (SOAP_OK == ptzBindingProxy.SetPreset(&tptz__SetPreset, tptz__SetPresetResponse))
    {
        cout << "tptz__SetPresetResponse.PresetToken.size " << tptz__SetPresetResponse.PresetToken.size() << endl;
        cout << "tptz__SetPresetResponse.PresetToken[0] " << tptz__SetPresetResponse.PresetToken[0] << endl;
    }
    else
    {
        std::cerr << "Error: absoluteMove" << endl;
        report_error(ptzBindingProxy.soap);
    }
    std::cout << "============================================" << std::endl;

    if (SOAP_OK == ptzBindingProxy.GetPresets(&tptz__GetPresets, tptz__GetPresetsResponse))
    {
        cout << "tptz__GetPresetsResponse.Preset.size " << tptz__GetPresetsResponse.Preset.size() << endl;
        cout << "tptz__GetPresetsResponse.Preset[0]->Name " << tptz__GetPresetsResponse.Preset[0]->Name->c_str() << endl;
        cout << "PTZPosition->PanTilt->x " << tptz__GetPresetsResponse.Preset[0]->PTZPosition->PanTilt->x << endl;
        cout << "PTZPosition->PanTilt->y " << tptz__GetPresetsResponse.Preset[0]->PTZPosition->PanTilt->y << endl;
        cout << "Zoom->x " << tptz__GetPresetsResponse.Preset[0]->PTZPosition->Zoom->x << endl;
        cout << "token " << tptz__GetPresetsResponse.Preset[0]->token->c_str() << endl;
        cout << "tptz__GetPresetsResponse.Preset[0]->Name " << tptz__GetPresetsResponse.Preset[1]->Name->c_str() << endl;
        cout << "PTZPosition->PanTilt->x " << tptz__GetPresetsResponse.Preset[1]->PTZPosition->PanTilt->x << endl;
        cout << "PTZPosition->PanTilt->y " << tptz__GetPresetsResponse.Preset[1]->PTZPosition->PanTilt->y << endl;
        cout << "Zoom->x " << tptz__GetPresetsResponse.Preset[1]->PTZPosition->Zoom->x << endl;
        cout << "token " << tptz__GetPresetsResponse.Preset[1]->token->c_str() << endl;
    }

    absoluteMove(profileToken, tptz__GetPresetsResponse.Preset[0]->PTZPosition->PanTilt->x, tptz__GetPresetsResponse.Preset[0]->PTZPosition->PanTilt->y, 0, 1, 0, 0);
}

bool GetVideoEncoderConfigurations(MediaBindingProxy mediaBindingProxy, soap *soap) //media информация
{
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);
    std::cout << "-------------------VideoEncoderConfigurations-------------------" << endl;

    _trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations = soap_new__trt__GetVideoEncoderConfigurations(soap, -1);
    _trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse = soap_new__trt__GetVideoEncoderConfigurationsResponse(soap, -1);

    if (SOAP_OK == mediaBindingProxy.GetVideoEncoderConfigurations(trt__GetVideoEncoderConfigurations, *trt__GetVideoEncoderConfigurationsResponse))
    {
        cout << endl;
        for (int i = 0; i < trt__GetVideoEncoderConfigurationsResponse->Configurations.size(); i++)
        {
            Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

            _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration = soap_new__trt__GetVideoEncoderConfiguration(soap, -1);
            _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse = soap_new__trt__GetVideoEncoderConfigurationResponse(soap, -1);

            trt__GetVideoEncoderConfiguration->ConfigurationToken = trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->token;
            cout << "token" << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->token << endl;

            if (SOAP_OK == mediaBindingProxy.GetVideoEncoderConfiguration(trt__GetVideoEncoderConfiguration, *trt__GetVideoEncoderConfigurationResponse))
            {
            }
            else
            {
                std::cerr << "Error: GetVideoEncoderConfiguration" << endl;
                report_error(mediaBindingProxy.soap);
            }
            cout << "\tname: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Name.c_str() << "  UseCount: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->UseCount << "  token: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->token.c_str() << endl;
            cout << "\t GuaranteedFrameRate: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->GuaranteedFrameRate << endl;
            string VideoEncoding = (trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__VideoEncoding__JPEG) ? "tt__VideoEncoding__JPEG" : (trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__VideoEncoding__MPEG4) ? "tt__VideoEncoding__MPEG4"
                                                                                                                                                                  : (trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__VideoEncoding__H264)    ? "tt__VideoEncoding__H264"
                                                                                                                                                                                                                                                                            : "Error VideoEncoding";
            cout << "\tEncoding: " << VideoEncoding << endl;

            cout << "\t Resolution  Width: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Resolution->Width << "  Height: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Resolution->Height << endl;
            cout << "\t Quality: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Quality << endl;
            cout << "\t RateControl: " << endl;
            cout << "\t\t FrameRateLimit: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->RateControl->FrameRateLimit << endl;
            cout << "\t\t EncodingInterval: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->RateControl->EncodingInterval << endl;
            cout << "\t\t BitrateLimit: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->RateControl->BitrateLimit << endl;

            if (trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->MPEG4 != 0)
            {
                cout << "\t MPEG4: " << endl;
                cout << "\t\t GovLength: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->MPEG4->GovLength << endl;
                cout << "\t\t Mpeg4Profile: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->MPEG4->Mpeg4Profile << endl;
            }
            else
                cout << "\t MPEG4: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->MPEG4 << endl;

            if (trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->H264 != 0)
            {
                cout << "\t H264Profile: " << endl;
                cout << "\t\t GovLength: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->H264->GovLength << endl;
                cout << "\t\t H264Profile: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->H264->H264Profile << endl;
            }
            else
                cout << "\t H264: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->H264 << endl;

            cout << "\t Multicast: " << endl;
            cout << "\t\t Address Type: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Multicast->Address->Type << endl;
            cout << "\t\t Address IPv4Address: " << *trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Multicast->Address->IPv4Address << endl;
            cout << "\t\t Address IPv6Address: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Multicast->Address->IPv6Address << endl;
            cout << "\t\t Port: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Multicast->Port << endl;
            cout << "\t\t Port: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Multicast->TTL << endl;
            cout << "\t\t Port: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Multicast->AutoStart << endl;

            cout << "\t SessionTimeout: " << trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->SessionTimeout << endl;

            Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

            _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions = soap_new__trt__GetVideoEncoderConfigurationOptions(soap, -1);
            _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse = soap_new__trt__GetVideoEncoderConfigurationOptionsResponse(soap, -1);

            trt__GetVideoEncoderConfigurationOptions->ConfigurationToken = &trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->token;
            std::cout << "      -------GetVideoEncoderConfigurationOptions  token " << *trt__GetVideoEncoderConfigurationOptions->ConfigurationToken << endl;

            if (SOAP_OK == mediaBindingProxy.GetVideoEncoderConfigurationOptions(trt__GetVideoEncoderConfigurationOptions, *trt__GetVideoEncoderConfigurationOptionsResponse))
            {
                cout << "\t GuaranteedFrameRateSupported: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->GuaranteedFrameRateSupported << endl;
                cout << "\t QualityRange Max: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Max << " min: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Min << endl;

                if (trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG != 0)
                {
                    cout << "\t JPEG: " << endl;
                    for (int j = 0; j < trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable.size(); j++)
                    {
                        cout << "\t\t ResolutionsAvailable Width: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[j]->Width << endl;
                        cout << "\t\t ResolutionsAvailable Height: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[j]->Height << endl
                             << endl;
                    }

                    cout << "\t\t FrameRateRange Max: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->FrameRateRange->Max << endl;
                    cout << "\t\t FrameRateRange Min: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->FrameRateRange->Min << endl
                         << endl;

                    cout << "\t\t EncodingIntervalRange Max: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->EncodingIntervalRange->Max << endl;
                    cout << "\t\t EncodingIntervalRange Min: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->EncodingIntervalRange->Min << endl
                         << endl;
                }
                else
                    cout << "\t JPEG: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG << endl;

                if (trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4 != 0)
                {
                    cout << "\t MPEG4: " << endl;
                    for (int j = 0; j < trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->ResolutionsAvailable.size(); j++)
                    {
                        cout << "\t\t ResolutionsAvailable Width: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->ResolutionsAvailable[j]->Width << endl;
                        cout << "\t\t ResolutionsAvailable Height: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->ResolutionsAvailable[j]->Height << endl
                             << endl;
                    }
                    cout << "\t\t GovLengthRange Max: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->GovLengthRange->Max << endl;
                    cout << "\t\t GovLengthRange Min: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->GovLengthRange->Min << endl
                         << endl;
                    cout << "\t\t FrameRateRange Max: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->FrameRateRange->Max << endl;
                    cout << "\t\t FrameRateRange Min: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->FrameRateRange->Min << endl
                         << endl;

                    cout << "\t\t EncodingIntervalRange Max: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->EncodingIntervalRange->Max << endl;
                    cout << "\t\t EncodingIntervalRange Min: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->EncodingIntervalRange->Min << endl
                         << endl;
                    for (int j = 0; j < trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->Mpeg4ProfilesSupported.size(); j++)
                        cout << "\t\t ResolutionsAvailable Width: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4->Mpeg4ProfilesSupported[j] << endl;
                }
                else
                    cout << "\t MPEG4: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->MPEG4 << endl;

                if (trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264 != 0)
                {
                    cout << "\t H264: " << endl;
                    for (int j = 0; j < trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable.size(); j++)
                    {
                        cout << "\t\t ResolutionsAvailable Width: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[j]->Width << endl;
                        cout << "\t\t ResolutionsAvailable Height: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable[j]->Height << endl
                             << endl;
                    }
                    cout << "\t\t GovLengthRange Max: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Max << endl;
                    cout << "\t\t GovLengthRange Min: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Min << endl
                         << endl;
                    cout << "\t\t FrameRateRange Max: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Max << endl;
                    cout << "\t\t FrameRateRange Min: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Min << endl
                         << endl;

                    cout << "\t\t EncodingIntervalRange Max: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Max << endl;
                    cout << "\t\t EncodingIntervalRange Min: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Min << endl
                         << endl;
                    for (int j = 0; j < trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported.size(); j++)
                        cout << "\t\t ResolutionsAvailable Width: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported[j] << endl;
                }
                else
                    cout << "\t H264: " << trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264 << endl;
            }
            else
            {
                std::cerr << "Error: GetVideoEncoderConfigurationOptions" << endl;
                report_error(mediaBindingProxy.soap);
            }
            cout << endl
                 << "           ************" << endl;
        }
    }
    else
    {
        std::cerr << "Error: GetVideoEncoderConfigurations" << endl;
        report_error(mediaBindingProxy.soap);
    }
}

bool GetVideoAnalyticsConfigurations(MediaBindingProxy mediaBindingProxy) //media информация
{
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

    _trt__GetVideoAnalyticsConfigurations trt__GetVideoAnalyticsConfigurations;
    _trt__GetVideoAnalyticsConfigurationsResponse trt__GetVideoAnalyticsConfigurationsResponse;

    if (SOAP_OK == mediaBindingProxy.GetVideoAnalyticsConfigurations(&trt__GetVideoAnalyticsConfigurations, trt__GetVideoAnalyticsConfigurationsResponse))
    {
        std::cout << "-------------------GetVideoAnalyticsConfigurations-------------------" << endl;
        std::cout << " Analytics Configurations size - " << trt__GetVideoAnalyticsConfigurationsResponse.Configurations.size() << endl;
        for (int i = 0; i < trt__GetVideoAnalyticsConfigurationsResponse.Configurations.size(); i++)
        {

            _trt__GetVideoAnalyticsConfiguration trt__GetVideoAnalyticsConfiguration;
            _trt__GetVideoAnalyticsConfigurationResponse trt__GetVideoAnalyticsConfigurationResponse;

            trt__GetVideoAnalyticsConfiguration.ConfigurationToken = trt__GetVideoAnalyticsConfigurationsResponse.Configurations[i]->token;
            Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

            if (SOAP_OK == mediaBindingProxy.GetVideoAnalyticsConfiguration(&trt__GetVideoAnalyticsConfiguration, trt__GetVideoAnalyticsConfigurationResponse))
            {
                cout << endl;
                cout << "\tConfiguration Name: " << trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].Name << endl;

                for (int j = 0; j < trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].AnalyticsEngineConfiguration->AnalyticsModule.size(); j++)
                {
                    cout << "\tAnalyticsModule name: " << trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].AnalyticsEngineConfiguration->AnalyticsModule[j]->Name << endl;
                    cout << "\tAnalyticsModule Type: " << trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].AnalyticsEngineConfiguration->AnalyticsModule[j]->Type << endl;

                    if (trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].AnalyticsEngineConfiguration->AnalyticsModule[j]->Parameters->SimpleItem.size() != 0)
                        cout << "\tAnalyticsModule Parameters SimpleItem: " << endl;
                    for (int k = 0; k < trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].AnalyticsEngineConfiguration->AnalyticsModule[j]->Parameters->SimpleItem.size(); k++)
                    {
                        cout << "\t\tName: " << trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].AnalyticsEngineConfiguration->AnalyticsModule[j]->Parameters->SimpleItem[k].Name << endl;
                        cout << "\t\tValue: " << trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].AnalyticsEngineConfiguration->AnalyticsModule[j]->Parameters->SimpleItem[k].Value << endl;
                        cout << endl;
                    }

                    if (trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].AnalyticsEngineConfiguration->AnalyticsModule[j]->Parameters->ElementItem.size() != 0)
                        cout << "\tAnalyticsModule Parameters ElementItem: " << endl;
                    for (int k = 0; k < trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].AnalyticsEngineConfiguration->AnalyticsModule[j]->Parameters->ElementItem.size(); k++)
                    {
                        cout << "\t\tName: " << trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].AnalyticsEngineConfiguration->AnalyticsModule[j]->Parameters->ElementItem[k].Name << endl;
                        cout << endl;
                    }

                    if (trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].RuleEngineConfiguration->Rule.size() != 0)
                        cout << "\tRuleEngineConfiguration: " << endl;

                    cout << "\t\t name: " << trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].RuleEngineConfiguration->Rule[j]->Name << endl;
                    cout << "\t\t Type: " << trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].RuleEngineConfiguration->Rule[j]->Type << endl;

                    /////////////////////
                    if (trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].RuleEngineConfiguration->Rule[j]->Parameters->SimpleItem.size() != 0)
                        cout << "\tRuleEngineConfiguration Parameters SimpleItem: " << endl;

                    for (int k = 0; k < trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].RuleEngineConfiguration->Rule[j]->Parameters->SimpleItem.size(); k++)
                    {
                        cout << "\t\tName: " << trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].RuleEngineConfiguration->Rule[j]->Parameters->SimpleItem[k].Name << endl;
                        cout << "\t\tValue: " << trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].RuleEngineConfiguration->Rule[j]->Parameters->SimpleItem[k].Value << endl;
                        cout << endl;
                    }
                    if (trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].RuleEngineConfiguration->Rule[j]->Parameters->ElementItem.size() != 0)
                        cout << "\tRuleEngineConfiguration Parameters ElementItem: " << endl;

                    for (int k = 0; k < trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].RuleEngineConfiguration->Rule[j]->Parameters->ElementItem.size(); k++)
                    {

                        cout << "\t\tName: " << trt__GetVideoAnalyticsConfigurationResponse.Configuration[i].RuleEngineConfiguration->Rule[j]->Parameters->ElementItem[k].Name << endl;
                        cout << endl;
                    }

                    cout << endl;
                }
            }
            else
            {
                std::cerr << "Error: GetVideoAnalyticsConfiguration" << endl;
                report_error(mediaBindingProxy.soap);
            }
            cout << endl;
        }
    }
    else
    {
        std::cerr << "Error: GetVideoAnalyticsConfigurations" << endl;
        report_error(mediaBindingProxy.soap);
    }
}

bool GetVideoSourceConfiguration(MediaBindingProxy mediaBindingProxy) //media информация
{
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);
    std::cout << "-------------------GetVideoSourceConfigurations-------------------" << endl;

    _trt__GetVideoSourceConfigurations trt__GetVideoSourceConfigurations;
    _trt__GetVideoSourceConfigurationsResponse trt__GetVideoSourceConfigurationsResponse;
    std::string VideoSourceToken;
    if (SOAP_OK == mediaBindingProxy.GetVideoSourceConfigurations(&trt__GetVideoSourceConfigurations, trt__GetVideoSourceConfigurationsResponse))
    {
        for (int i = 0; i < trt__GetVideoSourceConfigurationsResponse.Configurations.size(); i++)
        {
            Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

            _trt__GetVideoSourceConfiguration trt__GetVideoSourceConfiguration;
            _trt__GetVideoSourceConfigurationResponse trt__GetVideoSourceConfigurationResponse;

            trt__GetVideoSourceConfiguration.ConfigurationToken = trt__GetVideoSourceConfigurationsResponse.Configurations[i]->token;

            if (SOAP_OK == mediaBindingProxy.GetVideoSourceConfiguration(&trt__GetVideoSourceConfiguration, trt__GetVideoSourceConfigurationResponse))
            {

                cout << "\tName: " << trt__GetVideoSourceConfigurationResponse.Configuration->Name << endl;
                cout << "\ttoken: " << trt__GetVideoSourceConfigurationResponse.Configuration->token << endl;
                VideoSourceToken = trt__GetVideoSourceConfigurationResponse.Configuration->token;
                cout << "\tUseCount: " << trt__GetVideoSourceConfigurationResponse.Configuration->UseCount << endl;
                cout << "\tViewMode: " << trt__GetVideoSourceConfigurationResponse.Configuration->ViewMode << endl;
                cout << "\tSourceToken: " << trt__GetVideoSourceConfigurationResponse.Configuration->SourceToken << endl;
                cout << "\tBounds: " << endl;
                cout << "\t\theight: " << trt__GetVideoSourceConfigurationResponse.Configuration->Bounds->height << endl;
                cout << "\t\twidth: " << trt__GetVideoSourceConfigurationResponse.Configuration->Bounds->width << endl;
                cout << "\t\tx: " << trt__GetVideoSourceConfigurationResponse.Configuration->Bounds->x << endl;
                cout << "\t\ty: " << trt__GetVideoSourceConfigurationResponse.Configuration->Bounds->y << endl;
                if (trt__GetVideoSourceConfigurationResponse.Configuration->Extension != 0)
                {
                    cout << "\tExtension: " << endl;
                    if (trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Rotate != NULL)
                    {
                        cout << "\t\tRotate Degree: " << trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Rotate->Degree << endl;
                        cout << "\t\tRotate Mode: " << trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Rotate->Mode << endl;
                    }
                    if (trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Extension != 0)
                    {
                        cout << "\t\tExtension2: " << endl;
                        for (int j = 0; j < trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Extension->LensDescription.size(); j++)
                        {
                            cout << "\t\t\tLensDescription FocalLength: " << trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Extension->LensDescription[j]->FocalLength << endl;
                            cout << "\t\t\tLensDescription Offset x: " << trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Extension->LensDescription[j]->Offset->x << endl;
                            cout << "\t\t\tLensDescription Offset y: " << trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Extension->LensDescription[j]->Offset->y << endl;
                            cout << "\t\t\tLensDescription Projection: " << endl;
                            for (int k = 0; k < trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Extension->LensDescription.size(); k++)
                            {
                                cout << "\t\t\t\t : " << trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Extension->LensDescription[j]->Projection[k]->Angle << endl;
                                cout << "\t\t\t\t : " << trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Extension->LensDescription[j]->Projection[k]->Radius << endl;
                                cout << "\t\t\t\t : " << trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Extension->LensDescription[j]->Projection[k]->Transmittance << endl
                                     << endl;
                            }

                            cout << "\t\t\tLensDescription XFactor: " << trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Extension->LensDescription[j]->XFactor << endl
                                 << endl;
                        }
                        cout << "\t\tSceneOrientation Mode: " << trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Extension->SceneOrientation->Mode << endl;
                        cout << "\t\tSceneOrientation Orientation: " << trt__GetVideoSourceConfigurationResponse.Configuration->Extension->Extension->SceneOrientation->Orientation << endl;
                    }
                }

                std::cout << "             -------GetVideoSourceModes  token:  " << VideoSourceToken << " --------" << endl;
                _trt__GetVideoSourceModes trt__GetVideoSourceModes;
                _trt__GetVideoSourceModesResponse trt__GetVideoSourceModesResponse;
                Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

                trt__GetVideoSourceModes.VideoSourceToken = trt__GetVideoSourceConfigurationResponse.Configuration->token;
                cout << trt__GetVideoSourceConfigurationResponse.Configuration->token << endl;
                if (SOAP_OK == mediaBindingProxy.GetVideoSourceModes(&trt__GetVideoSourceModes, trt__GetVideoSourceModesResponse))
                {
                    for (int j = 0; j < trt__GetVideoSourceModesResponse.VideoSourceModes.size(); j++)
                    {
                        cout << "\ttoken: " << trt__GetVideoSourceModesResponse.VideoSourceModes[j]->token << endl;
                        cout << "\tEnabled: " << *trt__GetVideoSourceModesResponse.VideoSourceModes[j]->Enabled << endl;
                        cout << "\tMaxFramerate: " << trt__GetVideoSourceModesResponse.VideoSourceModes[j]->MaxFramerate << endl;
                        cout << "\tMaxResolution Height: " << trt__GetVideoSourceModesResponse.VideoSourceModes[j]->MaxResolution->Height << endl;
                        cout << "\tMaxResolution Width: " << trt__GetVideoSourceModesResponse.VideoSourceModes[j]->MaxResolution->Width << endl;
                        cout << "\t Encodings: " << trt__GetVideoSourceModesResponse.VideoSourceModes[j]->Encodings << endl;
                        cout << "\t Reboot: " << trt__GetVideoSourceModesResponse.VideoSourceModes[j]->Reboot << endl;
                        cout << "\t Description: " << *trt__GetVideoSourceModesResponse.VideoSourceModes[j]->Description << endl
                             << endl;
                    }
                }
                else
                {
                    std::cerr << "Error: GetVideoSourceModes" << endl;
                    report_error(mediaBindingProxy.soap);
                }
            }
            else
            {
                std::cerr << "Error: GetVideoSourceConfiguration" << endl;
                report_error(mediaBindingProxy.soap);
            }
        }
    }
    else
    {
        std::cerr << "Error: GetVideoSourceConfiguration" << endl;
        report_error(mediaBindingProxy.soap);
    }
}

bool GetVideoSources(MediaBindingProxy mediaBindingProxy) //media информация
{
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

    std::cout << "-------------------GetVideoSources input-------------------" << endl;

    _trt__GetVideoSources trt__GetVideoSources;
    _trt__GetVideoSourcesResponse trt__GetVideoSourcesResponse;

    if (SOAP_OK == mediaBindingProxy.GetVideoSources(&trt__GetVideoSources, trt__GetVideoSourcesResponse))
    {

        for (int i = 0; i < trt__GetVideoSourcesResponse.VideoSources.size(); i++)
        {
            cout << "\t token: " << trt__GetVideoSourcesResponse.VideoSources[i]->token << endl;
            cout << "\t Framerate: " << trt__GetVideoSourcesResponse.VideoSources[i]->Framerate << endl;
            cout << "\t Resolution Width: " << trt__GetVideoSourcesResponse.VideoSources[i]->Resolution->Width << endl;
            cout << "\t Resolution Height: " << trt__GetVideoSourcesResponse.VideoSources[i]->Resolution->Height << endl;
            if (trt__GetVideoSourcesResponse.VideoSources[i]->Imaging != 0)
            {
                cout << "\t Imaging: " << endl;
                //cout << "\t\t BacklightCompensation Mode: " << trt__GetVideoSourcesResponse.VideoSources[i]->Imaging << endl;
                //cout << "\t\t BacklightCompensation Mode: " << trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->BacklightCompensation << endl;
                //cout << "\t\t BacklightCompensation Mode: " << trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->BacklightCompensation->Mode << endl;
                if (trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->BacklightCompensation)
                {
                    cout << "\t\t BacklightCompensation Mode: " << printMode(trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->BacklightCompensation) << endl;
                    cout << "\t\t BacklightCompensation Level: " << trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->BacklightCompensation->Level << endl;
                }
                cout << "\t\t Brightness: " << *trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->Brightness << endl;
                cout << "\t\t ColorSaturation: " << *trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->ColorSaturation << endl;
                if (trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->Exposure)
                {
                    cout << "\t\t Exposure: " << endl;
                    cout << "\t\t\t Mode: " << printMode(trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->Exposure) << endl;
                    cout << "\t\t\t Priority: " << trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->Exposure->Priority << endl;
                    cout << "\t\t\t Window bottom: " << *trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->Exposure->Window->bottom << endl;
                    cout << "\t\t\t Window left: " << *trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->Exposure->Window->left << endl;
                    cout << "\t\t\t Window right: " << *trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->Exposure->Window->right << endl;
                    cout << "\t\t\t Window top: " << *trt__GetVideoSourcesResponse.VideoSources[i]->Imaging->Exposure->Window->top << endl;
                }
                // дописать вывод у поддерживающих камер
            }
        }
    }
    else
    {
        std::cerr << "Error: GetVideoSourceModes" << endl;
        report_error(mediaBindingProxy.soap);
    }
}

bool GetAudioEncoderConfiguration(MediaBindingProxy mediaBindingProxy, soap *soap) //media информация
{
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);
    std::cout << "-------------------AudioEncoderConfigurations-------------------" << endl;

    _trt__GetAudioEncoderConfigurations *trt__GetAudioEncoderConfigurations = soap_new__trt__GetAudioEncoderConfigurations(soap, -1);
    _trt__GetAudioEncoderConfigurationsResponse *trt__GetAudioEncoderConfigurationsResponse = soap_new__trt__GetAudioEncoderConfigurationsResponse(soap, -1);
    if (SOAP_OK == mediaBindingProxy.GetAudioEncoderConfigurations(trt__GetAudioEncoderConfigurations, *trt__GetAudioEncoderConfigurationsResponse))
    {

        cout << endl;
        for (int i = 0; i < trt__GetAudioEncoderConfigurationsResponse->Configurations.size(); i++)
        {
            Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

            _trt__GetAudioEncoderConfiguration *trt__GetAudioEncoderConfiguration = soap_new__trt__GetAudioEncoderConfiguration(soap, -1);
            _trt__GetAudioEncoderConfigurationResponse *trt__GetAudioEncoderConfigurationResponse = soap_new__trt__GetAudioEncoderConfigurationResponse(soap, -1);

            trt__GetAudioEncoderConfiguration->ConfigurationToken = trt__GetAudioEncoderConfigurationsResponse->Configurations[i]->token;

            if (SOAP_OK == mediaBindingProxy.GetAudioEncoderConfiguration(trt__GetAudioEncoderConfiguration, *trt__GetAudioEncoderConfigurationResponse))
            {
                string AudioEncoding = (trt__GetAudioEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__AudioEncoding__AAC) ? "tt__AudioEncoding__AAC" : (trt__GetAudioEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__AudioEncoding__G711) ? "tt__AudioEncoding__G711"
                                                                                                                                                                    : (trt__GetAudioEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__AudioEncoding__G726)   ? "tt__AudioEncoding__G726"
                                                                                                                                                                                                                                                                             : "Error AudioEncoding";
                cout << "\tEncoding: " << AudioEncoding << endl;
                cout << "\tname: " << trt__GetAudioEncoderConfigurationsResponse->Configurations[i]->Name.c_str() << "  UseCount: " << trt__GetAudioEncoderConfigurationsResponse->Configurations[i]->UseCount << "  token: " << trt__GetAudioEncoderConfigurationsResponse->Configurations[i]->token.c_str() << endl;
            }
            else
            {
                std::cerr << "Error: GetAudioEncoderConfiguration" << endl;
                report_error(mediaBindingProxy.soap);
            }

            cout << endl;
            std::cout << "-------------------AudioEncoderConfigurationOptions-------------------" << endl;
            Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

            _trt__GetAudioEncoderConfigurationOptions trt__GetAudioEncoderConfigurationOptions;
            _trt__GetAudioEncoderConfigurationOptionsResponse trt__GetAudioEncoderConfigurationOptionsResponse;
            string AudioEncoding;

            trt__GetAudioEncoderConfigurationOptions.ConfigurationToken = &trt__GetAudioEncoderConfigurationsResponse->Configurations[i]->token;

            if (SOAP_OK == mediaBindingProxy.GetAudioEncoderConfigurationOptions(&trt__GetAudioEncoderConfigurationOptions, trt__GetAudioEncoderConfigurationOptionsResponse))
            {
                for (int j = 0; j < trt__GetAudioEncoderConfigurationOptionsResponse.Options->Options.size(); j++)
                {
                    for (int k = 0; k < trt__GetAudioEncoderConfigurationOptionsResponse.Options->Options[j]->BitrateList->Items.size(); k++)
                    {
                        cout << "\tBitrateList: " << trt__GetAudioEncoderConfigurationOptionsResponse.Options->Options[j]->BitrateList->Items[k] << endl;
                    }
                    AudioEncoding = (trt__GetAudioEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__AudioEncoding__AAC) ? "tt__AudioEncoding__AAC" : (trt__GetAudioEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__AudioEncoding__G711) ? "tt__AudioEncoding__G711"
                                                                                                                                                                 : (trt__GetAudioEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__AudioEncoding__G726)   ? "tt__AudioEncoding__G726"
                                                                                                                                                                                                                                                                          : "Error AudioEncoding";
                    cout << "\tEncoding: " << AudioEncoding << endl;
                    for (int k = 0; k < trt__GetAudioEncoderConfigurationOptionsResponse.Options->Options[j]->BitrateList->Items.size(); k++)
                    {
                        cout << "\tSampleRateList: " << trt__GetAudioEncoderConfigurationOptionsResponse.Options->Options[j]->SampleRateList->Items[k] << endl;
                    }
                    cout << endl;
                }
            }
            else
            {
                std::cerr << "Error: AudioEncoderConfigurationOptions" << endl;
                report_error(mediaBindingProxy.soap);
            }
        }
    }
    else
    {
        std::cerr << "Error: GetAudioEncoderConfigurations" << endl;
        report_error(mediaBindingProxy.soap);
    }
}

bool GetAudioOutputConfigurations(MediaBindingProxy mediaBindingProxy) //media информация
{
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);
    std::cout << "-------------------AudioOutputConfigurations-------------------" << endl;

    _trt__GetAudioOutputConfigurations trt__GetAudioOutputConfigurations;
    _trt__GetAudioOutputConfigurationsResponse trt__GetAudioOutputConfigurationsResponse;
    if (SOAP_OK == mediaBindingProxy.GetAudioOutputConfigurations(&trt__GetAudioOutputConfigurations, trt__GetAudioOutputConfigurationsResponse))
    {
        if (trt__GetAudioOutputConfigurationsResponse.Configurations.size() == 0)
        {
            cout << "\tAudioOutputConfigurations : " << trt__GetAudioOutputConfigurationsResponse.Configurations.size() << endl;
        }
        for (int i = 0; i < trt__GetAudioOutputConfigurationsResponse.Configurations.size(); i++)
        {
            Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

            _trt__GetAudioOutputConfiguration trt__GetAudioOutputConfiguration;
            _trt__GetAudioOutputConfigurationResponse trt__GetAudioOutputConfigurationResponse;

            trt__GetAudioOutputConfiguration.ConfigurationToken = trt__GetAudioOutputConfigurationsResponse.Configurations[i]->token;

            if (SOAP_OK == mediaBindingProxy.GetAudioOutputConfiguration(&trt__GetAudioOutputConfiguration, trt__GetAudioOutputConfigurationResponse))
            {

                cout << "\tName: " << trt__GetAudioOutputConfigurationResponse.Configuration->Name << endl;
                cout << "\tOutputLevel: " << trt__GetAudioOutputConfigurationResponse.Configuration->OutputLevel << endl;
                cout << "\tOutputToken: " << trt__GetAudioOutputConfigurationResponse.Configuration->OutputToken << endl;
                cout << "\tSendPrimacy: " << trt__GetAudioOutputConfigurationResponse.Configuration->SendPrimacy << endl;
                cout << "\ttoken: " << trt__GetAudioOutputConfigurationResponse.Configuration->token << endl;
                cout << "\tUseCount: " << trt__GetAudioOutputConfigurationResponse.Configuration->UseCount << endl;
            }

            else
            {
                std::cerr << "Error: GetAudioOutputConfiguration" << endl;
                report_error(mediaBindingProxy.soap);
            }
        }
    }
    else
    {
        std::cerr << "Error: GetAudioOutputConfigurations" << endl;
        report_error(mediaBindingProxy.soap);
    }
}

bool GetAudioOutputs(MediaBindingProxy mediaBindingProxy) //media информация
{
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

    std::cout << "-------------------Audio Outputs-------------------" << endl;

    _trt__GetAudioOutputs trt__GetAudioOutputs;
    _trt__GetAudioOutputsResponse trt__GetAudioOutputsResponse;

    if (SOAP_OK == mediaBindingProxy.GetAudioOutputs(&trt__GetAudioOutputs, trt__GetAudioOutputsResponse))
    {

        cout << "\tAudioOutputs : " << trt__GetAudioOutputsResponse.AudioOutputs.size() << endl;
        for (int i = 0; i < trt__GetAudioOutputsResponse.AudioOutputs.size(); i++)
        {
            cout << "\tAudioOutputs token: " << trt__GetAudioOutputsResponse.AudioOutputs[i]->token << endl;

            ;
        }
    }

    else
    {
        std::cerr << "Error: GetAudioOutputs" << endl;
        report_error(mediaBindingProxy.soap);
    }
}

bool GetAudioSources(MediaBindingProxy mediaBindingProxy) //media информация
{
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

    std::cout << "-------------------Audio Inputs-------------------" << endl;
    _trt__GetAudioSources trt__GetAudioSources;
    _trt__GetAudioSourcesResponse trt__GetAudioSourcesResponse;
    if (SOAP_OK == mediaBindingProxy.GetAudioSources(&trt__GetAudioSources, trt__GetAudioSourcesResponse))
    {

        //cout << "\tAudioInputs : " << trt__GetAudioSourcesResponse.AudioSources.size() << endl;

        for (int i = 0; i < trt__GetAudioSourcesResponse.AudioSources.size(); i++)
        {
            string AudioChannels = (trt__GetAudioSourcesResponse.AudioSources[i]->Channels == 1) ? "Mono" : (trt__GetAudioSourcesResponse.AudioSources[i]->Channels == 2) ? "Stereo"
                                                                                                                                                                          : "No audio Inputs available";
            cout << "\t" << AudioChannels << endl;
            if (trt__GetAudioSourcesResponse.AudioSources[i]->Channels != 0)
                cout << "\taudio Inputs token: " << trt__GetAudioSourcesResponse.AudioSources[i]->token << endl;
        }
        cout << endl;
    }
    else
    {
        std::cerr << "Error: GetAudioSources" << endl;
        report_error(mediaBindingProxy.soap);
    }
}

bool Media(const std::string &profileToken) //получение всей информации о видео и аудио с камеры
{
    std::cout << "====================== MediaBinding Profiles ======================" << std::endl;

    MediaBindingProxy mediaBindingProxy;

    mediaBindingProxy.soap_endpoint = "http://10.24.72.22:8080/onvif/media";
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

    struct soap *soap = soap_new();

    //_trt__GetProfiles *GetProfiles = soap_new__trt__GetProfiles(soap);
    //_trt__GetProfilesResponse *GetProfilesResponse = soap_new__trt__GetProfilesResponse(soap);
    _trt__GetProfiles GetProfiles;
    _trt__GetProfilesResponse GetProfilesResponse;

    cout << "Profile name"
         << "         "
         << "Profile token" << endl;
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

    if (SOAP_OK == mediaBindingProxy.GetProfiles(&GetProfiles, GetProfilesResponse))
    {
        for (auto &Profile : GetProfilesResponse.Profiles)
            cout << Profile->Name << "         " << Profile->token << endl;
    }
    else
    {
        std::cerr << "Error: getProfiles" << endl;
        report_error(mediaBindingProxy.soap);
    }
    std::cout << "====================== MediaBinding GetSnapshotUri ======================" << std::endl;
    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

    _trt__GetSnapshotUri *GetSnapshotUri = soap_new__trt__GetSnapshotUri(soap);
    GetSnapshotUri->ProfileToken = profileToken;
    _trt__GetSnapshotUriResponse *GetSnapshotUriResponse = soap_new__trt__GetSnapshotUriResponse(soap);

    if (SOAP_OK == mediaBindingProxy.GetSnapshotUri(GetSnapshotUri, *GetSnapshotUriResponse))
    {
        cout << "SnapshotUri: " << GetSnapshotUriResponse->MediaUri->Uri << endl;
    }
    else
    {
        std::cerr << "Error: getSnapshotUri" << endl;
        report_error(mediaBindingProxy.soap);
    }

    std::cout << "====================== MediaBinding GetStreamUri ======================" << std::endl;

    Soap_wsse_add_UsernameTokenDigest(mediaBindingProxy.soap);

    _trt__GetStreamUri *GetStreamUri = soap_new__trt__GetStreamUri(soap);
    GetStreamUri->ProfileToken = profileToken;
    GetStreamUri->StreamSetup = soap_new_tt__StreamSetup(soap, -1);
    GetStreamUri->StreamSetup->Stream = tt__StreamType__RTP_Unicast;
    GetStreamUri->StreamSetup->Stream = tt__StreamType__RTP_Unicast;
    GetStreamUri->StreamSetup->Transport = soap_new_tt__Transport(soap, -1);
    GetStreamUri->StreamSetup->Transport->Protocol = tt__TransportProtocol__RTSP;
    GetStreamUri->ProfileToken = profileToken;

    _trt__GetStreamUriResponse *GetStreamUriResponse = soap_new__trt__GetStreamUriResponse(soap);
    if (SOAP_OK == mediaBindingProxy.GetStreamUri(GetStreamUri, *GetStreamUriResponse))
    {
        cout << "StreamUri: " << GetStreamUriResponse->MediaUri->Uri << endl;
    }
    else
    {
        std::cerr << "Error: getStreamUri" << endl;
        report_error(mediaBindingProxy.soap);
    }

    //std::cout << "-------------------Video-------------------" << endl;
    GetVideoEncoderConfigurations(mediaBindingProxy, soap);
    //GetVideoAnalyticsConfigurations(mediaBindingProxy);
    GetVideoSourceConfiguration(mediaBindingProxy);
    GetVideoSources(mediaBindingProxy);
    std::cout << "-------------------Audio-------------------" << endl
              << endl;
    GetAudioEncoderConfiguration(mediaBindingProxy, soap); // ДОПИСАТЬ ВСЕ ВЫВОДЫ ФУНКЦИИ
    cout << endl;
    GetAudioOutputConfigurations(mediaBindingProxy);
    cout << endl;
    GetAudioOutputs(mediaBindingProxy);
    cout << endl;
    GetAudioSources(mediaBindingProxy);

    CLEANUP_SOAP(soap);
}

bool getEvents()
{
    PullPointSubscriptionBindingProxy pullPointSubscriptionBindingProxy;

    //pullPointSubscriptionBindingProxy.soap_endpoint = "http://10.24.72.22:8080/onvif/events";
    pullPointSubscriptionBindingProxy.soap_endpoint = "http://10.24.72.33:8899/onvif/Events";

    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(pullPointSubscriptionBindingProxy.soap, nullptr, username.c_str(), password.c_str()))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << std::endl;
        report_error(pullPointSubscriptionBindingProxy.soap);
        return false;
    }

    _tev__GetEventProperties tev__GetEventProperties;
    _tev__GetEventPropertiesResponse tev__GetEventPropertiesResponse;

    if (SOAP_OK == pullPointSubscriptionBindingProxy.GetEventProperties(&tev__GetEventProperties, tev__GetEventPropertiesResponse))
    {
        std::cout << "tev__GetEventPropertiesResponse.MessageContentFilterDialect.size(): " << tev__GetEventPropertiesResponse.MessageContentFilterDialect.size() << std::endl;
        std::cout << "tev__GetEventPropertiesResponse.MessageContentFilterDialect[0](): " << tev__GetEventPropertiesResponse.MessageContentFilterDialect[0] << std::endl;
        std::cout << "tev__GetEventPropertiesResponse.MessageContentSchemaLocation.size(): " << tev__GetEventPropertiesResponse.MessageContentSchemaLocation.size() << std::endl;
        std::cout << "tev__GetEventPropertiesResponse.MessageContentSchemaLocation[0]: " << tev__GetEventPropertiesResponse.MessageContentSchemaLocation[0] << std::endl;
        std::cout << "tev__GetEventPropertiesResponse.ProducerPropertiesFilterDialect.size: " << tev__GetEventPropertiesResponse.ProducerPropertiesFilterDialect.size() << std::endl;
        //std::cout << "tev__GetEventPropertiesResponse.ProducerPropertiesFilterDialect[0]: " << tev__GetEventPropertiesResponse.ProducerPropertiesFilterDialect[0]<< std::endl;
        std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation: " << tev__GetEventPropertiesResponse.TopicNamespaceLocation.size() << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation[0].size(): " << tev__GetEventPropertiesResponse.TopicNamespaceLocation[0].size() << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation[0]: " << tev__GetEventPropertiesResponse.TopicNamespaceLocation[0] << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation[0][0]: " << tev__GetEventPropertiesResponse.TopicNamespaceLocation[0][0] << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.wsnt__FixedTopicSet: " << tev__GetEventPropertiesResponse.wsnt__FixedTopicSet << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.wsnt__TopicExpressionDialect.size(): " << tev__GetEventPropertiesResponse.wsnt__TopicExpressionDialect.size() << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.wsnt__TopicExpressionDialect[0].size(): " << tev__GetEventPropertiesResponse.wsnt__TopicExpressionDialect[0].size() << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation[0][0]: " << tev__GetEventPropertiesResponse.wsnt__TopicExpressionDialect[0] << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.wstop__TopicSet->documentation: " << tev__GetEventPropertiesResponse.wstop__TopicSet->documentation << std::endl;
    }
    else
    {
        std::cerr << "Error: GetEventProperties" << endl;
        report_error(pullPointSubscriptionBindingProxy.soap);
    }

    _tev__GetServiceCapabilities tev__GetServiceCapabilities;
    _tev__GetServiceCapabilitiesResponse tev__GetServiceCapabilitiesResponse;

    if (SOAP_OK == pullPointSubscriptionBindingProxy.GetServiceCapabilities(&tev__GetServiceCapabilities, tev__GetServiceCapabilitiesResponse))
    {
        //std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation.size(): " << tev__GetServiceCapabilitiesResponse.Capabilities->EventBrokerProtocols->c_str() << std::endl;
        //std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation.size(): " << tev__GetServiceCapabilitiesResponse.Capabilities->MaxEventBrokers << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation.size(): " << tev__GetServiceCapabilitiesResponse.Capabilities->MaxNotificationProducers << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation.size(): " << tev__GetServiceCapabilitiesResponse.Capabilities->MaxPullPoints << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation.size(): " << tev__GetServiceCapabilitiesResponse.Capabilities->PersistentNotificationStorage << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation.size(): " << tev__GetServiceCapabilitiesResponse.Capabilities->WSPausableSubscriptionManagerInterfaceSupport << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation.size(): " << tev__GetServiceCapabilitiesResponse.Capabilities->WSPullPointSupport << std::endl;
        std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation.size(): " << tev__GetServiceCapabilitiesResponse.Capabilities->WSSubscriptionPolicySupport << std::endl;
    }
    else
    {
        std::cerr << "Error: GetEventProperties" << endl;
        report_error(pullPointSubscriptionBindingProxy.soap);
    }

    /*_tev__GetEventBrokers tev__GetEventBrokers;
    _tev__GetEventBrokersResponse tev__GetEventBrokersResponse;
   // tev__GetEventBrokers.Address
    if (SOAP_OK == pullPointSubscriptionBindingProxy.GetEventBrokers(&tev__GetEventBrokers, tev__GetEventBrokersResponse))
    {
        std::cout << "ev__GetEventPropertiesResponse.TopicNamespaceLocation.size(): " << tev__GetEventBrokersResponse.EventBroker.size() << std::endl;
    }
    else
    {
        std::cerr << "Error: GetEventProperties" << endl;
        report_error(pullPointSubscriptionBindingProxy.soap);
    }*/
}

bool AddSecurity(soap *pProxy, const bool &bAddWsse)
{
    if (bAddWsse)
    {
        soap_wsse_add_Security(pProxy);
    }

    if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(pProxy, nullptr, username.c_str(), password.c_str()))
    {
        std::cerr << "Error: soap_wsse_add_UsernameTokenDigest" << endl;
        report_error(pProxy);
        return false;
    }

    if (SOAP_OK != soap_wsse_add_Timestamp(pProxy, "Time", 10))
    {
        std::cerr << "Error adding timestamp" << endl;
        report_error(pProxy);
        return false;
    }
}

void PrintOnvifParameter(string str1, string str2)
{
    cout << str1 << str2 << endl;
}

void PrintOnvifParameter(string str1)
{
    cout << str1 << endl;
}

void ReceivingEvents() //получение событий с камеры
{
    DeviceBindingProxy DeviceProxy;
    AddSecurity(DeviceProxy.soap, true);

    DeviceProxy.soap_endpoint = "http://10.24.72.22:8080/onvif/devices";

    _tds__GetCapabilities deviceCapabilities;
    deviceCapabilities.Category.push_back(tt__CapabilityCategory__All);
    _tds__GetCapabilitiesResponse deviceCapabilitiesResponse;

    if (SOAP_OK == DeviceProxy.GetCapabilities(&deviceCapabilities, deviceCapabilitiesResponse))
    {
        if (deviceCapabilitiesResponse.Capabilities->Events != nullptr)
        {
            cout << "Events address: " + deviceCapabilitiesResponse.Capabilities->Events->XAddr << endl;

            PullPointSubscriptionBindingProxy EventsProxy;
            EventsProxy.soap_endpoint = deviceCapabilitiesResponse.Capabilities->Events->XAddr.c_str();
            AddSecurity(EventsProxy.soap, true);

            _tev__GetEventProperties gep;
            _tev__GetEventPropertiesResponse gepr;

            if (EventsProxy.GetEventProperties(&gep, gepr) == SOAP_OK)
            {
                PrintOnvifParameter("Event: TopicNamespaceLocation");
                for (size_t i = 0; i < gepr.TopicNamespaceLocation.size(); i++)
                {
                    PrintOnvifParameter("", gepr.TopicNamespaceLocation.at(i).c_str());
                }
                //Верно, если набор тем фиксирован на все времена
                PrintOnvifParameter("Event: fixedTopicSet:", gepr.wsnt__FixedTopicSet ? "Yes" : "No");
                PrintOnvifParameter("Event: Topics:");
                const char *tag;
                const char *type;
                //gepr.soap_put(DeviceProxy.soap, tag, type);
                if (gepr.wstop__TopicSet != NULL && gepr.wstop__TopicSet->documentation != NULL && gepr.wstop__TopicSet->documentation->__mixed != NULL)
                {
                    PrintOnvifParameter("", gepr.wstop__TopicSet->documentation->__mixed);
                }
                else
                {
                    PrintOnvifParameter("", "None");
                }
                //Должны быть возвращены, если устройство поддерживает фильтрацию содержимого сообщения.
                //Устройство, которое не поддерживает какой-либо MessageContentFilterDialect, возвращает единственный пустой URL-адрес.
                //Определяет набор функций XPath, поддерживаемый для фильтрации содержимого сообщений.
                PrintOnvifParameter("Event: MessageContentFilterDialect");
                for (size_t i = 0; i < gepr.MessageContentFilterDialect.size(); i++)
                {
                    PrintOnvifParameter("", gepr.MessageContentFilterDialect.at(i).c_str());
                }
                //Язык описания содержимого сообщений позволяет ссылаться на типы, зависящие от поставщика
                PrintOnvifParameter("Event: MessageContentSchemaLocation");
                for (size_t i = 0; i < gepr.MessageContentSchemaLocation.size(); i++)
                {
                    PrintOnvifParameter("", gepr.MessageContentSchemaLocation.at(i).c_str());
                }
                //Расширенный фильтрации
                PrintOnvifParameter("Event: ProducerPropertiesFilterDialect");
                for (size_t i = 0; i < gepr.ProducerPropertiesFilterDialect.size(); i++)
                {
                    PrintOnvifParameter("", gepr.ProducerPropertiesFilterDialect.at(i).c_str());
                }
                //Определяет синтаксис выражения XPath, поддерживаемый для сопоставления тематических выражений.
                PrintOnvifParameter("Event: wsnt__TopicExpressionDialect");
                for (size_t i = 0; i < gepr.wsnt__TopicExpressionDialect.size(); i++)
                {
                    PrintOnvifParameter("", gepr.wsnt__TopicExpressionDialect.at(i).c_str());
                }
            }
            else
            {
                EventsProxy.soap_stream_fault(std::cerr);
                PrintOnvifParameter("Error getting event profiles");
            }

            PrintOnvifParameter("Event: Creating pull point");
            AddSecurity(EventsProxy.soap, true);

            _tev__CreatePullPointSubscription *cpps = soap_new__tev__CreatePullPointSubscription(DeviceProxy.soap, -1);
            _tev__CreatePullPointSubscriptionResponse *cppr = soap_new__tev__CreatePullPointSubscriptionResponse(DeviceProxy.soap, -1);
            std::string termtime = "PT60S";
            cpps->InitialTerminationTime = &termtime;

            if (EventsProxy.CreatePullPointSubscription(cpps, *cppr) == SOAP_OK)
            {
                PrintOnvifParameter("Event: pull point subscription created:");
                //Дата и время, когда PullPoint будет отключен без дальнейших запросов на вытягивание
                PrintOnvifParameter("termination time: ", to_string(cppr->wsnt__TerminationTime.tv_sec));
                //SubscriptionReference - cсылка на конечную точку подписки, которая будет использоваться для получения сообщений
                PrintOnvifParameter("SubscriptionReference.Address: ", cppr->SubscriptionReference.Address);

                if (cppr->SubscriptionReference.__anyAttribute != nullptr)
                {
                    PrintOnvifParameter("__anyAttribute: ", cppr->SubscriptionReference.__anyAttribute);
                }
                PrintOnvifParameter("SubscriptionReference.__size", std::to_string(cppr->SubscriptionReference.__size));
                for (int i = 0; i < cppr->SubscriptionReference.__size; i++)
                {
                    PrintOnvifParameter("__any", cppr->SubscriptionReference.__any[i]);
                }

                if (cppr->SubscriptionReference.Metadata != nullptr)
                {
                    if (cppr->SubscriptionReference.Metadata->__anyAttribute != nullptr)
                    {
                        PrintOnvifParameter("Metadata->__anyAttribute",
                                            cppr->SubscriptionReference.ReferenceParameters->__anyAttribute);
                    }
                    for (int i = 0;

                         i < cppr->SubscriptionReference.Metadata->__size; i++)
                    {
                        PrintOnvifParameter("MetaData__any",
                                            cppr->SubscriptionReference.Metadata->__any[i]);
                    }
                }
                if

                    (cppr->SubscriptionReference.ReferenceParameters != nullptr)
                {
                    if (cppr->SubscriptionReference.ReferenceParameters->__anyAttribute != nullptr)
                    {
                        PrintOnvifParameter("ReferenceParameter->__anyAttribute",
                                            cppr->SubscriptionReference.ReferenceParameters->__anyAttribute);
                    }
                    PrintOnvifParameter("SubscriptionReference.ReferenceParameters->__size",
                                        std::to_string(cppr->SubscriptionReference.ReferenceParameters->__size));
                    for (int i = 0; i < cppr->SubscriptionReference.ReferenceParameters->__size; i++)
                    {
                        PrintOnvifParameter("ReferenceParameter",
                                            cppr->SubscriptionReference.ReferenceParameters->__any[i]);
                    }
                }

                // Настройте прокси для чтения из справочника подписки
                std::string sSubscriptionAddress = cppr->SubscriptionReference.Address;
                PullPointSubscriptionBindingProxy PullPointProxy(sSubscriptionAddress.c_str());
                AddSecurity(PullPointProxy.soap, true);
                struct soap *soap_wsa = soap_new();
                //soap_register_plugin_arg(PullPointProxy.soap, soap_wsa);

                while (true)
                {
                    //usleep(5000000);
                    AddSecurity(PullPointProxy.soap, false);
                    if (SOAP_OK != soap_wsa_request(PullPointProxy.soap,
                                                    nullptr,
                                                    sSubscriptionAddress.c_str(),
                                                    "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesRequest") ||
                        (SOAP_OK != soap_wsa_add_ReplyTo(PullPointProxy.soap,
                                                         "http://www.w3.org/2005/08/addressing/anonymous")))
                    {
                        PullPointProxy.soap_stream_fault(std::cerr);
                        PrintOnvifParameter("soap_wsa_request error");
                    }

                    _tev__PullMessages PullMessages;
                    //мс
                    PullMessages.Timeout = 10000;
                    PullMessages.MessageLimit = 1024;
                    _tev__PullMessagesResponse PullMessagesResponse;

                    int nRetCode = SOAP_OK;
                    //if ((nRetCode = PullPointProxy.PullMessages("http://10.25.200.40/onvif/devices", "tns1:VideoAnalytics//.", &PullMessages, PullMessagesResponse)) == SOAP_OK)
                    if ((nRetCode = PullPointProxy.PullMessages(&PullMessages, PullMessagesResponse)) == SOAP_OK)
                    {
                        for (auto msg : PullMessagesResponse.wsnt__NotificationMessage)
                        {
                            cout << "!!!" << endl;
                            const char *type = new char[100];
                            cout << "Dialect: " << msg->Topic->Dialect << endl;
                            cout << "__mixed: " << msg->Topic->__mixed << endl;
                        }
                    }
                    else
                    {
                        PullPointProxy.soap_stream_fault(std::cerr);
                        PrintOnvifParameter("PullPointProxy.PullMessages() error");
                    }
                }
            }
        }
        else
        {
            std::cerr << "No device events capabilities" << endl;
        }
    }
    else
    {
        std::cerr << "Error getting device capabilities" << endl;
    }
}

void DeviceInformation()
{
    cout << "-------------------DeviceInformation-------------------" << endl;

    DeviceBindingProxy proxyDevice;
    //proxyDevice.soap_endpoint = "http://10.24.72.31/onvif/Device";
    proxyDevice.soap_endpoint = "http://10.22.200.240/onvif/device_service";

    Soap_wsse_add_UsernameTokenDigest(proxyDevice.soap);
    _tds__GetDeviceInformation tds__GetDeviceInformation;
    _tds__GetDeviceInformationResponse tds__GetDeviceInformationResponse;

    if (SOAP_OK == proxyDevice.GetDeviceInformation(&tds__GetDeviceInformation, tds__GetDeviceInformationResponse))
    {
        cout << "\t  Manufacturer: " << tds__GetDeviceInformationResponse.Manufacturer << endl;
        cout << "\t  Model: " << tds__GetDeviceInformationResponse.Model << endl;
        cout << "\t  FirmwareVersion: " << tds__GetDeviceInformationResponse.FirmwareVersion << endl;
        cout << "\t  HardwareId: " << tds__GetDeviceInformationResponse.HardwareId << endl;
    }
    else
    {
        std::cerr << "Error: GetDeviceInformation" << endl;
        report_error(proxyDevice.soap);
    }
}

void GetSystemDateAndTime(const string host) //получить время устройства
{
    cout << "-------------------GetSystemDateAndTime-------------------" << endl;

    DeviceBindingProxy deviceBindingProxy;
    deviceBindingProxy.soap_endpoint = host.c_str();

    Soap_wsse_add_UsernameTokenDigest(deviceBindingProxy.soap);

    struct soap *soap = soap_new();
    _tds__GetSystemDateAndTime *tds__GetSystemDateAndTime = soap_new__tds__GetSystemDateAndTime(soap);
    _tds__GetSystemDateAndTimeResponse *tds__GetSystemDateAndTimeResponse = soap_new__tds__GetSystemDateAndTimeResponse(soap);

    if (SOAP_OK == deviceBindingProxy.GetSystemDateAndTime(tds__GetSystemDateAndTime, *tds__GetSystemDateAndTimeResponse))
    {
        //Указывает, установлено ли время вручную или через NTP
        //enum { 'Manual', 'NTP' }
        cout << "DateTimeType:    " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DateTimeType << endl;
        //Информативный индикатор включения / выключения летнего времени
        cout << "DaylightSavings:    " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DaylightSavings << endl;
        //Информация о часовом поясе в формате Posix
        cout << "TimeZone:    " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ << endl;
        //Текущая системная дата и время в формате UTC
        cout << "UTCDateTime:    " << endl;
        cout << "\tTime: " << endl;
        cout << "\t\tHour: " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Hour << endl;
        cout << "\t\tMinute: " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Minute << endl;
        cout << "\t\tSecond: " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Second << endl;
        cout << "\tDate: " << endl;
        cout << "\t\tYear: " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Year << endl;
        cout << "\t\tMonth: " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Month << endl;
        cout << "\t\tDay: " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Day << endl;
        //Дата и время в местном формате
        if (tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime)
        {
            cout << "LocalDateTime: " << endl;
            cout << "\tTime: " << endl;
            cout << "\t\tHour: " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Hour << endl;
            cout << "\t\tMinute: " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Minute << endl;
            cout << "\t\tSecond: " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Second << endl;
            cout << "\tDate: " << endl;
            cout << "\t\tYear: " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Year << endl;
            cout << "\t\tMonth: " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Month << endl;
            cout << "\t\tDay: " << tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Day << endl;
        }
    }
    else
    {
        std::cerr << "Error: GetSystemDateAndTime" << endl;
        report_error(deviceBindingProxy.soap);
    }

    CLEANUP_SOAP(soap);
}

void SetSystemDateAndTime(const string host) //задать время устройства
{
    cout << "-------------------SetSystemDateAndTime-------------------" << endl;

    DeviceBindingProxy deviceBindingProxy;
    deviceBindingProxy.soap_endpoint = host.c_str();
    Soap_wsse_add_UsernameTokenDigest(deviceBindingProxy.soap);

    struct soap *soap = soap_new();
    _tds__SetSystemDateAndTime *tds__SetSystemDateAndTime = soap_new__tds__SetSystemDateAndTime(soap);
    _tds__SetSystemDateAndTimeResponse *tds__SetSystemDateAndTimeResponse = soap_new__tds__SetSystemDateAndTimeResponse(soap);

    //Указывает, установлено ли время вручную или через NTP
    //enum { 'Manual', 'NTP' }
    tds__SetSystemDateAndTime->DateTimeType = tt__SetDateTimeType__Manual;
    //Информативный индикатор включения / выключения летнего времени
    tds__SetSystemDateAndTime->DaylightSavings = 0;
    //Информация о часовом поясе в формате Posix
    tds__SetSystemDateAndTime->TimeZone = soap_new_tt__TimeZone(soap, -1);
    tds__SetSystemDateAndTime->TimeZone->TZ = "GMT+07:00";
    //Текущая системная дата и время в формате UTC
    tds__SetSystemDateAndTime->UTCDateTime = soap_new_tt__DateTime(soap);
    tds__SetSystemDateAndTime->UTCDateTime->Time = soap_new_tt__Time(soap);
    tds__SetSystemDateAndTime->UTCDateTime->Time->Hour = 10;
    tds__SetSystemDateAndTime->UTCDateTime->Time->Minute = 20;
    tds__SetSystemDateAndTime->UTCDateTime->Time->Second = 22;
    tds__SetSystemDateAndTime->UTCDateTime->Date = soap_new_tt__Date(soap);
    tds__SetSystemDateAndTime->UTCDateTime->Date->Year = 2021;
    tds__SetSystemDateAndTime->UTCDateTime->Date->Month = 6;
    tds__SetSystemDateAndTime->UTCDateTime->Date->Day = 18;

    cout << "DateTimeType:    " << tds__SetSystemDateAndTime->DateTimeType << endl;
    //Информативный индикатор включения / выключения летнего времени
    cout << "DaylightSavings:    " << tds__SetSystemDateAndTime->DaylightSavings << endl;
    //Информация о часовом поясе в формате Posix
    cout << "TimeZone:    " << tds__SetSystemDateAndTime->TimeZone->TZ << endl;
    //Текущая системная дата и время в формате UTC
    cout << "UTCDateTime:    " << endl;
    cout << "\tTime: " << endl;
    cout << "\t\tHour: " << tds__SetSystemDateAndTime->UTCDateTime->Time->Hour << endl;
    cout << "\t\tMinute: " << tds__SetSystemDateAndTime->UTCDateTime->Time->Minute << endl;
    cout << "\t\tSecond: " << tds__SetSystemDateAndTime->UTCDateTime->Time->Second << endl;
    cout << "\tDate: " << endl;
    cout << "\t\tYear: " << tds__SetSystemDateAndTime->UTCDateTime->Date->Year << endl;
    cout << "\t\tMonth: " << tds__SetSystemDateAndTime->UTCDateTime->Date->Month << endl;
    cout << "\t\tDay: " << tds__SetSystemDateAndTime->UTCDateTime->Date->Day << endl;
    Soap_wsse_add_UsernameTokenDigest(deviceBindingProxy.soap);

    if (SOAP_OK == deviceBindingProxy.SetSystemDateAndTime(tds__SetSystemDateAndTime, *tds__SetSystemDateAndTimeResponse))
    {
        cout << "!!!" << endl;
    }
    else
    {
        std::cerr << "Error: SetSystemDateAndTime" << endl;
        report_error(deviceBindingProxy.soap);
    }

    CLEANUP_SOAP(soap);
}

void SystemDateAndTime()
{
    const string host = "http://10.24.72.22:8080/onvif/devices";
    //const string host = "http://10.21.200.137/onvif/device_service";
    //const string host = "http://10.25.200.40/onvif/device_service";
    //const string host = "http://10.24.72.33:8899/onvif/device_service";
    //const string host = "http://10.24.72.31/onvif/Device";

    GetSystemDateAndTime(host);

    SetSystemDateAndTime(host);

    GetSystemDateAndTime(host);
}

void PTZ()
{
    //изменение параметров камеры
    //test("000");

    _ocp_PTZStatus status;
    //if (testPtz("MainProfileToken",status))
    //testPtz("MainProfileToken", status, -0.1, 0, 0, 0, 0, 0);
    //usleep(2000000);
    // testPtz("MainProfileToken",status,0,-1,0,0,1,0);
    // usleep(2000000);

    //media("MainProfileToken");

    //std::cout << "====================== PTZBinding Configurations ======================" << std::endl;
    //getConfiguraions();
    //ptz("MainProfileToken");
    // absoluteMove(PROFILETOKEN, -0.01, 0, 0, 0, 0, 0);
    //  usleep(7000000);
    //  stop(PROFILETOKEN, true, true);

    // ptz("SubProfileToken");

    //absoluteMove1();
    //getCapabilites();

    string profileToken = "MainProfileToken";
    //_ocp_PTZStatus status;
    bool getStatusResult = getStatus(profileToken, status);
    if (getStatusResult)
    {
        std::cout << "====================== PTZBinding Status ======================" << std::endl;
        cout << "Pan:  " << status.pan << endl;
        cout << "Tilt: " << status.tilt << endl;
        cout << "Zoom: " << status.zoom << endl;
        cout << "MovingStatusPanTilt: " << status.move_status_pan_tilt << endl;
        cout << "MovingStatusZoom: " << status.move_status_zoom << endl;
    }

    std::cout << "====================== PTZBinding AbsoluteMove ======================" << std::endl;
   /* absoluteMove(profileToken, 0, -0.1, 0, 0, -1, 0);
    usleep(2000000);
    getc(stdin);
    absoluteMove(profileToken, 0, 0.1, 0, 0, 0, 0);
    usleep(2000000);
    getc(stdin);

    absoluteMove(profileToken, 1, 0, 0, 1, 0, 0);
    usleep(2000000);
    getc(stdin);

    absoluteMove(profileToken, 0.9, 0.9, 0, 0.5, 0.5, 0);
    usleep(2000000);
    getc(stdin);*/

    absoluteMove(profileToken, 1, 0, 0, 1, 0, 0);

    cout << "Moving" << endl;
    do
    {
        usleep(5000000);
        getStatusResult = getStatus(profileToken, status);
    } while (getStatusResult && (status.move_status_pan_tilt || status.move_status_zoom));
    cout << "stop" << endl;
    // stop(profileToken, true, true);
    if (getStatusResult)
    {
        std::cout << "====================== PTZBinding Status ======================" << std::endl;
        cout << "Pan:  " << status.pan << endl;
        cout << "Tilt: " << status.tilt << endl;
        cout << "Zoom: " << status.zoom << endl;
        cout << "MovingStatusPanTilt: " << status.move_status_pan_tilt << endl;
        cout << "MovingStatusZoom: " << status.move_status_zoom << endl;
    }

    std::cout << "====================== PTZBinding ContinuousMove ======================" << std::endl;
    // It will stop after 5000ms
    /*continuousMove(profileToken, -1, 0, 0, 5000);
    usleep(5000000);
    continuousMove(profileToken, 1, 1, 0, 5);
    usleep(5000000);
    continuousMove(profileToken, -1, -1, 0, 0);
    usleep(5000000);*/
    continuousMove(profileToken, -1, 0, 0, 0);

    cout << "Moving" << endl;
    do
    {
        usleep(5000000);
        getStatusResult = getStatus(profileToken, status);
    } while (getStatusResult && (status.move_status_pan_tilt || status.move_status_zoom));
    cout << "stop" << endl;
    stop(profileToken, true, true);
    if (getStatusResult)
    {
        std::cout << "====================== PTZBinding Status ======================" << std::endl;
        cout << "Pan:  " << status.pan << endl;
        cout << "Tilt: " << status.tilt << endl;
        cout << "Zoom: " << status.zoom << endl;
        cout << "MovingStatusPanTilt: " << status.move_status_pan_tilt << endl;
        cout << "MovingStatusZoom: " << status.move_status_zoom << endl;
    }
}

int main()
{
    //PTZ();
    //SystemDateAndTime();
    //getEvents();
    //checkImaging();
    changesettingsImages();

    //Media("MainProfileToken");
    //DeviceInformation();
    //cout<<"========================="<<endl;
    //ReceivingEvents();
    return 0;
}
