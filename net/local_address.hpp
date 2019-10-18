#ifndef WCCS_NET_LOCAL_ADDRESS_H
#define WCCS_NET_LOCAL_ADDRESS_H

#include <ifaddrs.h> // getifaddrs / freeifaddrs
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <cstring> // std::memcpy

namespace whol {
    // 获取本地内网地址
    template <int ADDRESS_FAMILY>
    class local_address {
    public:
        // 构建并获取本级内网地址
        local_address() {
#if defined(__ANDROID_API__) && __ANDROID_API__ < 24
        std::memset(&addr_, 0, sizeof(addr_));
#else
        struct ifaddrs* addresses;
        if(getifaddrs(&addresses) != 0) {
            throw std::runtime_error("failed to fetch local address");
        }
        addr_.ss_family = AF_UNSPEC;
        for(ifaddrs* address = addresses; address != nullptr; address = address->ifa_next) {
            if(ADDRESS_FAMILY == address->ifa_addr->sa_family && is_local(address->ifa_addr)) {
                std::memcpy(&addr_, address->ifa_addr, 
                    ADDRESS_FAMILY == AF_INET6 ? sizeof(struct sockaddr_in6) :
                        sizeof(struct sockaddr_in));
            }
        }
        freeifaddrs(addresses);
        if(addr_.ss_family == AF_UNSPEC) {
            throw std::runtime_error("failed to fetch local address");
        }
#endif
    }
        // 返回地址的二进制数据
        const unsigned char* to_bytes() const {
            switch(addr_.ss_family) {
            case AF_INET:
                return reinterpret_cast<const unsigned char*>(
                    &reinterpret_cast<const struct sockaddr_in*>(&addr_)->sin_addr);
            case AF_INET6:
                return reinterpret_cast<const unsigned char*>(
                    &reinterpret_cast<const struct sockaddr_in6*>(&addr_)->sin6_addr);
            default:
                return nullptr;
            }
        }
        // 返回地址的文本表达形式
        std::string to_string() const {
            std::string buffer(40, '\0');
            int nprint = 0;
            const unsigned char* raw = to_bytes();
            switch(addr_.ss_family) {
            case AF_INET:
                nprint = std::sprintf(buffer.data(), "%d.%d.%d.%d",
                    raw[0], raw[1], raw[2], raw[3]);
                break;
            case AF_INET6:
                nprint = std::sprintf(buffer.data(), "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                    raw[0], raw[1], raw[2], raw[3], raw[4],
                    raw[5], raw[6], raw[7], raw[8], raw[9], 
                    raw[10],raw[11],raw[12],raw[13],raw[14], raw[15]);
                break;
            }
            buffer.resize(nprint);
            return buffer;
        }
    private:
        // 存储地址
        struct sockaddr_storage addr_;
        // 用于判定内网地址
        static bool is_local(struct sockaddr* addr) {
            switch(addr->sa_family) {
                case AF_INET: {
                    auto in4 = reinterpret_cast<struct sockaddr_in*>(addr);
                    return (in4->sin_addr.s_addr & 0x000000ff) == 0x0000000a // 10.0.0.0/8
                        || (in4->sin_addr.s_addr & 0x0000f0ff) == 0x000010ac // 172.16.0.0/12
                        || (in4->sin_addr.s_addr & 0x0000ffff) == 0x0000a8c0; // 192.168.0.0/16
                }
                case AF_INET6: {
                    auto in6 = reinterpret_cast<struct sockaddr_in6*>(addr);
                    std::uint16_t x = *reinterpret_cast<std::uint16_t*>(&in6->sin6_addr);
                    return x & 0x80fe; // fe80::/10
                }
                default:
                    return false;
            }
        }
    };
}

#endif // WCCS_NET_LOCAL_ADDRESS_H