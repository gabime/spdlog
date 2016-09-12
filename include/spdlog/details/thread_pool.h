#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <deque>
#include <memory>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <stdexcept>
#include <assert.h>


typedef std::function<bool(void)> Func;
typedef std::shared_ptr<Func> shared_function_ptr;
typedef std::weak_ptr<Func> weak_function_ptr;

namespace spdlog
{
namespace details
{

class thread_pool {
public:
    thread_pool(size_t num_threads,
                const std::function<void()>& worker_warmup_cb = nullptr,
                const std::function<void()>& worker_teardown_cb = nullptr);

    void subscribe_handle(const shared_function_ptr& loop_handle );

    ~thread_pool();
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > _workers;
    // the task queue


    std::deque< weak_function_ptr > _loop_handles;
    std::deque< bool > _busy;

    // synchronization
    std::mutex _mutex;

    std::condition_variable _condition;

    std::function<void()> _worker_warmup_cb;

    std::function<void()> _worker_teardown_cb;

    size_t _index;

    bool _stop;
};

// the constructor just launches some amount of workers
inline thread_pool::thread_pool(size_t num_threads,
                                const std::function<void()>& worker_warmup_cb,
                                const std::function<void()>& worker_teardown_cb):
    _worker_warmup_cb(worker_warmup_cb),
    _worker_teardown_cb(worker_teardown_cb),
    _index(0),
    _stop(false)
{
    assert(num_threads > 0);

    for(size_t i = 0;i<num_threads;++i)
        _workers.emplace_back( [this]()
        {
            if( _worker_warmup_cb) _worker_warmup_cb();
            while( !_stop)
            {
                {
                    shared_function_ptr handle;
                    size_t index = 0;
                    bool is_busy = false;

                    // find an handle
                    {
                        std::unique_lock<std::mutex> lock(_mutex);

                        // scan the _loop_handles to find an available one.
                        for (size_t count=0; count<_loop_handles.size(); count++)
                        {
                            _index  = (_index+1) % _loop_handles.size();
                            handle  = _loop_handles[index].lock();
                            is_busy = _busy[index];

                            // if the weak pointer points to a delated handle, remove it
                            if(!handle ){
                                _loop_handles.erase( _loop_handles.begin() + index);
                                _busy.erase ( _busy.begin()  + index);
                                _index  = (_index) % _loop_handles.size();
                            }
                            else{
                                _busy[index] = true;
                            }
                            if( handle && !is_busy) break;
                        }
                    }

                    if(handle && !is_busy)
                    {
                        bool continue_loop = (*handle)();
                        if(!continue_loop){
                            _loop_handles.erase( _loop_handles.begin() + index);
                            _busy.erase ( _busy.begin()  + index);
                        }
                        else{
                                std::unique_lock<std::mutex> lock(_mutex);
                                _busy[index] = false;
                        }
                        // not busy anymore. notify to other threads
                        _condition.notify_one();
                    }
                    else{
                        // this happens when you didn't find an handle that is not busy
                        std::unique_lock<std::mutex> lock(_mutex);
                        _condition.wait(lock);
                    }
                }
            }
            if( _worker_teardown_cb) _worker_teardown_cb();
        }
        );
}

inline void thread_pool::subscribe_handle(const shared_function_ptr &loop_handle)
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _loop_handles.push_back( loop_handle );
        _busy.push_back( false );
    }
    _condition.notify_one();
}


// the destructor joins all threads
inline thread_pool::~thread_pool()
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _stop = true;
    }
    _condition.notify_all();

    for(std::thread &worker: _workers)
        worker.join();
}

}
}
#endif

