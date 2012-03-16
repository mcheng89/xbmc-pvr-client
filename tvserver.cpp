#include "tvserver.h"
#include "utils.h"
#include "client.h"
#include <json-c/json.h>

std::string StartTVStream(int channel, TranscodeSettings transcode) {
    char strChannel[5];
    snprintf(strChannel, 5, "%i", channel);

    std::string path = std::string("/play?channel=")+strChannel;
    if (transcode.video_bitrate!=0 && transcode.video_resolution!=0) {
        char strBitrate[6], strResolution[5];
        snprintf(strBitrate, 6, "%i", transcode.video_bitrate);
        snprintf(strResolution, 5, "%i", transcode.video_resolution);
        path = path + "&bitrate="+strBitrate + "&resolution="+strResolution;
    }
    XBMC->Log(ADDON::LOG_NOTICE, ("PVRCLIENT: StartTVStream() - "+path).c_str() );
    std::string data = httpRequest(stream_ip,stream_port,path,
                            (stream_secured?httpDigestHeaders(stream_username,stream_password,"GET",path):"")
                        );
    json_object *json_data = json_tokener_parse(data.c_str());
    if (json_data != 0) {
        json_object *stream_obj = json_object_object_get(json_data, "url");
        std::string stream_url = json_object_get_string(stream_obj);

        XBMC->Log(ADDON::LOG_NOTICE, ("PVRCLIENT: Playing stream url: "+stream_url).c_str() );
        json_object_put(json_data);

        return stream_url;
    }

    return "";
}

int LoadChannelMap(std::vector<struct ChannelMap> &channels) {
    channels.clear();

    std::string path = "/getChannels";
    XBMC->Log(ADDON::LOG_NOTICE, "PVRCLIENT: LoadChannelMap()" );
    XBMC->Log(ADDON::LOG_NOTICE, "PVRCLIENT: Downloading channelmap from server..." );
    std::string data = httpRequest(stream_ip,stream_port,path,
                            (stream_secured?httpDigestHeaders(stream_username,stream_password,"GET",path):"")
                        );
    if (data == "") {
        XBMC->Log(ADDON::LOG_NOTICE, "PVRCLIENT: Error downloading channelmap" );
        return -1;
    }

    XBMC->Log(ADDON::LOG_NOTICE, "PVRCLIENT: Parsing channelmap data..." );
    std::vector<std::string> lines = split(data,"\n",false);
    for (int i=0; i<lines.size(); i++) {
        //XBMC->Log(ADDON::LOG_NOTICE, ("PVRCLIENT: Channel - "+lines.at(i)).c_str() );
        std::vector<std::string> csv = split(lines.at(i),",",true);
        //XBMC->Log(ADDON::LOG_NOTICE, lines.at(i).c_str());
        if (csv.size() < 2) continue;
        struct ChannelMap channel;
        channel.number = atoi(csv.at(0).c_str());
        channel.name = csv.at(1);
        channels.push_back(channel);
    }
    return 0;
}

int GetEPGDataForChannel(ADDON_HANDLE handle, int channel, time_t iStart, time_t iEnd) {
    static int unique_id = 1;
    XBMC->Log(ADDON::LOG_NOTICE, "PVRCLIENT: GetEPGDataForChannel()" );

    if (iEnd-iStart>32400)
        iEnd = iStart+32400;
    for (int start=iStart; start<iEnd;) {
        char strBuffer[11], strChannel[11];
        snprintf(strChannel, 11, "%i", channel);
        snprintf(strBuffer, 11, "%i", start);
        std::string request = std::string("{\"start_time\":")+strBuffer+",";
        snprintf(strBuffer, 11, "%i", (start+10800>iEnd?iEnd:start+10800));
        request = request + "\"end_time\":"+strBuffer+",\"channels\":["+strChannel+"]}";

        start = start+10800;

        //XBMC->Log(ADDON::LOG_NOTICE, ("PVRCLIENT: "+request).c_str() );
        std::string path = "/getEPGData";
        std::string data = httpRequest(stream_ip,stream_port,path,
                                (stream_secured?httpDigestHeaders(stream_username,stream_password,"POST",path):"")
                            ,request);
        //XBMC->Log(ADDON::LOG_NOTICE, ("PVRCLIENT: "+data).c_str() );
        json_object *json_data = json_tokener_parse(data.c_str());
        if (json_data != 0) {
            json_object *channel_epg = json_object_object_get(json_data, strChannel);
            if (channel_epg) {
                int channel_len = json_object_array_length(channel_epg);
                for (int i=0; i<channel_len; i++) {
                    json_object *channel_obj = json_object_array_get_idx(channel_epg, i);

                    json_object *program_title_obj = json_object_object_get(channel_obj, "program_title");
                    json_object *program_subtitle_obj = json_object_object_get(channel_obj, "program_subtitle");
                    json_object *program_descript_obj = json_object_object_get(channel_obj, "program_descript");
                    json_object *start_time_obj = json_object_object_get(channel_obj, "start_time");
                    json_object *end_time_obj = json_object_object_get(channel_obj, "end_time");

                    EPG_TAG tag;
                    memset(&tag, 0, sizeof(EPG_TAG));
                    tag.iUniqueBroadcastId = unique_id++;
                    tag.strTitle           = json_object_get_string(program_title_obj);
                    tag.iChannelNumber     = channel;
                    tag.startTime          = json_object_get_int(start_time_obj);
                    tag.endTime            = json_object_get_int(end_time_obj);
                    tag.strEpisodeName     = json_object_get_string(program_subtitle_obj);
                    tag.strPlot            = json_object_get_string(program_descript_obj);
                    PVR->TransferEpgEntry(handle, &tag);

                    if (start < tag.endTime)
                        start = tag.endTime;
                }
            }
        }
        json_object_put(json_data);
    }

    return 0;
}
