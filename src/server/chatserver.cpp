#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

#include <functional>
#include <string>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       const string& nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop){
        // 注册连接回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

        // 注册消息回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

        // 设置线程数量
        _server.setThreadNum(4);
    }

void ChatServer::start(){
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn){
    // 客户端断开连接
    if(!conn->connected()){
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time){
    string buf = buffer->retrieveAllAsString();
    // 数据反序列化
    json js = json::parse(buf);
    // 达到的目的：完全解耦网络模块的代码和业务模块的代码   即  不要指名道姓的调用业务模块的函数
    // 通过js["msgid"] 获取=》业务handler=》传入conn js time    
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    //回调对应的业务处理函数
    msgHandler(conn, js, time);
}