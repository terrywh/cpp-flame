#ifndef WCCS_TIME_CACHED_CLOCK_H
#define WCCS_TIME_CACHED_CLOCK_H

#include <cstdint>
#include <atomic>
#include <chrono>
#include <iostream>

namespace wccs::time {
    template <typename DIFF_TYPE = std::atomic_int64_t>
    class cached_clock {
    public:
        // 可选提供当前参照时间
        explicit cached_clock(std::chrono::milliseconds current =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())) {
            swap(current);
        }
        // 系统时钟 
        operator std::chrono::system_clock::time_point() const { 
            return std::chrono::system_clock::time_point(
                std::chrono::milliseconds( static_cast<std::int64_t>(*this) ));
        }
        // 时间戳：秒
        operator std::uint32_t() const {
            return std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count() + (diff_ / 1000);
        }
        // 时间戳：毫秒
        operator std::int64_t() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count() + diff_;
        }

        void swap(std::chrono::milliseconds current = std::chrono::milliseconds(0)) {
            std::int64_t local = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now().time_since_epoch() ).count(),
                server = std::chrono::duration_cast<std::chrono::milliseconds>( current ).count();

            diff_ = server - local;
        }
    private:
        DIFF_TYPE diff_; // 毫秒
    };
}

#endif // WCCS_TIME_CACHED_CLOCK_H