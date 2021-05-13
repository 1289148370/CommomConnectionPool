#pragma once
#include<string>
#include<iostream>
#include<queue>
#include<mutex>
#include<atomic>//C++11�ṩԭ����
#include<thread>//C++�ṩ���̣߳�������������ָ�룬����
#include<functional>
#include<condition_variable>
using namespace std;
#include "Connection.h"

//ʵ�����ӳع���ģ��
class ConnectionPool
{
public:
	//��ȡ���ӳض���ʵ��
	static ConnectionPool* getConnectionPool();
	//���ⲿ�ṩ�ӿڣ������ӳ��л�ȡһ�����õĿ�������
	//Connection* getConnection();
	//���ø��ⲿר�ŷ���һ�����ӵ�ָ�룬��Ϊ���껹ҪbackConnection()�黹�����ӳ�
	//ֱ�ӷ���һ������ָ�룬����ָ�����������Զ������������ͷ�Connection
	//������ָ���ض���ɾ����ʽ������ʱ��Ҫ�ͷ����ӣ����Ƿ��ظ����ӳ�
	shared_ptr<Connection> getConnection();
private:
	ConnectionPool(); //����1�����캯��˽�л�

//public://������

	bool loadConfigFile();//�������ļ��м���������

	//�����ڶ������߳��У�ר�Ÿ�������������
	//��ͨ�ĳ�Ա���������ĵ��������ڶ���Ҫ������Ƴ��̺߳��������thisָ������Ҫ�Ķ������
	//���������Ǻ���������<functional>��
	void produceConnectionTask();

	//����һ���µĶ�ʱ�̣߳�ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж�������ӻ���
	void scannerConnectionTask();

	//��ʹ�����ӳ�ʱ������Ϣͨ�����������ļ��У������仯ֱ���޸������ļ�����
	string _ip;//ip��ַ
	unsigned short _port;//�˿ں�
	string _username;//�û���
	string _password;//����
	string _dbname;//���ӵ����ݿ�����
	int _initSize;//���ӳصĳ�ʼ������
	int _maxSize;//���ӳص����������
	int _maxIdleTime;//���ӳص�������ʱ��
	int _connectionTimeOut;//���ӳػ�ȡ���ӵĳ�ʱ��


	queue<Connection*> _connectionQue;//�洢mysql���ӵĶ���
	mutex _queueMutex;//ά�����Ӷ��е��̰߳�ȫ������

	//��¼������������connection���ӵ������������ܳ���maxSize
	//��ͨ������++���������̰߳�ȫ�ģ���atomic_int����CAS��ԭ������++�����ǰ�ȫ��
	//�ڶ���߳��ж���Դ��������ӽ�������++����Ҫ�����̰߳�ȫ
	atomic_int _connectionCnt;

	condition_variable cv;//���������������������������̺߳������̵߳�ͨ��
};