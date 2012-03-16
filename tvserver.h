#ifndef TVSERVER_H
#define TVSERVER_H

#include <string>
#include <vector>
#include "xbmc/xbmc_pvr_types.h"

struct ChannelMap {
    int number;
    std::string name;
};

struct TranscodeSettings {
    int video_resolution;
    int video_bitrate;
    TranscodeSettings():video_resolution(0),video_bitrate(0){}
};

std::string StartTVStream(int channel, TranscodeSettings transcode=TranscodeSettings());
int LoadChannelMap(std::vector<struct ChannelMap> &channels);
int GetEPGDataForChannel(ADDON_HANDLE handle, int channel, time_t iStart, time_t iEnd);

#endif // TVSERVER_H
