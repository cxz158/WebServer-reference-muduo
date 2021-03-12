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
bool Buffer::readfd(int fd)
{
    while(true)
    {
        char extrabuf[1024];
        struct iovec vec[2];
        const size_t writeable = writeableBytes(); 
        vec[0].iov_base = buffer_.data() + writeIndex_;
        vec[0].iov_len = writeable;
        vec[1].iov_base = extrabuf;
        vec[1].iov_len = sizeof extrabuf;
        int n = readv(fd, vec, 2);

        if(n == 0)
            return true;
        if(n < 0)
        {
            if(errno != EAGAIN)
                log("Buffer::readfd error!\n");
            break;
        }
        else if( static_cast<size_t>(n) < writeable ) //n > 0
            writeIndex_ += n;
        else 
        {
            writeIndex_ += writeableBytes();
            append(extrabuf, n - writeable);
        }
    }
    return false;
}

void Buffer::append(const char* data, size_t len)
{
    if(writeableBytes() < len)
        buffer_.resize(buffer_.size() + len);
    std::copy(data, data+len, buffer_.begin()+writeIndex_);
    writeIndex_ += len; 
}

char* Buffer::findLine()
{
    auto i = std::find(buffer_.begin()+readIndex_, buffer_.end(), '\n');
    if(i == buffer_.end())
        return  nullptr;
    else
        return i.base();
}

void Buffer::init() {  
    if(writeIndex_ == readIndex_)
    {
        buffer_.resize(BUFFSIZE);
        bzero(buffer_.data(), std::min(buffer_.size(),writeIndex_));
        writeIndex_ = readIndex_ = checkInex_ = 0; 
    }
    else
    {
        int readable = readableBytes();
        std::copy(buffer_.begin()+readIndex_, buffer_.begin()+writeIndex_,
                  buffer_.begin());
        readIndex_ = checkInex_ = 0;
        writeIndex_ = readable;
    }
}
