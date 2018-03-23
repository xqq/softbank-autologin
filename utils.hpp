#ifndef _SOFTBANK_AUTOLOGIN_UTILS_HPP
#define _SOFTBANK_AUTOLOGIN_UTILS_HPP

#include <functional>

template <typename T>
class AutoDeleter {
public:
    explicit AutoDeleter(std::function<void(T*)> deleter) : user_data_(nullptr), deleter_(deleter) { }

    AutoDeleter(T* user_data, std::function<void(T*)> deleter) : user_data_(user_data), deleter_(deleter) { }

    ~AutoDeleter() {
        deleter_(user_data_);
    }
private:
    // Disallow copy and assign
    AutoDeleter(const AutoDeleter&) = delete;
    AutoDeleter& operator=(const AutoDeleter&) = delete;
private:
    T* user_data_;
    std::function<void(T*)> deleter_;
};


inline bool starts_with(const char* in, const char* prefix) {
    size_t prefix_length = strlen(prefix);
    size_t in_length = strlen(in);
    return in_length < prefix_length ? false : strncmp(prefix, in, prefix_length) == 0;
}


#endif // _SOFTBANK_AUTOLOGIN_UTILS_HPP