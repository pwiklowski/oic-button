#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include "QGuiApplication"
#include "OICServer.h"
#include <QQmlApplicationEngine>

class Application : public QGuiApplication
{
    Q_OBJECT
public:
    explicit Application(int argc, char *argv[]);

    OICServer* getServer(){return server;}
    void setSocketFd(int s) { m_socketFd = s;}
signals:

public slots:
    void notifyObservers(QString name, quint8 value);
    static void* runDiscovery(void* param);
    static void* run(void* param);


private:
    void send_packet(sockaddr_in destination, COAPPacket* packet, COAPResponseHandler func);
    void send_packet(COAPPacket* packet, COAPResponseHandler func);

    int m_socketFd;
    cbor* m_value;
    OICServer* server;
    QQmlApplicationEngine engine;
    pthread_t m_thread;
    pthread_t m_discoveryThread;

    quint16 m_front;
};

#endif // APPLICATION_H
