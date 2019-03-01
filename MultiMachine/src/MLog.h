#ifndef MLOG_H
#define MLOG_H

#include "ConfigLoad.h"
#include <memory>
#include <pthread.h>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <string>
#include <initializer_list>
#include <atomic>
#include <stdexcept>
#include <sys/time.h>

namespace nqueens
{
/**
 * Message Log
 * 功能：日志记录
 * 定义为singleton类，延迟加载，等待ConfigLoad获取配置文件名,pthread_once方式确保线程安全
 * 基于双缓冲队列，多线程写队列，单线程读队列并写入日志文件，定时写日志可让锁竞争更小
 * 双缓冲优点：1.append不用等待IO操作 2.合并多条日志，避免每一条日志都唤醒WriteLog，减少IO操作
 */
enum class Level
{
    INFO,
    WARN,
    ERROR
};
//utc_timer是为了减少 localtime_r 函数的调用，内部缓存了上一次记录的分钟和秒数，仅当分钟与缓存不一致
//时，才调用 localtime_r ，减少资源占用，在负载高时有更明显的性能提升
struct utc_timer
{
    utc_timer()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);        //获取时间
        _sys_acc_sec = tv.tv_sec;       //初始化时间缓存
        _sys_acc_min = _sys_acc_sec / 60;
        struct tm cur_tm;
        localtime_r((time_t*)&_sys_acc_sec, &cur_tm);   //转换为当地时间
        year = cur_tm.tm_year + 1900;                   
        mon  = cur_tm.tm_mon + 1;
        day  = cur_tm.tm_mday;
        hour  = cur_tm.tm_hour;
        min  = cur_tm.tm_min;
        sec  = cur_tm.tm_sec;           
        reset_utc_fmt();                    //初始化时间
    }

    char* get_curr_time(int* p_msec = NULL)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        if (p_msec)
            *p_msec = tv.tv_usec / 1000;
        if ((uint32_t)tv.tv_sec != _sys_acc_sec)
        {//如果不是同一秒内，刷新秒数，更新秒缓存
            sec = tv.tv_sec % 60;
            _sys_acc_sec = tv.tv_sec;
            if (_sys_acc_sec / 60 != _sys_acc_min)
            {//如果不是同一分钟内，刷新所有时间，更新缓存
                _sys_acc_min = _sys_acc_sec / 60;
                struct tm cur_tm;
                localtime_r((time_t*)&_sys_acc_sec, &cur_tm);
                year = cur_tm.tm_year + 1900;
                mon  = cur_tm.tm_mon + 1;
                day  = cur_tm.tm_mday;
                hour = cur_tm.tm_hour;
                min  = cur_tm.tm_min;
                reset_utc_fmt();
            }
            else
            {//同一分钟，只需要更新秒数
                reset_utc_fmt_sec();
            }
        }
        //return tv.tv_sec;
        return &utc_fmt[0];
    }

    int year, mon, day, hour, min, sec;
    char utc_fmt[20];

private:
    void reset_utc_fmt()
    {
        snprintf(utc_fmt, 20, "%d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);
    }
    
    void reset_utc_fmt_sec()
    {
        snprintf(utc_fmt + 17, 3, "%02d", sec);
    }

    uint64_t _sys_acc_min;
    uint64_t _sys_acc_sec;
};

class MLog
{
  public:
    static const std::shared_ptr<MLog> &GetIns() //返回唯一实例
    {
        pthread_once(&sm_pOnce, &MLog::init); //可以确保多线程时init函数只执行一次，确保多线程安全
        return sm_pIns;
    }
    void append(Level level, std::initializer_list<std::string> msg);
    void stop();                           //终止写日志线程，写日志线程绑定了一个MLog的智能指针，要依靠智能指针析构MLog，必须先终止写日志线程;
    static std::string GetErr(int errnum); //用于解析errno
    ~MLog();
    MLog(const MLog &) = delete;
    MLog &operator=(const MLog &) = delete;

  private:
    MLog(const std::string &stLogFile);
    static void init(); //pthread_once只执行一次的函数，用于初始化唯一实例
    void WriteLog();    //在写日志线程中运行

  private:
    static std::shared_ptr<MLog> sm_pIns; //静态唯一实例
    static pthread_once_t sm_pOnce;       //必须是非本地变量
    std::ofstream m_fOut;                 //日志文件输出流，销毁时自动close文件描述符
    //用于读取缓冲队列并写入日志文件的线程，不能在构造函数中初始化,
    //因为需要绑定sm_pIns(未构造完毕不能传入)，所以必须是heap object
    std::shared_ptr<std::thread> m_pWrThread;
    static const int sm_nBufSize = 20;  //缓冲区大小，以一条记录60个字符算，1024条记录是60KB，测试的时候设小点
    std::vector<std::string> m_vsCurr;  //待写入缓冲，让其他线程写入日志
    std::vector<std::string> m_vsWrite; //写入日志文件的缓冲
    size_t m_nIndexC;                   //记录m_vsCurr有多少条记录
    size_t m_nIndexW;                   //记录m_vsWrite有多少条记录
    std::mutex m_mutex;                 //对待写缓冲区加锁，两个条件变量共用一个mutex
    std::condition_variable m_WrCond;   //条件变量，定时写日志
    std::condition_variable m_EmCond;   //条件变量，等待待写入缓冲区被清空
    std::atomic<bool> m_bIsRun;         //原子类型，控制写日志文件线程是否继续运行
    utc_timer m_time;
};

//记录日志的文件和对应行
#define WHERE std::string(__FILE__) + ":" + std::to_string(__LINE__)

//有点丑陋宏：获取日志智能指针并调用append
#define _LOG MLog::GetIns()->append

#define STOP_LOG()              \
    do                          \
    {                           \
        MLog::GetIns()->stop(); \
    } while (0)

#define _GE MLog::GetErr

#define RUNTIME_ERROR()                                     \
    do                                                      \
    {                                                       \
        throw std::runtime_error(WHERE + " " + _GE(errno)); \
    } while (0)

#define THROW_EXCEPT(msg)              \
    do                                 \
    {                                  \
        throw std::runtime_error(msg); \
    } while (0)
}

#endif //MLOG_H