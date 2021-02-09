#include "log.h"

namespace Log{

    pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
    std::unique_ptr<LoggingAsync> LoggingAsync_;
    std::string logFileName_ = "./Logfile.log";

    void once_init()
    {
        LoggingAsync_.reset(new LoggingAsync(logFileName_));
        LoggingAsync_->start();
    }

}
