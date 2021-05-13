#include"ConnectionPool.h"
#include<iostream>
#include"public.h"
using namespace std;


//线程安全的懒汉单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool() {
	static ConnectionPool pool; //静态对象的初始化由编译其自动进行lock和unlock,所以线程安全，用到它时才初始化
	return &pool;
}
//从配置文件中加载配置项
bool ConnectionPool::loadConfigFile() {
	FILE *pf = fopen("mysql.ini", "r");
	if (pf == nullptr) {
		LOG("mysql.ini file is not exist!");
		return false;
	}
	while (!feof(pf)) {
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;//字符串中C++提供了多种方法
		int idx = str.find('=', 0);
		if (idx == -1) {//无效的配置项
			continue;
		}
		//ip = 127.0.0.0\n
		int endidx = str.find('\n', idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);


		if (key == "ip") _ip = value;
		else if (key == "port") _port = atoi(value.c_str());
		else if (key == "username") _username = value;
		else if (key == "password")_password = value;
		else if (key == "dbname")_dbname = value;
		else if (key == "initSize") _initSize = atoi(value.c_str());
		else if (key == "maxSize")_maxSize = atoi(value.c_str());
		else if (key == "maxIdleTime")_maxIdleTime = atoi(value.c_str());
		else if (key == "connectionTimeOut")_connectionTimeOut = atoi(value.c_str());
		//cout << endl;
	}
	return true;
}

//连接池的构造
ConnectionPool::ConnectionPool() {
	//加载配置项了
	if (!loadConfigFile()) {
		return;
	}

	//创建初始数量的连接
	for (int i = 0; i < _initSize; i++) {
		Connection *p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();//刷新一下开始空闲的起始时间
		_connectionQue.push(p);
		_connectionCnt++;
	}

	//启动一个新的线程，作为连接的生产者linux thread =>pthread_create
	thread produce(std::bind(&ConnectionPool::produceConnectionTask,this));
	//需要传入一个线程函数，这个线程函数和pthread_create一样需要C接口，不可能写一个连接池里的成员方法
	//如果写一个全局的接口，访问不到连接池中队列
	produce.detach();//设置成守护线程，当前的主线程、进程完了自动结束


	//启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行多余的连接回收
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();
};

//运行在独立的线程中，专门负责生产新连接
void ConnectionPool::produceConnectionTask() {
	for (;;) {
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty()) {
			cv.wait(lock);//队列不空，此处生产者线程进入
		}
		//连接数量没有达到上限，继续创建新的连接
		if (_connectionCnt < _maxSize) {
			Connection *p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime();//刷新一下开始空闲的起始时间
			_connectionQue.push(p);
			_connectionCnt++;
		}
		//通知消费者进程可以消费连接了
		cv.notify_all();
	}
}


//给外部提供接口，从连接池中获取一个可用的空闲连接，智能指针默认delete
shared_ptr<Connection> ConnectionPool::getConnection() {
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty()) {
		//如果是超时醒来
		if(cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeOut))) {
			//上面要么是生产者进程唤醒消费者进程，要么就是自己等待了最大时间自己醒
			if (_connectionQue.empty()) {
				LOG("获取空闲连接超时了...获取连接失败！");
				return nullptr;
			}
		}
		
	}
	/*
	shared_ptr智能指针析构时，会把connection资源直接delete掉，相当于调用connection的析构函数，
	connection资源就被close掉了，这里需要自定义shared_ptr的释放资源的方式，把connection直接归还到queue中
	自定义删除器*/
	shared_ptr<Connection> sp(_connectionQue.front(),
		[&](Connection *pcon) {
		//这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
		unique_lock<mutex> lock(_queueMutex);
		pcon->refreshAliveTime();//刷新一下开始空闲的起始时间
		_connectionQue.push(pcon);
	});
	_connectionQue.pop();
	//if (_connectionQue.empty()) {
	//	cv.notify_all();//谁消费了队列中的最后一个connection,谁负责通知一下生产者生产连接
	//}
	cv.notify_all();//不要上面的if也可以，消费完连接以后，通知生产者线程检查一下，如果队列为空，赶紧生产
	return sp;
}

//扫描超过maxIdleTime时间的空闲连接，进行多余的连接回收
void ConnectionPool::scannerConnectionTask() {
	for (;;) {
		//通过sleep模拟定时效果
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));
		//扫描整个队列，释放多余队列
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize) {
			//获取队头的connection，队头的先入队，队头不超时，后面的也不超时
			Connection *p = _connectionQue.front();
			if (p->getAliveeTime() >= (_maxIdleTime * 1000)) {
				_connectionQue.pop();
				_connectionCnt--;
				delete p;//调用~connection()，释放连接
			}
			else {
				break;//队头元素没有超过最大时间，其余也没有
			}
		}
	}
}