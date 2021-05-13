// CommomConnectionPool.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "connection.h"
using namespace std;
#include"ConnectionPool.h"

int main()
{
	/*Connection conn;
	conn.connect("localhost", 3306, "root", "fangcheng.125", "chat");*/

	/*
	Connection conn;
	char sql[1024] = { 0 };
	sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
		"zhang san", 20, "male");
	bool temp = conn.connect("172.0.0.1", 3306, "root", "fangcheng.125", "chat");
	cout << temp << endl;
 	conn.update(sql);
	*/
	//ConnectionPool *cp = ConnectionPool::getConnectionPool();
	//cp->loadConfigFile();
	clock_t begin = clock();
	
	//thread t1([]() {
	//	for (int i = 0; i < 1000; ++i) {
	//		//Connection conn;
	//		//char sql[1024] = { 0 };
	//		//sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
	//		//	"zhang san", 20, "male");
	//		////bool temp = 
	//		//conn.connect("localhost", 3306, "root", "fangcheng.125", "chat");
	//		////cout << temp << endl;
	//		//conn.update(sql);

	//		//用连接池
	//		ConnectionPool *cp = ConnectionPool::getConnectionPool();
	//		shared_ptr<Connection> sp = cp->getConnection();
	//		char sql[1024] = { 0 };
	//		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
	//			"zhang san", 20, "male");
	//		sp->update(sql);
	//	}
	//});
	//thread t2([]() {
	//	for (int i = 0; i < 1000; ++i) {
	//		//Connection conn;
	//		//char sql[1024] = { 0 };
	//		//sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
	//		//	"zhang san", 20, "male");
	//		////bool temp = 
	//		//conn.connect("localhost", 3306, "root", "fangcheng.125", "chat");
	//		////cout << temp << endl;
	//		//conn.update(sql);

	//		//用连接池
	//		ConnectionPool *cp = ConnectionPool::getConnectionPool();
	//		shared_ptr<Connection> sp = cp->getConnection();
	//		char sql[1024] = { 0 };
	//		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
	//			"zhang san", 20, "male");
	//		sp->update(sql);
	//	}
	//});
	//thread t3([]() {
	//	for (int i = 0; i < 1000; ++i) {
	//		//Connection conn;
	//		//char sql[1024] = { 0 };
	//		//sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
	//		//	"zhang san", 20, "male");
	//		////bool temp = 
	//		//conn.connect("localhost", 3306, "root", "fangcheng.125", "chat");
	//		////cout << temp << endl;
	//		//conn.update(sql);

	//		//用连接池
	//		ConnectionPool *cp = ConnectionPool::getConnectionPool();
	//		shared_ptr<Connection> sp = cp->getConnection();
	//		char sql[1024] = { 0 };
	//		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
	//			"zhang san", 20, "male");
	//		sp->update(sql);
	//	}
	//});
	//thread t4([]() {
	//	for (int i = 0; i < 2500; ++i) {
	//		Connection conn;
	//		char sql[1024] = { 0 };
	//		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
	//			"zhang san", 20, "male");
	//		//bool temp = 
	//		conn.connect("localhost", 3306, "root", "fangcheng.125", "chat");
	//		//cout << temp << endl;
	//		conn.update(sql);

	//		////用连接池
	//		//ConnectionPool *cp = ConnectionPool::getConnectionPool();
	//		//shared_ptr<Connection> sp = cp->getConnection();
	//		//char sql[1024] = { 0 };
	//		//sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
	//		//	"zhang san", 20, "male");
	//		//sp->update(sql);
	//	}
	//});
	//t1.join();
	//t2.join();
	//t3.join();
	////t4.join();
//#if 1
	//ConnectionPool *cp = ConnectionPool::getConnectionPool();
	for (int i = 0; i < 9000; ++i) {
		Connection conn;
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
			"zhang san", 20, "male");
		//bool temp = 
		conn.connect("localhost", 3306, "root", "fangcheng.125", "chat");
		//cout << temp << endl;
		conn.update(sql);

		//用连接池
		
		/*shared_ptr<Connection> sp = cp->getConnection();
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
			"zhang san", 20, "male");
		sp->update(sql);*/
	}
//#endif

	clock_t end = clock();
	std::cout << (end - begin) << "ms" << endl;


	return 0;
}
