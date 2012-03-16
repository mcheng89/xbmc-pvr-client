/*
 *      Copyright (C) 2011 Pulse-Eight
 *      http://www.pulse-eight.com/
 *
 *      3/15/12 - Modified by Michael Cheng for CetonTV
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "client.h"
#include "xbmc/xbmc_pvr_dll.h"
#include "tvserver.h"
#include <time.h>

using namespace std;
using namespace ADDON;

bool           m_bCreated       = false;
ADDON_STATUS   m_CurStatus      = ADDON_STATUS_UNKNOWN;
int            g_iClientId      = -1;
bool           m_bIsPlaying     = false;

/* User adjustable settings are saved here.
 * Default values are defined inside client.h
 * and exported to the other source files.
 */
std::string g_strUserPath             = "";
std::string g_strClientPath           = "";

CHelper_libXBMC_addon *XBMC           = NULL;
CHelper_libXBMC_pvr   *PVR            = NULL;

std::vector<struct ChannelMap> channels;

std::string stream_ip;
int         stream_port;
bool        stream_secured;
std::string stream_username;
std::string stream_password;
int         stream_resolution;
int         stream_quality;

int         stream_resolution_values[7] = {0,240,360,480,576,720,1080};
int         stream_quality_values_240[3] = {100,200,300};
int         stream_quality_values_360[3] = {300,500,700};
int         stream_quality_values_480[3] = {500,1000,1500};
int         stream_quality_values_576[3] = {1250,1750,2250};
int         stream_quality_values_720[3] = {2000,3000,4000};
int         stream_quality_values_1080[3] = {3000,4000,5000};

int         current_channel;

extern "C" {

ADDON_STATUS ADDON_Create(void* hdl, void* props)
{
    if (!hdl || !props)
        return ADDON_STATUS_UNKNOWN;

    PVR_PROPERTIES* pvrprops = (PVR_PROPERTIES*)props;

    XBMC = new CHelper_libXBMC_addon;
    if (!XBMC->RegisterMe(hdl)) {
        delete XBMC;
        return ADDON_STATUS_UNKNOWN;
    }
    PVR = new CHelper_libXBMC_pvr;
    if (!PVR->RegisterMe(hdl))
        goto ADDON_ERROR;

    XBMC->Log(LOG_NOTICE, "PVRCLIENT: %s - Creating the PVR demo add-on", __FUNCTION__);

    if (ADDON_SetSetting("ALL SETTINGS","")!=ADDON_STATUS_OK)
        goto ADDON_ERROR;

    srand(time(0));
    if (LoadChannelMap(channels) != 0)
        goto ADDON_ERROR;

    m_CurStatus     = ADDON_STATUS_UNKNOWN;
    g_strUserPath   = pvrprops->strUserPath;
    g_strClientPath = pvrprops->strClientPath;

    m_CurStatus = ADDON_STATUS_OK;
    m_bCreated = true;
    return m_CurStatus;

    ADDON_ERROR:
    delete XBMC;
    delete PVR;
    return ADDON_STATUS_UNKNOWN;
}

ADDON_STATUS ADDON_GetStatus() { return m_CurStatus; }

void ADDON_Destroy()
{
    m_bCreated = false;
    m_CurStatus = ADDON_STATUS_UNKNOWN;
}

bool ADDON_HasSettings() { return true; }

unsigned int ADDON_GetSettings(ADDON_StructSetting ***sSet)
{
    return 0;
}

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
    XBMC->Log(LOG_NOTICE, "PVRCLIENT: ADDON_SetSetting()");

    char buffer[1024];
    if (!XBMC->GetSetting("tvserver_ip", buffer))
        return ADDON_STATUS_UNKNOWN;
    stream_ip = buffer;
    if (!XBMC->GetSetting("tvserver_port", &stream_port))
        return ADDON_STATUS_UNKNOWN;
    if (!XBMC->GetSetting("tvserver_secured", &stream_secured))
        return ADDON_STATUS_UNKNOWN;
    if (!XBMC->GetSetting("tvserver_username", buffer))
        return ADDON_STATUS_UNKNOWN;
    stream_username = buffer;
    if (!XBMC->GetSetting("tvserver_password", buffer))
        return ADDON_STATUS_UNKNOWN;
    stream_password = buffer;
    if (!XBMC->GetSetting("tvserver_resolution", &stream_resolution))
        return ADDON_STATUS_UNKNOWN;
    stream_resolution = stream_resolution_values[stream_resolution];

    if (stream_resolution == 240) {
        if (!XBMC->GetSetting("tvserver_quality240", &stream_quality))
            return ADDON_STATUS_UNKNOWN;
        stream_quality = stream_quality_values_240[stream_quality];
    } else if (stream_resolution == 360) {
        if (!XBMC->GetSetting("tvserver_quality360", &stream_quality))
            return ADDON_STATUS_UNKNOWN;
        stream_quality = stream_quality_values_360[stream_quality];
    } else if (stream_resolution == 480) {
        if (!XBMC->GetSetting("tvserver_quality480", &stream_quality))
            return ADDON_STATUS_UNKNOWN;
        stream_quality = stream_quality_values_480[stream_quality];
    } else if (stream_resolution == 576) {
        if (!XBMC->GetSetting("tvserver_quality576", &stream_quality))
            return ADDON_STATUS_UNKNOWN;
        stream_quality = stream_quality_values_576[stream_quality];
    } else if (stream_resolution == 720) {
        if (!XBMC->GetSetting("tvserver_quality720", &stream_quality))
            return ADDON_STATUS_UNKNOWN;
        stream_quality = stream_quality_values_720[stream_quality];
    } else if (stream_resolution == 1080) {
        if (!XBMC->GetSetting("tvserver_quality1080", &stream_quality))
            return ADDON_STATUS_UNKNOWN;
        stream_quality = stream_quality_values_1080[stream_quality];
    }

    return ADDON_STATUS_OK;
}

