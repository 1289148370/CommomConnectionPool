#pragma once
#include<string>
#include<iostream>
#include<queue>
#include<mutex>
#include<atomic>//C++11提供原子类
#include<thread>//C++提供的线程，还包含了智能指针，绑定器
#include<functional>
#include<condition_variable>
using namespace std;
#include "Connection.h"

//实现连接池功能模块
class ConnectionPool
{
public:
	//获取连接池对象实例
	static ConnectionPool* getConnectionPool();
	//给外部提供接口，从连接池中获取一个可用的空闲连接
	//Connection* getConnection();
	//不用给外部专门返回一个连接的指针，因为用完还要backConnection()归还给连接池
	//直接返回一个智能指针，智能指针出作用域会自动析构，进而释放Connection
	//给智能指针重定义删除方式，析构时不要释放连接，而是返回给连接池
	shared_ptr<Connection> getConnection();
private:
	ConnectionPool(); //单例1：构造函数私有化

//public://测试用

	bool loadConfigFile();//从配置文件中加载配置项

	//运行在独立的线程中，专门负责生产新连接
	//普通的成员方法，它的调用依赖于对象，要把它设计成线程函数必须把this指针所需要的对象绑定上
	//绑定器，都是函数对象，在<functional>中
	void produceConnectionTask();

	//启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行多余的连接回收
	void scannerConnectionTask();

	//在使用连接池时以下信息通常放在配置文件中，环境变化直接修改配置文件即可
	string _ip;//ip地址
	unsigned short _port;//端口号
	string _username;//用户名
	string _password;//密码
	string _dbname;//连接的数据库名称
	int _initSize;//连接池的初始连接量
	int _maxSize;//连接池的最大连接量
	int _maxIdleTime;//连接池的最大空闲时间
	int _connectionTimeOut;//连接池获取连接的超时间


	queue<Connection*> _connectionQue;//存储mysql连接的队列
	mutex _queueMutex;//维护连接队列的线程安全互斥锁

	//记录连接所创建的connection连接的总数量，不能超过maxSize
	//普通的整形++操作不是线程安全的，而atomic_int基于CAS的原子整形++操作是安全的
	//在多个线程中都会对创建的连接进行数量++，就要考虑线程安全
	atomic_int _connectionCnt;

	condition_variable cv;//设置条件变量，用于连接生产线程和消费线程的通信
};