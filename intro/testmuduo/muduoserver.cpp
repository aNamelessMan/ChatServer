/**
 * muduo网络库主要给用户给用户提供了两个类
 * TcpServer：用于编写服务器程序
 * TcpClient：用于编写客户端程序
 * 
 * epoll + 线程池
 * 优点：把网络IO和业务代码分割开来
 *              暴露给业务代码的借口主要是  用户的连接和断开    用户的可读写事件
 * 
 * 
*/
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Timestamp.h>
#include <iostream>
#include <functional>
#include <string>

using namespace std;
using namespace muduo::net;
using namespace muduo;
using namespace std::placeholders;
/**
 * 基于muduo网络库开发服务器程序
 * 1.组合TcpServer对象
 * 2.创建EventLoop事件循环对象的指针
 * 3.根据TcpServer构造函数的参数自定义ChatServer的构造函数
 * 4.在构造函数中注册处理连接的回调函数和读写事件的回调函数
 * 5.设置合适的服务端线程数量，muduo会自动分配IO和worker线程
 * 
 * 使用muduo库编程，只需改写回调函数即可，其余部分都相当固定
*/
class ChatServer{
public:
    ChatServer(EventLoop* loop,             //时间循环reactor反应堆
            const InetAddress& listenAddr,  //ip+port
            const string& nameArg)          //服务器的名字
            :_server(loop, listenAddr, nameArg), _loop(loop)
        {
            //给服务器注册用户连接的创建和断开回调      回调：可以确定函数要做什么，但是运行时才能确定函数调用的时机，这种情况需要回调
            _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));//因为为了访问成员变量，所以将回调函数作为成员函数，此时参数会多出一个this指针，需要绑定器
            
            //给服务器注册用户读写事件回调
            _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

            //设置服务器端的线程数量    1个IO线程   3个worker线程
            _server.setThreadNum(4);
        }

        //开启事件循环
        void start(){
            _server.start();
        }
private:
    //专门处理用户连接的创建和断开  epoll listenfd accept
    void onConnection(const TcpConnectionPtr &conn){
        if(conn->connected()){
            cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state:online" << endl;
        }else{
            cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state:offline" << endl;
            conn->shutdown();   //close(fd)
            // _loop->quit();终止服务
        }
    }
    //专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn,    //连接
                            Buffer *buffer,         //缓冲区
                            Timestamp time)         //接收到数据的事件信息
    {
        string buf = buffer->retrieveAllAsString();
        cout << "recv data:" << buf << "time:" << time.toString() << endl;//此处只是简单发回收到的消息
        conn->send(buf);
    }
    muduo::net::TcpServer _server;  //#1
    muduo::net::EventLoop *_loop;   //#2 epoll
};

int main(){
    EventLoop loop; //epoll
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop, addr, "Echo");

    server.start(); //listenfd  epoll_ctl
    loop.loop();    //epoll_wait    以阻塞方式等待新用户连接，已连接用户的读写事件

    return 0;
}