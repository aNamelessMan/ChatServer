#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;

//json序列化示例1
string func1(){
    json js;
    //类似于map
    js["msg_type"] = 2;
    js["from"] = "Peter";
    js["to"] = "Bob";
    js["msg"] = "Hello, what're you doing now?";

    //cout << js << endl;
    string sendbuf = js.dump();//序列化为string后即可通过网络进行传输
    cout << sendbuf.c_str() << endl;
    return sendbuf;
}

//json序列化示例2
string func2(){
    json js;
    // 添加数组
    js["id"] = {1,2,3,4,5};
    // 添加key-value
    js["name"] = "zhang san";
    // 添加json对象
    js["msg"]["zhang san"] = "hello world";
    js["msg"]["liu shuo"] = "hello china";
    // 上面等同于下面这句一次性添加数组对象
    // js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}};
    string sendbuf = js.dump();//序列化为string后即可通过网络进行传输
    cout << sendbuf.c_str() << endl;
    return sendbuf;
}

//json序列化示例3
string func3(){
    json js;
    // 直接序列化一个vector容器
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);
    js["list"] = vec;
    // 直接序列化一个map容器
    map<int, string> m;
    m.insert({1, "黄山"});
    m.insert({2, "华山"});
    m.insert({3, "泰山"});
    js["path"] = m;
    string sendbuf = js.dump();//序列化为string后即可通过网络进行传输
    cout << sendbuf.c_str() << endl;
    return sendbuf;
}
int main(){
    string recvbuf = func1();//序列化
    json jsbuf = json::parse(recvbuf);//反序列化
    cout << jsbuf["msg_type"] << endl;
    cout << jsbuf["from"] << endl;
    cout << jsbuf["to"] << endl;
    cout << jsbuf["msg"] << endl;
    return 0;
}