#pragma once
#include <cstdint>
#include <atomic>
#include <chrono>
#include <iostream>

namespace flame { namespace time {
    // 偏移校准时钟
    template <typename DIFF_TYPE = std::atomic_int64_t>
    class delta_clock {
    public:
        // 构建，可选远端参照时间
        explicit delta_clock(std::chrono::milliseconds remote =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())) {
            swap(remote);
        }
        // 系统时钟 
        operator std::chrono::system_clock::time_point() const { 
            return std::chrono::system_clock::time_point(
                std::chrono::milliseconds( static_cast<std::int64_t>(*this) ));
        }
        // 时间戳：毫秒
        operator std::int64_t() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count() + diff_;
        }

        void swap(std::chrono::milliseconds r) {
            std::int64_t local = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now().time_since_epoch() ).count(),
                remote = std::chrono::duration_cast<std::chrono::milliseconds>( r ).count();

            diff_ = remote - local;
        }
        // 
        std::string iso() {
            std::string buffer(19, '\0');
            auto now = static_cast<std::chrono::system_clock::time_point>(*this);
            auto tt = std::chrono::system_clock::to_time_t(now);
            auto tm = std::localtime(&tt);
            sprintf(buffer.data(), "%04d-%02d-%02d %02d:%02d:%02d",
                1900 + tm->tm_year, 1+ tm->tm_mon, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
            return buffer;
        }
    private:
        DIFF_TYPE diff_; // 毫秒
    };
} }
