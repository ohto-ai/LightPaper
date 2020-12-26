#include "SingleApplication.h"
#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QLocalServer>
#include <QFileInfo>

SingleApplication::SingleApplication(int& argc, char** argv)
	: QApplication{ argc, argv }
	, isInstanceRunning{ false }
	, localServer{ nullptr }
	, serverName{ QFileInfo(applicationFilePath()).fileName() }
{
	initLocalConnection();
}

// 实例已在运行
bool SingleApplication::instanceRunning() const
{
	return isInstanceRunning;
}

// 通过socket通讯实现程序单实例运行，监听到新的连接时触发该函数
void SingleApplication::receiveNewLocalConnection()
{
	QLocalSocket* socket = localServer->nextPendingConnection();
	if (!socket)
		return;
	socket->waitForReadyRead(1000);
	QTextStream stream(socket);
	emit newInstanceStartup(stream.readAll());
	socket->deleteLater();
}

// 通过socket通讯实现程序单实例运行，初始化本地连接，如果连接不上server，则创建，否则退出
void SingleApplication::initLocalConnection()
{
	isInstanceRunning = false;
	QLocalSocket socket;
	socket.connectToServer(serverName);
	if (socket.waitForConnected(500))
	{
		isInstanceRunning = true;
		// 将启动参数发送到服务端
		QTextStream stream(&socket);
		stream << arguments().join(' ');
		stream.flush();
		socket.waitForBytesWritten();
		return;
	}
	// 连接不上服务器，就创建一个
	createLocalServer();
}

// 创建LocalServer
void SingleApplication::createLocalServer()
{
	localServer->deleteLater();
	localServer = new QLocalServer(this);
	connect(localServer, &QLocalServer::newConnection, this, &SingleApplication::receiveNewLocalConnection);
	if (!localServer->listen(serverName) && localServer->serverError() == QAbstractSocket::AddressInUseError)
	{
		QLocalServer::removeServer(serverName);
		localServer->listen(serverName);
	}
}