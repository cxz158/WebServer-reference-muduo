/******************************************************************
*File Name: /home/cxz/code/mywork/net/Buffer.cpp
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Wed 24 Feb 2021 05:47:19 PM PST
***************************************************************/
#include "Buffer.h"
#include "../base/log.h"
#include <sys/types.h>
#include <sys/uio.h>
#include <assert.h>

int Buffer::BUFFSIZE = 1024;

//从文件描述符中读， 写入buffer
int Buffer::readfd(int fd)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writeable = writeableBytes(); 

    vec[0].iov_base = buffer_.data() + writeIndex_;
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int n = readv(fd, vec, 2);
    if(n < 0)
        log_syserr("Buffer::readfd failed - \n");
    else if( static_cast<size_t>(n) < writeable )
        writeIndex_ += n;
    else 
    {
        writeIndex_ += writeableBytes();
        append(extrabuf, n - writeable);
    }
    return n;
}

void Buffer::append(const char* data, size_t len)
{
    if(writeableBytes() == 0)
        std::copy(data, data+len, std::back_inserter(buffer_));
    else
        std::copy(data, data+len, buffer_.begin()+writeIndex_);
    writeIndex_ = buffer_.size(); 
}

char* Buffer::findLine()
{
    auto i = std::find(buffer_.begin()+readIndex_, buffer_.end(), '\n');
    if(i == buffer_.end())
        return  nullptr;
    else
        return i.base();
}

inline
void Buffer::init() {  
    assert(writeIndex_ == readIndex_);
    buffer_.resize(BUFFSIZE);
    bzero(buffer_.data(), std::min(writeIndex_, BUFFSIZE));
    writeIndex_ = readIndex_ = 0; 
}
