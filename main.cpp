#include <iostream>
#include <string>
#include <curl/curl.h>
#include "account.hpp"
#include "ssid.hpp"
#include "utils.hpp"

#define ENABLE_PROXY curl_easy_setopt(curl, CURLOPT_PROXY, "http://127.0.0.1:8888")

enum SSIDType {
    k0001SoftBank = 0,
    kMobilePoint1 = 1
};

const char* kUserAgent = "User-Agent: Mozilla/5.0 (iPad; CPU OS 9_1 like Mac OS X) AppleWebKit/601.1.46 (KHTML, like Gecko) Version/9.0 Mobile/13B137 Safari/601.1";

void do_softbank_login(SSIDType type, const char* loginurl);
bool check_connect_success();
size_t drop_libcurl_data(void* buffer, size_t size, size_t nmemb, void* userdata);

int main() {
#ifndef NDEBUG
    AutoDeleter<void> pause([](void*) {
        system("pause");
    });
#endif

    std::cout << "Softbank Wi-Fi Spot (0001softbank) Login Tool\n\n";

    SSIDType ssid_type = k0001SoftBank;
    std::string connected_ssid = is_connected_to_ssid({ "0001softbank", "mobilepoint1" });

    if (connected_ssid == "false") {
        std::cout << "Please select 0001softbank / mobilepoint1 in available networks\n";
        std::cout << "Choose your SSID: [0]0001softbank  [1]mobilepoint1: ";
        
        std::string input_ssid;
        std::cin >> input_ssid;
        
        if (input_ssid == "0") {
            ssid_type = SSIDType::k0001SoftBank;
        } else if (input_ssid == "1") {
            ssid_type = SSIDType::kMobilePoint1;
        } else {
            std::cout << "Wrong SSID type, try again please\n";
            return -1;
        }
    } else {
        if (connected_ssid == "0001softbank") {
            ssid_type = SSIDType::k0001SoftBank;
        } else if (connected_ssid == "mobilepoint1") {
            ssid_type = SSIDType::kMobilePoint1;
        }
    }

    // =============== check login status ===============
    CURLcode result;
    CURL* curl = curl_easy_init();
    AutoDeleter<void> curl_deleter([&curl](void*) {
        if (curl != nullptr) {
            curl_easy_cleanup(curl);
            curl = nullptr;
        }
    });

    curl_easy_setopt(curl, CURLOPT_URL, "http://www.google.com/generate_204");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, kUserAgent);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, drop_libcurl_data);
    ENABLE_PROXY;

    result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        std::cout << "Cannot connect to LoginServer\n";
        return -1;
    }
    
    int64_t response_code = 0;
    result = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (result == CURLE_OK && response_code == 204) {
        std::cout << "Already logged-in, exit\n";
        return 0;
    }

    if (response_code != 301 && response_code != 302) {
        std::cout << "Invalid response code " << response_code << std::endl;
        return -1;
    }
    //================================================


    // get login url (redirect location)
    char* location = nullptr;
    result = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &location);
    if (location == nullptr || strlen(location) == 0) {
        std::cout << "Invalid redirect\n";
        return -1;
    }

    if (!starts_with(location, "https://plogin1.pub.w-lan.jp") && !starts_with(location, "https://www.login4.w-lan.jp")) {
        std::cout << "Invalid redirect location " << location << std::endl;
        return -1;
    }

    // ask for account if empty
    if (sws_username.length() == 0 || sws_password.length() == 0) {
        std::cout << "Username: ";
        std::cin >> sws_username;
        std::cout << "Password: ";
        std::cin >> sws_password;
    }

    // do login
    do_softbank_login(ssid_type, location);

    if (check_connect_success() == false) {
        std::cout << "Login failed\n";
        return -1;
    }

    std::cout << "Success\n";
    return 0;
}

void do_softbank_login(SSIDType type, const char* loginurl) {
    std::string login_url(loginurl);
    size_t wrs_pos = login_url.find("wrs");
    login_url.replace(wrs_pos, 3, "wrslogin");

    CURL* curl = curl_easy_init();
    AutoDeleter<void> curl_deleter([&curl](void*) {
        if (curl != nullptr) {
            curl_easy_cleanup(curl);
            curl = nullptr;
        }
    });

    curl_easy_setopt(curl, CURLOPT_URL, login_url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, kUserAgent);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, drop_libcurl_data);
    ENABLE_PROXY;

    std::string post_string;

    char* sws_username_encoded = curl_easy_escape(curl, sws_username.c_str(), sws_username.length());
    char* sws_password_encoded = curl_easy_escape(curl, sws_password.c_str(), sws_password.length());

    AutoDeleter<void> urlencoded_deleter(nullptr, [&sws_username_encoded, &sws_password_encoded](void*) {
        curl_free(sws_username_encoded);
        curl_free(sws_password_encoded);
        sws_username_encoded = nullptr;
        sws_password_encoded = nullptr;
    });

    if (type == SSIDType::k0001SoftBank) {
        post_string = "SWSUserName=" + std::string(sws_username_encoded) + "&SWSPassword=" + std::string(sws_password_encoded) + "&doLogin.x=15&doLogin.y=12";
    } else if (type == SSIDType::kMobilePoint1) {
        post_string = "UserName=" + std::string(sws_username_encoded) + "&Suffix=sbwifi.jp&Password=" + std::string(sws_password_encoded) + "&button=Login";
    }

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_string.c_str());
    
    CURLcode result = curl_easy_perform(curl);
}

bool check_connect_success() {
    CURL* curl = curl_easy_init();
    AutoDeleter<void> curl_deleter([&curl](void*) {
        if (curl != nullptr) {
            curl_easy_cleanup(curl);
            curl = nullptr;
        }
    });

    curl_easy_setopt(curl, CURLOPT_URL, "http://www.google.com/generate_204");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, kUserAgent);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, drop_libcurl_data);
    ENABLE_PROXY;

    CURLcode result = curl_easy_perform(curl);

    if (result == CURLE_OK) {
        int64_t response_code = 0;
        result = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (result == CURLE_OK && response_code == 204) {
            return true;
        }
    }

    return false;
}

size_t drop_libcurl_data(void* buffer, size_t size, size_t nmemb, void* userdata) {
    return size * nmemb;
}
