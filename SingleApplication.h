#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>

class QLocalServer;

class SingleApplication : public QApplication
{
	Q_OBJECT
public:
	SingleApplication(int& argc, char** argv);
	bool instanceRunning() const;					// The instance is already running
	void receiveNewLocalConnection();				// New connection received
signals:
	void newInstanceStartup(QStringList commandLine);	// New instance startup
private:

	void initLocalConnection();		// Initialize the local connection	
	void createLocalServer();		// Creating a Server
	bool isInstanceRunning;			// Whether an instance is already running
	QLocalServer* localServer;		// The local socket Server
	QString serverName;				// The service name
};

#endif // SINGLEAPPLICATION_H