void ADDON_Stop() {}
void ADDON_FreeSettings() {}

/***********************************************************
 * PVR Client AddOn specific public library functions
 ***********************************************************/

const char* GetPVRAPIVersion(void)
{
  static const char *strApiVersion = XBMC_PVR_API_VERSION;
  return strApiVersion;
}
const char* GetMininumPVRAPIVersion(void)
{
  static const char *strMinApiVersion = XBMC_PVR_MIN_API_VERSION;
  return strMinApiVersion;
}

/***********************************************************
 * PVR server methods
 ***********************************************************/
PVR_ERROR GetAddonCapabilities(PVR_ADDON_CAPABILITIES* pCapabilities)
{
    XBMC->Log(LOG_NOTICE, "PVRCLIENT: GetAddonCapabilities()");
    pCapabilities->bSupportsEPG             = true;
    pCapabilities->bSupportsTV              = true;
    pCapabilities->bSupportsRadio           = false;
    pCapabilities->bSupportsRecordings      = false;
    pCapabilities->bSupportsTimers          = false;
    pCapabilities->bSupportsChannelGroups   = false;
    pCapabilities->bSupportsChannelScan     = false;
    pCapabilities->bHandlesInputStream      = false;
    pCapabilities->bHandlesDemuxing         = false;
    return PVR_ERROR_NO_ERROR;
}
const char *GetBackendName(void)
{
    XBMC->Log(LOG_NOTICE, "PVRCLIENT: GetBackendName()");
    static const char *strBackendName = "CetonTV";
    return strBackendName;
}
const char *GetBackendVersion(void)
{
    XBMC->Log(LOG_NOTICE, "PVRCLIENT: GetBackendVersion()");
    static std::string strBackendVersion = "v2.0.0";
    return strBackendVersion.c_str();
}
const char *GetConnectionString(void)
{
    XBMC->Log(LOG_NOTICE, "PVRCLIENT: GetConnectionString()");
    static std::string strConnectionString = "connected";
    return strConnectionString.c_str();
}
PVR_ERROR GetDriveSpace(long long *iTotal, long long *iUsed)
{
    *iTotal = 1024 * 1024 * 1024;
    *iUsed  = 0;
    return PVR_ERROR_NO_ERROR;
}

