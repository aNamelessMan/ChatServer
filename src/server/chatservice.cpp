//相当于xxserviceimpl_pb    即具体的业务逻辑接口实现
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
    // LOG_INFO << "do login service!!!";
    int id{js["id"]};
    string pwd{js["password"]};

    User user = _userModel.query(id);
    if(user.getID() == id && user.getPwd() == pwd){
        if(user.getState() == "online"){
            // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该用户已经登录，不允许重复登录";
            conn->send(response.dump());
        }else{
            // 登录成功,更新用户信息为在线
            user.setState("online");
            _userModel.updateState(user);

            _userModel.updateState(user);
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getID();
            response["name"] = user.getName();
            conn->send(response.dump());
        }
    }else{
        // 该用户不存在/密码错误，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp){
    // LOG_INFO << "do reg service!!!";
    string name{js["name"]}, pwd{js["password"]};

    User user;
    user.setName(name);
    user.setPwd(pwd);

    if(_userModel.insert(user)){
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getID();
        conn->send(response.dump());
    }else{
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}