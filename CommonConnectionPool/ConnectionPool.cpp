#include"ConnectionPool.h"
#include<iostream>
#include"public.h"
using namespace std;


//�̰߳�ȫ���������������ӿ�
ConnectionPool* ConnectionPool::getConnectionPool() {
	static ConnectionPool pool; //��̬����ĳ�ʼ���ɱ������Զ�����lock��unlock,�����̰߳�ȫ���õ���ʱ�ų�ʼ��
	return &pool;
}
//�������ļ��м���������
bool ConnectionPool::loadConfigFile() {
	FILE *pf = fopen("mysql.ini", "r");
	if (pf == nullptr) {
		LOG("mysql.ini file is not exist!");
		return false;
	}
	while (!feof(pf)) {
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;//�ַ�����C++�ṩ�˶��ַ���
		int idx = str.find('=', 0);
		if (idx == -1) {//��Ч��������
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

//���ӳصĹ���
ConnectionPool::ConnectionPool() {
	//������������
	if (!loadConfigFile()) {
		return;
	}

	//������ʼ����������
	for (int i = 0; i < _initSize; i++) {
		Connection *p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();//ˢ��һ�¿�ʼ���е���ʼʱ��
		_connectionQue.push(p);
		_connectionCnt++;
	}

	//����һ���µ��̣߳���Ϊ���ӵ�������linux thread =>pthread_create
	thread produce(std::bind(&ConnectionPool::produceConnectionTask,this));
	//��Ҫ����һ���̺߳���������̺߳�����pthread_createһ����ҪC�ӿڣ�������дһ�����ӳ���ĳ�Ա����
	//���дһ��ȫ�ֵĽӿڣ����ʲ������ӳ��ж���
	produce.detach();//���ó��ػ��̣߳���ǰ�����̡߳����������Զ�����


	//����һ���µĶ�ʱ�̣߳�ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж�������ӻ���
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();
};

//�����ڶ������߳��У�ר�Ÿ�������������
void ConnectionPool::produceConnectionTask() {
	for (;;) {
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty()) {
			cv.wait(lock);//���в��գ��˴��������߳̽���
		}
		//��������û�дﵽ���ޣ����������µ�����
		if (_connectionCnt < _maxSize) {
			Connection *p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime();//ˢ��һ�¿�ʼ���е���ʼʱ��
			_connectionQue.push(p);
			_connectionCnt++;
		}
		//֪ͨ�����߽��̿�������������
		cv.notify_all();
	}
}


//���ⲿ�ṩ�ӿڣ������ӳ��л�ȡһ�����õĿ������ӣ�����ָ��Ĭ��delete
shared_ptr<Connection> ConnectionPool::getConnection() {
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty()) {
		//����ǳ�ʱ����
		if(cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeOut))) {
			//����Ҫô�������߽��̻��������߽��̣�Ҫô�����Լ��ȴ������ʱ���Լ���
			if (_connectionQue.empty()) {
				LOG("��ȡ�������ӳ�ʱ��...��ȡ����ʧ�ܣ�");
				return nullptr;
			}
		}
		
	}
	/*
	shared_ptr����ָ������ʱ�����connection��Դֱ��delete�����൱�ڵ���connection������������
	connection��Դ�ͱ�close���ˣ�������Ҫ�Զ���shared_ptr���ͷ���Դ�ķ�ʽ����connectionֱ�ӹ黹��queue��
	�Զ���ɾ����*/
	shared_ptr<Connection> sp(_connectionQue.front(),
		[&](Connection *pcon) {
		//�������ڷ�����Ӧ���߳��е��õģ�����һ��Ҫ���Ƕ��е��̰߳�ȫ����
		unique_lock<mutex> lock(_queueMutex);
		pcon->refreshAliveTime();//ˢ��һ�¿�ʼ���е���ʼʱ��
		_connectionQue.push(pcon);
	});
	_connectionQue.pop();
	//if (_connectionQue.empty()) {
	//	cv.notify_all();//˭�����˶����е����һ��connection,˭����֪ͨһ����������������
	//}
	cv.notify_all();//��Ҫ�����ifҲ���ԣ������������Ժ�֪ͨ�������̼߳��һ�£��������Ϊ�գ��Ͻ�����
	return sp;
}

//ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж�������ӻ���
void ConnectionPool::scannerConnectionTask() {
	for (;;) {
		//ͨ��sleepģ�ⶨʱЧ��
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));
		//ɨ���������У��ͷŶ������
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize) {
			//��ȡ��ͷ��connection����ͷ������ӣ���ͷ����ʱ�������Ҳ����ʱ
			Connection *p = _connectionQue.front();
			if (p->getAliveeTime() >= (_maxIdleTime * 1000)) {
				_connectionQue.pop();
				_connectionCnt--;
				delete p;//����~connection()���ͷ�����
			}
			else {
				break;//��ͷԪ��û�г������ʱ�䣬����Ҳû��
			}
		}
	}
}