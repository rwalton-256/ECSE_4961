#include <list>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <cassert>

template<typename T>
class thread_safe_queue
{
private:
    std::list<T> _mDataList;
    std::condition_variable _mCondVariable;
    std::mutex _mMutex;
    std::atomic<bool> _mWriteDone;
public:
    thread_safe_queue() : _mWriteDone( false ) {}
    void push_back( const T& _aVal )
    {
        std::unique_lock<std::mutex> lock( _mMutex );
        _mDataList.push_back( _aVal );
        _mCondVariable.notify_one();
    }
    bool pop_front( T& _aVal )
    {
        std::unique_lock<std::mutex> lock( _mMutex );
        while( true )
        {
            if( _mWriteDone.load() && _mDataList.size() == 0 )
            {
                return false;
            }
            if( _mDataList.size() )
            {
                _aVal = _mDataList.front();
                _mDataList.pop_front();
                return true;
            }
            _mCondVariable.wait( lock );
        }
    }
    void write_done()
    {
        _mWriteDone.store( true );
        _mCondVariable.notify_all();
    }
};
