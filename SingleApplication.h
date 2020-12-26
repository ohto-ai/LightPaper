#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>

class QLocalServer;

class SingleApplication : public QApplication
{
	Q_OBJECT
public:
	SingleApplication(int& argc, char** argv);
	bool instanceRunning() const;					// 实例已经运行
	void receiveNewLocalConnection();				// 收到新的连接
signals:
	void newInstanceStartup(QString commandLine);	// 新实例启动
private:
	
	void initLocalConnection();		// 初始化本地连接	
	void createLocalServer();		// 创建服务端
	bool isInstanceRunning;			// 是否已经有实例在运行
	QLocalServer* localServer;		// 本地socket Server
	QString serverName;				// 服务名称
};

#endif // SINGLEAPPLICATION_H