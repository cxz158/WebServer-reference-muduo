/******************************************************************
*File Name: HttpState.h
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Sun 28 Feb 2021 11:32:53 PM PST
***************************************************************/
#pragma once

/* 记录正在分析请求行， 请求头 */
enum class CHECK_STATE {
    CHECK_STATE_REQUESTLINE = 0, //请求行
    CHECK_STATE_HEADER,         //请求头
    CHECK_STATE_CONTENT         //请求体
};

/* 记录当前分析行的状态：1.收到一个完整的行 2.收到一个错误（非法）的行 3.行数据还不完整（没有读到/n/r） */
enum class LINE_STATUS{
    LINE_OK = 0,
    LINE_BAD,
    LINE_INCOMPLETE
};

/* http解析结果 */
enum class HTTP_CODE{
    NO_REQUEST,         //请求不完整
    GET_REQUEST,        //获得一个完整的请求
    BAD_REQUEST,        //获得一个错误的请求
    FORBIDDEN_REQUEST,  //权限不足
    INTERNAL_ERROR,     //系统错误
    FILE_REQUEST,       //请求文件
    CLOSED_CONNECTION,  //链接以关闭
};

/* 请求方式，目前仅考虑GET */
enum class METHOD
{
    GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH
};
