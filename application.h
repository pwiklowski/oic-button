#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include "QGuiApplication"
#include "OICServer.h"
#include <QQmlApplicationEngine>
#include <arpa/inet.h>
#include <net/if.h>

class Application : public QGuiApplication
{
    Q_OBJECT
public:
    explicit Application(int& argc, char *argv[]);

    OICServer* getServer(){return server;}
    void setSocketFd(int s) { m_socketFd = s;}
signals:

public slots:
    void notifyObservers(QString name, quint8 value);
    static void* runDiscovery(void* param);
    static void* run(void* param);


private:
    String convertAddress(sockaddr_in a);
    void send_packet(sockaddr_in destination, COAPPacket* packet);
    void send_packet(COAPPacket* packet);

    int m_socketFd;
    cbor* m_value;
    OICServer* server;
    QQmlApplicationEngine engine;
    pthread_t m_thread;
    pthread_t m_discoveryThread;

    quint16 m_front;
};

#endif // APPLICATION_H