/***********************************************************
 * PVR EPG methods
 ***********************************************************/
PVR_ERROR GetEPGForChannel(ADDON_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd)
{
    if (!m_bCreated) return PVR_ERROR_SERVER_ERROR;
    XBMC->Log(LOG_NOTICE, "PVRCLIENT: GetEPGForChannel()");
    //if (m_data) return m_data->GetEPGForChannel(handle, channel, iStart, iEnd);
    GetEPGDataForChannel(handle, channel.iUniqueId, iStart, iEnd);
    return PVR_ERROR_NO_ERROR;
}

/***********************************************************
 * PVR channel methods
 ***********************************************************/
int GetChannelsAmount(void)
{
    if (!m_bCreated) return -1;
    char channel_len[6];
    snprintf(channel_len, 6, "%i", channels.size());
    XBMC->Log(LOG_NOTICE, (std::string("PVRCLIENT: GetChannelsAmount() - size = ")+channel_len).c_str());
    return channels.size();
}

PVR_ERROR GetChannels(ADDON_HANDLE handle, bool bRadio)
{
    if (!m_bCreated) return PVR_ERROR_SERVER_ERROR;
    XBMC->Log(LOG_NOTICE, "PVRCLIENT: GetChannels()");
    for (int i = 0; i < channels.size(); i++)
    {
        struct ChannelMap channel = channels.at(i);
        XBMC->Log(LOG_NOTICE, channel.name.c_str());

        PVR_CHANNEL xbmcChannel;
        memset(&xbmcChannel, 0, sizeof(PVR_CHANNEL));

        char strChannel[5];
        snprintf(strChannel, 5, "%i", channel.number);

        xbmcChannel.iUniqueId         = channel.number;
        xbmcChannel.bIsRadio          = bRadio;
        xbmcChannel.iChannelNumber    = channel.number;
        strncpy(xbmcChannel.strChannelName, channel.name.c_str(), sizeof(xbmcChannel.strChannelName) - 1);
        //strncpy(xbmcChannel.strInputFormat, "", sizeof(xbmcChannel.strInputFormat) - 1);
        strncpy(xbmcChannel.strStreamURL, (std::string("pvr://stream/tv/")+strChannel+".ts").c_str(), sizeof(xbmcChannel.strStreamURL) - 1);
        //stream.Format("pvr://stream/tv/%i.ts", tag.iUniqueId);
        xbmcChannel.iEncryptionSystem = 0;
        strncpy(xbmcChannel.strIconPath, "", sizeof(xbmcChannel.strIconPath) - 1);
        xbmcChannel.bIsHidden         = false;
        PVR->TransferChannelEntry(handle, &xbmcChannel);
    }
    return PVR_ERROR_NO_ERROR;
}

/***********************************************************
 * PVR live stream methods
 ***********************************************************/
bool OpenLiveStream(const PVR_CHANNEL &channel)
{
    XBMC->Log(LOG_NOTICE, "PVRCLIENT: OpenLiveStream()");
    return false;
}

void CloseLiveStream(void)
{
    XBMC->Log(LOG_NOTICE, "PVRCLIENT: CloseLiveStream()");
    m_bIsPlaying = false;
}

int GetCurrentClientChannel(void)
{
    XBMC->Log(LOG_NOTICE, "PVRCLIENT: GetCurrentClientChannel()");
    //return m_currentChannel.iUniqueId;
    return current_channel;
}

bool SwitchChannel(const PVR_CHANNEL &channel)
{
    XBMC->Log(LOG_NOTICE, "PVRCLIENT: SwitchChannel()");
    CloseLiveStream();
    return OpenLiveStream(channel);
}

