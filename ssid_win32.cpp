#include <cstdio>
#include <cstdint>
#include <memory>
#include <functional>
#include <Windows.h>
#include <wlanapi.h>
#include "ssid.hpp"
#include "utils.hpp"

std::string is_connected_to_ssid(std::initializer_list<std::string> prefer_list) {
    std::string not_connected = "false";
    DWORD current_version = 0;
    DWORD result = 0;

    HANDLE client_handle = NULL;
    AutoDeleter<void> client_handle_deleter([&client_handle](void*) {
        if (client_handle != NULL) {
            WlanCloseHandle(client_handle, NULL);
            client_handle = NULL;
        }
    });

    WLAN_INTERFACE_INFO_LIST* if_list = nullptr;
    AutoDeleter<void> if_list_deleter([&if_list](void*) {
        if (if_list != nullptr) {
            WlanFreeMemory(if_list);
            if_list = nullptr;
        }
    });

    result = WlanOpenHandle(2, nullptr, &current_version, &client_handle);
    if (result != ERROR_SUCCESS) {
        printf("WlanOpenHandle failed with error: %u\n", result);
        return not_connected;
    }

    result = WlanEnumInterfaces(client_handle, nullptr, &if_list);
    if (result != ERROR_SUCCESS) {
        printf("WlanEnumInterfaces failed with error: %u\n", result);
        return not_connected;
    }

    for (unsigned int i = 0; i < if_list->dwNumberOfItems; i++) {
        WLAN_INTERFACE_INFO* if_info = static_cast<WLAN_INTERFACE_INFO*>(&if_list->InterfaceInfo[i]);

        if (if_info->isState != wlan_interface_state_connected) {
            continue;
        }

        DWORD connect_info_size = sizeof(WLAN_CONNECTION_ATTRIBUTES);
        WLAN_OPCODE_VALUE_TYPE opcode = wlan_opcode_value_type_invalid;
        WLAN_CONNECTION_ATTRIBUTES* connect_info = nullptr;
        AutoDeleter<void> connect_info_deleter([&connect_info](void*) {
            if (connect_info != nullptr) {
                WlanFreeMemory(connect_info);
                connect_info = nullptr;
            }
        });

        result = WlanQueryInterface(client_handle,
                                    &if_info->InterfaceGuid,
                                    wlan_intf_opcode_current_connection,
                                    nullptr,
                                    &connect_info_size,
                                    (PVOID*)&connect_info,
                                    &opcode);

        if (result != ERROR_SUCCESS) {
            printf("WlanQueryInterface failed with error: %u\n", result);
            return not_connected;
        } else {
            std::string ssid;
            for (unsigned int k = 0;
                k < connect_info->wlanAssociationAttributes.dot11Ssid.uSSIDLength;
                k++) {
                ssid.push_back(connect_info->wlanAssociationAttributes.dot11Ssid.ucSSID[k]);
            }
            for (auto& prefer : prefer_list) {
                if (ssid == prefer) {
                    printf("Interface [%S] has connected to SSID [%s]\n", if_info->strInterfaceDescription, ssid.c_str());
                    return ssid;
                }
            }
        }
    }

    return not_connected;
}