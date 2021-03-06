#pragma once
#include <boost/context/fiber.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/post.hpp>
#include <boost/system/error_code.hpp>
#include <thread>
#include <memory>
#include <cassert>

template <class CoroutineT>
class basic_coroutine_handler {
public:
    // 创建空的处理器
    basic_coroutine_handler()
    : count_(nullptr)
    , error_(nullptr) {}
    // 创建指定协程的处理器
    basic_coroutine_handler(std::shared_ptr<CoroutineT> co)
    : count_(nullptr)
    , error_(nullptr)
    , co_(co) {}
    // 复制
    basic_coroutine_handler(const basic_coroutine_handler& ch) = default;
    // 指定错误返回
    basic_coroutine_handler& operator[](boost::system::error_code& error) {
#ifndef NDEBUG
        assert(co_->id_ == std::this_thread::get_id());
#endif
        error_ = &error;
        return *this;
    }
    // 模拟的回调
    void operator()(const boost::system::error_code& error, std::size_t count = 0) {
        if(error_) *error_ = error;
        if(count_) *count_ = count;
        resume();
    }
    // 重置处理器
    void reset() {
        count_ = nullptr;
        error_ = nullptr;
        co_.reset();
    }
    // 重置处理器用于控制指定的协程
    void reset(std::shared_ptr<CoroutineT> co) {
        count_ = nullptr;
        error_ = nullptr;
        co_ = co;
    }
    // 协程暂停
    void yield() {
#ifndef NDEBUG // 仅在调试模式计算线程标识
        assert(co_->id_ == std::this_thread::get_id());
#endif
        // 一般当前上下文位于当前执行器（线程）
        co_->c2_ = std::move(co_->c2_).resume();
    }
    // 协程恢复
    void resume() {
        // 恢复实际执行的上下文，可能需要对应执行器（线程）
        boost::asio::post(co_->ex_, [co = co_] () {
            co->c1_ = std::move(co->c1_).resume();
        });
    }
private:
    std::size_t*                count_;
    boost::system::error_code*  error_;
    std::shared_ptr<CoroutineT>    co_;
    friend CoroutineT;
    friend class boost::asio::async_result<basic_coroutine_handler<CoroutineT>,
        void (boost::system::error_code error, std::size_t size)>;
};
// 协程
class coroutine {
public:
    // 注意：请使用 go 创建并启动协程
    template <class Executor>
    coroutine(const Executor& ex)
    : ex_(ex)
    , gd_(ex) {
        
    }
private:
    boost::context::fiber  c1_;
    boost::context::fiber  c2_;
    boost::asio::executor  ex_;
    boost::asio::executor_work_guard<boost::asio::executor> gd_; // 协程还未运行完毕时，阻止退出
#ifndef NDEBUG // 仅在调试模式计算线程标识
    std::thread::id        id_;
#endif
    friend class basic_coroutine_handler<coroutine>;
};
// 协程句柄
using coroutine_handler = basic_coroutine_handler<coroutine>;
// 默认上下文
boost::asio::io_context& default_context() {
    static boost::asio::io_context context;
    return context;
}
// 启动协程
template <class Executor, class Handler, class CoroutineHandler = coroutine_handler>
void go(const Executor& executor, Handler&& handler) {
    auto co = std::make_shared<coroutine>(executor); 
    // 在执行器上运行协程
    boost::asio::post(executor, [co, fn = std::move(handler)] () mutable {
#ifndef NDEBUG // 仅在调试模式计算线程标识
        co->id_ = std::this_thread::get_id();
#endif
        co->c1_ = boost::context::fiber([co, fn = std::move(fn)] (boost::context::fiber&& c2) mutable {
            CoroutineHandler ch(co);
            co->c2_ = std::move(c2);
            fn(ch); // 实际协程函数
            return std::move(co->c2_);
        });
        // coroutine::current = co;
        co->c1_ = std::move(co->c1_).resume();
    });
}
// 启动协程（默认执行器）
template <class Handler, class CoroutineHandler = coroutine_handler>
void go(Handler&& handler) {
    go(default_context().get_executor(), handler);
}
// 对 ASIO 异步函数调用时，存在下定义时，自动暂停协程
namespace boost { namespace asio {
    template <>
    class async_result<coroutine_handler, void (boost::system::error_code error, std::size_t size)> {
    public:
        explicit async_result(coroutine_handler& ch) : ch_(ch), count_(0) {
            ch_.count_ = &count_;
        }
        using completion_handler_type = coroutine_handler;
        using return_type = std::size_t;
        return_type get() {
            ch_.yield();
            return count_;
        }
    private:
        coroutine_handler &ch_;
        std::size_t           count_;
    };

    template <>
    class async_result<coroutine_handler, void (boost::system::error_code error)> {
    public:
        explicit async_result(coroutine_handler& ch) : ch_(ch) {
        }
        using completion_handler_type = coroutine_handler;
        using return_type = void;
        void get() {
            ch_.yield();
        }
    private:
        coroutine_handler &ch_;
    };
} } // namespace boost::asio
