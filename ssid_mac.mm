#import <CoreWLAN/CoreWLAN.h>
#include "ssid.hpp"

std::string is_connected_to_ssid(std::initializer_list<std::string> prefer_list) {
    std::string not_connected = "false";

    CWInterface* wifi = [[CWWiFiClient sharedWiFiClient] interface];
    const char* ssid = [wifi.ssid UTF8String];

    if (ssid == nullptr) {
        return not_connected;
    }

    for (auto& prefer : prefer_list) {
        if (ssid == prefer) {
            printf("Interface [%s] has connected to SSID [%s]\n", [wifi.interfaceName UTF8String], ssid);
            return ssid;
        }
    }

    return not_connected;
}
