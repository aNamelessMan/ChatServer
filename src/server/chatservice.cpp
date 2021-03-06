//相当于xxserviceimpl_pb    即具体的业务逻辑接口实现
#include "chatservice.hpp"
#include "public.hpp"
#include <string>
#include <muduo/base/Logging.h>
#include <map>

using namespace muduo;
using namespace std;

ChatService* ChatService::instance(){
    static ChatService service;
    return &service;
}

ChatService::ChatService(){
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
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
// {"msgid":1, "id":1, "password":"123456"}
// {"msgid":1, "id":2, "password":"666666"}
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
            // 登录成功，记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);// 参数传引用，给connMutex上锁
                _userConnMap.insert({id, conn});
            }// 出作用域，lock_guard析构函数给mutex解锁

            // 登录成功,更新用户信息为在线
            user.setState("online");
            _userModel.updateState(user);

            _userModel.updateState(user);
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getID();
            response["name"] = user.getName();

            // 查询用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty()){
                response["offlinemsg"] = vec;
                // 读取该用户的离线消息后，把该用户的所有离线消息删除掉
                _offlineMsgModel.remove(id);
            }
            // 查询用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty()){
                vector<string> vec2;
                for(User &user:userVec){
                    json js;
                    js["id"] = user.getID();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

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

// {"msgid":3, "name":"li si", "password":"666666"}
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

void ChatService::clientCloseException(const TcpConnectionPtr &conn){
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for(auto iter = _userConnMap.begin(); iter != _userConnMap.end(); iter++){
            if(iter->second == conn){
                // 从map中删除用户的连接信息
                user.setID(iter->first);
                _userConnMap.erase(iter);
                break;
            }
        }
    }
    // 更新用户状态为offline
    if(user.getID() != -1){
        user.setState("offline");
        _userModel.updateState(user);
    }
}

// {"msgid":5, "id":1, "from":"zhang san", "to":2, "msg":"hello"}
// {"msgid":5, "id":2, "from":"li si", "to":1, "msg":"world"}
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp){
    int toid{js["to"].get<int>()};

    {
        lock_guard<mutex> lock(_connMutex);
        auto iter = _userConnMap.find(toid);
        if(iter != _userConnMap.end()){
            // toid在线，转发消息   服务器主动推送消息给toid用户
            iter->second->send(js.dump());
            return;
        }
        // toid不在线，存储离线消息
        _offlineMsgModel.insert(toid, js.dump());
    }// 尽可能将 使用map的范围/锁的粒度 减小，使更多语句可以并行
    // 只要用到map就必须加锁！
}

// 服务器异常，业务重置方法
void ChatService::reset(){
    // 把online状态的用户设置为offline
    _userModel.resetState();
}

//  添加好友业务 msgid id firendid
// {"msgid":6, "id":1, "friendid":2}
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 存储好友信息
    _friendModel.insert(userid, friendid);
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp &time){
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupname"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);
    if(_groupModel.createGroup(group)){
        // 存储群组创建人信息
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp &time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp &time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for(int id:useridVec){
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end()){
            // 转发群消息
            it->second->send(js.dump());
        }else{
            // 存储离线群消息
            _offlineMsgModel.insert(id, js.dump());
        }
    }
}

