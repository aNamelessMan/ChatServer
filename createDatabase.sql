-- source /home/XX/github/ChatServer/createDatabase.sql
-- 注意创建表的语句中最后一行不能有逗号
-- 注意区分中文符号
-- 事务只和DML语句(增删查改)有关，而创建表等属于DDL语句

create database chat;
use chat;

create table User(
    id INTEGER auto_increment comment '用户id',
    name varchar(50) not null unique comment '用户名',
    password varchar(50) not null comment '用户密码',
    state enum('online', 'offline') default 'offline' comment '当前登录状态',
    primary key(id)
);

create table Friend(
    userid INTEGER not null comment '用户id',
    friendid INTEGER not null comment '好友id',
    primary key(userid, friendid)
);

create table AllGroup(
    id INTEGER auto_increment comment '组id',
    groupname varchar(50) not null unique comment '组名称',
    groupdesc varchar(50) default '' comment '组功能描述',
    primary key(id)
);

create table GroupUser(
    groupid INTEGER not null comment '组id',
    userid INTEGER not null comment '组员id',
    groupole enum('creator', 'normal') default 'normal' comment '组内角色',
    primary key(groupid, userid)
);

create table offlineMessage(
    userid INTEGER not null comment '用户id',
    message varchar(500) not null comment '离线消息（存储Json字符串）'
);