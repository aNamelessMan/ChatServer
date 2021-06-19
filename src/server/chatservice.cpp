#include "chatservice.hpp"
#include "public.hpp"
#include <string>
#include <muduo/base/Logging.h>

using namespace muduo;
using namespace std;

ChatService* ChatService::instance(){
    static ChatService service;
    return &service;
}

ChatService::ChatService(){
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
}

MsgHandler ChatService::getHandler(int msgid){
    // 记录错误日志，msgid没有对应的处理函数
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end()){
        // 返回一个默认的处理，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp){
            LOG_ERROR << "msgid:" << msgid << "can not find handler!";
        };
    }else{
        return it->second;
    }
}

// 处理登录业务     ORM将业务层解耦和数据层 业务层操作的都是对象    DAO层
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp){
    LOG_INFO << "do login service!!!";
}

void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp){
    LOG_INFO << "do reg service!!!";
}