#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "json.hpp"
using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

// 聊天服务器业务类
class ChatService{
public:
    // 获取单例对象的接口函数
    static ChatService* instance();
    void login(const TcpConnectionPtr &conn, json &js, Timestamp);
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp);
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp);
    MsgHandler getHandler(int msgid);
    void clientCloseException(const TcpConnectionPtr &conn);
    void reset();
private:
    ChatService();
    // 存储消息id和其对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;

    // 存储在线用户的通信连接   注意线程安全！  onMessage会在多个线程中执行 可能同时修改，但stl的容器是没有考虑线程安全问题的
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    // 定义互斥锁，保证_userConnMap的线程安全
    mutex _connMutex;

    // 数据操作类对象
    UserModel _userModel;

    OfflineMsgModel _offlineMsgModel;
};

#endif