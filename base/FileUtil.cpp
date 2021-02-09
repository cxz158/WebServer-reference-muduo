#include "FileUtil.h"
#include <string.h>
FileUtil::FileUtil(std::string filename): fp_(fopen(filename.c_str(),"ae")) //'e' mean O_CLOEXEC
{
    // 设置自己的缓冲区
    setbuffer(fp_, buffer_, sizeof(buffer_));
}

FileUtil::~FileUtil() { fclose(fp_); }

void FileUtil::append(const char* logline, const size_t len)
{
    size_t n = this->write(logline, len);
    size_t remain = len - n;
    while(remain > 0)//一次write未写完,还有剩余继续写
    {
        size_t x = this->write(logline + n, remain);
        if(x == 0)
        {
            int err = ferror(fp_);
            if(err)
                fprintf(stderr, "FileUtil::append() failed %s\n",strerror(err));
            break;
        }
        n += x;
        remain = len -n;
    }
}


void func(va_list args);
// Faster versions when locking is not required.
void FileUtil::flush() {fflush_unlocked(fp_);}

size_t FileUtil::write(const char* logline, size_t len)
{
    return fwrite_unlocked(logline, 1, len, fp_); //仅有日志处理程序向文件中写入，可以使用unlock版本
}
