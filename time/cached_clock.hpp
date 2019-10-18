#ifndef WCCS_TIME_CACHED_CLOCK_H
#define WCCS_TIME_CACHED_CLOCK_H

#include <cstdint>
#include <chrono>

namespace wccs::time {
    class cached_clock {
    public:
        // 可选提供当前参照时间
        explicit cached_clock(std::chrono::milliseconds current)
        : cached_(current.count())
        , steady_(std::chrono::steady_clock::now()) {
            if(cached_ == 0) {
                cached_ = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();
            }
        }
        // 系统时钟 
        operator std::chrono::system_clock::time_point() const { 
            return std::chrono::system_clock::time_point(
                std::chrono::milliseconds( static_cast<std::int64_t>(*this) ));
        }
        // 时间戳：秒
        operator std::uint32_t() const {
            return cached_ / 1000 +
                std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - steady_).count();
        }
        // 时间戳：毫秒
        operator std::int64_t() const {
            return cached_ +
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - steady_).count();
        }
    private:
        std::int64_t cached_; // 毫秒
        std::chrono::steady_clock::time_point steady_;
    };
}

#endif // WCCS_TIME_CACHED_CLOCK_H