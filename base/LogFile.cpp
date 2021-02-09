#include "LogFile.h"

LogFile::LogFile(const std::string& basename, int flushEveryN)
    :basename_(basename),
    flushEveryN_(flushEveryN),
    count_(0),
    mutex_(new MutexLock){
    
    file_.reset(new FileUtil(basename)); 
}


void LogFile::append(const char* logline, int len)
{
    MutexLockGuard lock(*mutex_); 
    append_unlocked(logline, len);
}

void LogFile::flush()
{
    MutexLockGuard lock(*mutex_);
    file_->flush();
}

void LogFile::append_unlocked(const char* logline, int len) //因为FileUtil 使用的是unlocked_write
{
    file_->append(logline,len);
    count_++;
    if(count_ > flushEveryN_){
        count_ = 0;
        file_->flush();
    }
}
