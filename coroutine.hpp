#ifndef WCCS_COROUTINE_H
#define WCCS_COROUTINE_H

#include <boost/context/fiber.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/system/error_code.hpp>
#include <thread>
#include <memory>
#include <cassert>

namespace wccs {
    // 内部实现类
    namespace detail {
        template <class CoroutineT>
        class coroutine_handler {
        public:

            coroutine_handler()
            : count_(nullptr)
            , error_(nullptr) {}

            coroutine_handler(std::shared_ptr<CoroutineT> co)
            : count_(nullptr)
            , error_(nullptr)
            , co_(co) {}

            coroutine_handler(const coroutine_handler& ch) = default;

            coroutine_handler& operator[](boost::system::error_code& error) {
                assert(co_->id_ == std::this_thread::get_id());
                error_ = &error;
                return *this;
            }

            void operator()(const boost::system::error_code& error, std::size_t count = 0) {
                if(error_) *error_ = error;
                if(count_) *count_ = count;
                resume();
            }

            void reset() {
                count_ = nullptr;
                error_ = nullptr;
                co_.reset();
            }

            void reset(std::shared_ptr<CoroutineT> co) {
                count_ = nullptr;
                error_ = nullptr;
                co_ = co;
            }

            void yield() {
                assert(co_->id_ == std::this_thread::get_id());
                // 一般当前上下文位于当前执行器（线程）
                co_->c2_ = std::move(co_->c2_).resume();
            }

            void resume() {
                // 恢复实际执行的上下文，可能需要对应执行器（线程）
                boost::asio::post(co_->ex_, [co = co_] () {
                    co->c1_ = std::move(co->c1_).resume();
                });
            }
        private:
            std::size_t*                count_;
            boost::system::error_code*  error_;
            std::shared_ptr<CoroutineT> co_;
            friend CoroutineT;
            friend class boost::asio::async_result<detail::coroutine_handler<CoroutineT>,
                void (boost::system::error_code error, std::size_t size)>;
        };

    }
    // 协程
    class coroutine {
    public:
        // 注意：请使用 coroutine::start 创建并启动协程
        template <class Executor>
        coroutine(Executor&& ex)
        : ex_(ex) {
            
        }

        template <class Executor, class Handler>
        static void start(Executor&& et, Handler&& fn) {
            auto co = std::make_shared<coroutine>(et);
            
            boost::asio::post(co->ex_, [co, fn] () mutable {
                co->id_ = std::this_thread::get_id();
                co->c1_ = boost::context::fiber([co, fn, gd = boost::asio::make_work_guard(co->ex_)] (boost::context::fiber&& c2) mutable {
                    detail::coroutine_handler<coroutine> ch(co);
                    co->c2_ = std::move(c2);
                    fn(ch);
                    return std::move(co->c2_);
                });
                // coroutine::current = co;
                co->c1_ = std::move(co->c1_).resume();
            });
        }
    private:
        boost::context::fiber c1_;
        boost::context::fiber c2_;
        boost::asio::executor ex_;
        std::thread::id       id_;

        friend class detail::coroutine_handler<coroutine>;
    };
    // 协程句柄
    using coroutine_handler = detail::coroutine_handler<coroutine>;
}

namespace boost::asio {
    template <>
    class async_result<wccs::coroutine_handler, void (boost::system::error_code error, std::size_t size)> {
    public:
        explicit async_result(wccs::coroutine_handler& ch) : ch_(ch), count_(0) {
            ch_.count_ = &count_;
        }
        using completion_handler_type = wccs::coroutine_handler;
        using return_type = std::size_t;
        return_type get() {
            ch_.yield();
            return count_;
        }
    private:
        wccs::coroutine_handler &ch_;
        std::size_t           count_;
    };

    template <>
    class async_result<wccs::coroutine_handler, void (boost::system::error_code error)> {
    public:
        explicit async_result(wccs::coroutine_handler& ch) : ch_(ch) {
        }
        using completion_handler_type = wccs::coroutine_handler;
        using return_type = void;
        void get() {
            ch_.yield();
        }
    private:
        wccs::coroutine_handler &ch_;
    };
} // namespace boost::asio

#endif // WCCS_COROUTINE_H
