#include <chrono>
#include <iostream>
#include <string>
#include <map>
#include <iomanip>

class Tracer
{
private:
    std::chrono::high_resolution_clock::time_point _mStartTime;
    std::string _mName;
    std::string _mCurrTrace;
    std::chrono::high_resolution_clock::time_point _mTraceStart;
    std::map<std::string, std::chrono::duration<double> > _mTraceMap;
    bool progress_update;
public:
    Tracer( const std::string& _aName )
      : _mStartTime(std::chrono::high_resolution_clock::now()),
        _mName( _aName ),
        progress_update( false )
    {
        std::cout << "\033[33m" << "Task " << _mName << " started" << "\033[39m" << std::endl;
    }
    ~Tracer()
    {
        if( _mCurrTrace != "" )
        {
            end_trace();
        }
        if( progress_update )
        {
            std::cout << "\033[1A" << "\033[K";
        }
        std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - _mStartTime;
        std::cout << "\033[32m" << "Task " << _mName << " finished after " << std::setprecision( 6 ) << elapsed.count() << " seconds" << "\033[39m" << std::endl;
        for( auto itr=_mTraceMap.begin(); itr!=_mTraceMap.end(); itr++ )
        {
            std::cout << "   sub_task " << itr->first << " took " << itr->second.count() << " seconds, "
                      << itr->second.count() / elapsed.count() * 100 << "\% of total time" << std::endl;
        }
    }
    void end_trace()
    {
        std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - _mTraceStart;
        if( _mTraceMap.find(_mCurrTrace) != _mTraceMap.end() )
        {
            _mTraceMap[_mCurrTrace] += elapsed;
        }
        else
        {
            _mTraceMap[_mCurrTrace] = elapsed;
        }
    }
    void begin_trace( const std::string& _aTrace )
    {
        if( _mCurrTrace != "" )
        {
            end_trace();
        }
        _mTraceStart = std::chrono::high_resolution_clock::now();
        _mCurrTrace = _aTrace;
    }
    void update_progress( float progress )
    {
        if( progress_update == false )
        {
            progress_update = true;
            std::cout << std::endl;
        }
        if( progress < 0.0F ) progress = 0.0F;
        if( progress > 1.0F ) progress = 1.0F;

        std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - _mStartTime;

        std::cout << "\033[1A" << "\033[K"
                  << std::fixed << std::showpoint << std::setw( 5 ) << std::setprecision( 1 )
                  << progress * 100.0F << "% ---- " << elapsed.count() << "s" << std::endl;
    }
};