PVR_ERROR SignalStatus(PVR_SIGNAL_STATUS &signalStatus)
{
    XBMC->Log(LOG_NOTICE, "PVRCLIENT: SignalStatus()");
    snprintf(signalStatus.strAdapterName, sizeof(signalStatus.strAdapterName), "pvr demo adapter 1");
    snprintf(signalStatus.strAdapterStatus, sizeof(signalStatus.strAdapterStatus), "OK");
    return PVR_ERROR_NO_ERROR;
}
const char *GetLiveStreamURL(const PVR_CHANNEL &channel)
{
    XBMC->Log(LOG_NOTICE, "PVRCLIENT: GetLiveStreamURL()");
    current_channel = channel.iChannelNumber;
    std::string stream_url;
    if (stream_resolution == 0)
        stream_url = StartTVStream(channel.iChannelNumber);
    else {
        TranscodeSettings ts;
        ts.video_resolution = stream_resolution;
        ts.video_bitrate = stream_quality;
        stream_url = StartTVStream(channel.iChannelNumber, ts);
    }
    return stream_url.c_str();
}
PVR_ERROR GetStreamProperties(PVR_STREAM_PROPERTIES* pProperties)
{
  return PVR_ERROR_NOT_IMPLEMENTED;
}
int GetChannelGroupsAmount(void)
{
  return -1;
}
PVR_ERROR GetChannelGroups(ADDON_HANDLE handle, bool bRadio)
{
  return PVR_ERROR_SERVER_ERROR;
}
PVR_ERROR GetChannelGroupMembers(ADDON_HANDLE handle, const PVR_CHANNEL_GROUP &group)
{
  return PVR_ERROR_SERVER_ERROR;
}
int GetRecordingsAmount(void)
{
  return -1;
}
PVR_ERROR GetRecordings(ADDON_HANDLE handle)
{
  return PVR_ERROR_NOT_IMPLEMENTED;
}

/***********************************************************
 * UNUSED API FUNCTIONS
 ***********************************************************/
PVR_ERROR DialogChannelScan(void) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR CallMenuHook(const PVR_MENUHOOK &menuhook) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR DeleteChannel(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR RenameChannel(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR MoveChannel(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR DialogChannelSettings(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR DialogAddChannel(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
bool OpenRecordedStream(const PVR_RECORDING &recording) { return false; }
void CloseRecordedStream(void) {}
int ReadRecordedStream(unsigned char *pBuffer, unsigned int iBufferSize) { return 0; }
long long SeekRecordedStream(long long iPosition, int iWhence /* = SEEK_SET */) { return 0; }
long long PositionRecordedStream(void) { return -1; }
long long LengthRecordedStream(void) { return 0; }
void DemuxReset(void) {}
void DemuxFlush(void) {}
int ReadLiveStream(unsigned char *pBuffer, unsigned int iBufferSize) { return 0; }
long long SeekLiveStream(long long iPosition, int iWhence /* = SEEK_SET */) { return -1; }
long long PositionLiveStream(void) { return -1; }
long long LengthLiveStream(void) { return -1; }
PVR_ERROR DeleteRecording(const PVR_RECORDING &recording) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR RenameRecording(const PVR_RECORDING &recording) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR SetRecordingPlayCount(const PVR_RECORDING &recording, int count) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR SetRecordingLastPlayedPosition(const PVR_RECORDING &recording, int lastplayedposition) { return PVR_ERROR_NOT_IMPLEMENTED; }
int GetRecordingLastPlayedPosition(const PVR_RECORDING &recording) { return -1; }
int GetTimersAmount(void) { return -1; }
PVR_ERROR GetTimers(ADDON_HANDLE handle) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR AddTimer(const PVR_TIMER &timer) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR DeleteTimer(const PVR_TIMER &timer, bool bForceDelete) { return PVR_ERROR_NOT_IMPLEMENTED; }
PVR_ERROR UpdateTimer(const PVR_TIMER &timer) { return PVR_ERROR_NOT_IMPLEMENTED; }
void DemuxAbort(void) {}
DemuxPacket* DemuxRead(void) { return NULL; }
unsigned int GetChannelSwitchDelay(void) { return 0; }
void PauseStream(bool bPaused) {}
bool CanPauseStream(void) { return false; }
bool CanSeekStream(void) { return false; }
bool SeekTime(int,bool,double*) { return false; }
void SetSpeed(int) {};
}
