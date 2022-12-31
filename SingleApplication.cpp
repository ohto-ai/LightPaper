#include "SingleApplication.h"
#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QLocalServer>
#include <QFileInfo>

SingleApplication::SingleApplication(int& argc, char** argv)
	: QApplication{ argc, argv }
	, isInstanceRunning{ false }
	, localServer{ nullptr }
	, serverName{ applicationFilePath() + "-mutex" }
{
	initLocalConnection();
}

// The instance is already running
bool SingleApplication::instanceRunning() const
{
	return isInstanceRunning;
}

// Socket communication through the implementation of a single
// instance of the program to run, listening to the new connection
// when the function is triggered
void SingleApplication::receiveNewLocalConnection()
{
	QLocalSocket* socket = localServer->nextPendingConnection();
	if (!socket)
		return;
	socket->waitForReadyRead(1000);
	QTextStream stream(socket);
	emit newInstanceStartup(stream.readAll().split('\n'));
	socket->deleteLater();
}

// Through socket communication to realize the single instance of
// the program to run, initialize the local connection, if theconnection
// is not connected to the server, then create, otherwise exit
void SingleApplication::initLocalConnection()
{
	isInstanceRunning = false;
	QLocalSocket socket;
	socket.connectToServer(serverName);
	if (socket.waitForConnected(500))
	{
		isInstanceRunning = true;
		QTextStream stream(&socket);
		stream << arguments().join('\n');
		stream.flush();
		socket.waitForBytesWritten();
		return;
	}

	createLocalServer();
}

// Create LocalServer
void SingleApplication::createLocalServer()
{
	if (localServer != nullptr)
		localServer->deleteLater();
	localServer = new QLocalServer(this);
	connect(localServer, &QLocalServer::newConnection, this, &SingleApplication::receiveNewLocalConnection);
	if (!localServer->listen(serverName) && localServer->serverError() == QAbstractSocket::AddressInUseError)
	{
		QLocalServer::removeServer(serverName);
		localServer->listen(serverName);
	}
}