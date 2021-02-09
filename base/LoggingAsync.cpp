#include "LoggingAsync.h"
#include "LogFile.h"
#include <assert.h>

LoggingAsync::LoggingAsync(const std::string basename, int flushInterval)
    :flushInterval_(flushInterval),
     running_(false),
     basename_(basename),
     thread_(std::bind(&LoggingAsync::threadFunc,this),"Logging"), 
     mutex_(),
     cond_(mutex_),
     latch_(1),
     currentBuffer_(new Buffer),
     nextBuffer_(new Buffer),
     buffers_()
{
    assert(basename.size() > 1);
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(4);   //按照书上的说法，正常buffers中最多会存有三个BufferPtr，但muduo的源码中却申请的是16个单元，为什么？
}



void LoggingAsync::append(const char* logline, int len)
{
    MutexLockGuard lock(mutex_);
    if(currentBuffer_->avail() > len)
    {
        currentBuffer_->append(logline,len);
    }
    else
    {
        buffers_.push_back(std::move(currentBuffer_));

        if(nextBuffer_)
        {
            currentBuffer_ = std::move(nextBuffer_);
        }
        else  //日志写入量突增，临时分配一个新的buffer（少数情况）
        {
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline, len);
        cond_.notify_one();
    }
}

void LoggingAsync::threadFunc()
{
    assert(running_);
    latch_.countDown();

    LogFile output(basename_);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    BufferVector buffersToWrite;
    buffersToWrite.reserve(4);
    while(running_ || !currentBuffer_->empty()) // linya的WebServer源码 和muduo 中对这里都只进行了running 的判断，但我测试log的时候,发现
                                                //日志不能很好的写入文件，发生了race comption，设想：
                                                //1.程序在进行到while循环前就提前stop()了，此时running_会被置为false,结果当前currentBuffer_中的数据就没能成功的写入
                                                //2.主线程在日志线程执行到临界区外后,append(),然后stop(),此时currentBuffer_ 中的数据也无法取出会出来
    {
        //临界区，取出要写的数据，交换buffer
        {
            MutexLockGuard lock(mutex_);
            if(buffers_.empty())
            {
                cond_.wait_seconds(flushInterval_);
            }
            std::cout<<"currentBuffer_ = "<<currentBuffer_->data()<<std::endl;
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if(!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        //assert(buffersToWrite.empty());
        for(size_t i = 0;i < buffersToWrite.size(); i++)
        {
            std::cout<<"buffersToWrite[i] = "<<buffersToWrite[i]->data()<<std::endl;
            output.append(buffersToWrite[i]->data(),buffersToWrite[i]->length());
        }

        if(buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }

        if(!newBuffer1)
        {
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->init();
        }

        if(!newBuffer2)
        {
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->init();
        }
        output.flush();
    }
    output.flush();
}




