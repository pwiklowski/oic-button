#include "application.h"
#include "OICServer.h"
#include "QQmlContext"
#include "QDebug"
#include <arpa/inet.h>
#include <net/if.h>
#include <poll.h>


extern uint64_t get_current_ms();


Application::Application(int& argc, char *argv[]) : QGuiApplication(argc, argv)
{
    server = new OICServer("Button PC","0685B960-736F-46F7-BEC0-9E6CBD61ADC1", [&](COAPPacket* packet){
        this->send_packet(packet);
    });

    cbor* initial = new cbor(CBOR_TYPE_MAP);

    initial->append("rt", "oic.r.switch.binary");
    initial->append("value", 1);

    OICResource* button = new OICResource("/switch", "oic.r.switch.binary","oic.if.r", [&](cbor* data){
        qDebug() << "Front updated";
    }, initial);

    server->addResource(button);
    server->start();

    engine.rootContext()->setContextProperty("app", this);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    pthread_create(&m_thread, NULL, &Application::run, this);
    pthread_create(&m_discoveryThread, NULL, &Application::runDiscovery, this);

}

String Application::convertAddress(sockaddr_in a){
    char addr[30];
    sprintf(addr, "%d.%d.%d.%d %d",
            (uint8_t) (a.sin_addr.s_addr),
            (uint8_t) (a.sin_addr.s_addr >> 8),
            (uint8_t) (a.sin_addr.s_addr >> 16 ),
            (uint8_t) (a.sin_addr.s_addr >> 24),
            htons(a.sin_port));

    return addr;
}

void* Application::run(void* param){
    Application* a = (Application*) param;
    OICServer* oic_server = a->getServer();

    const int on = 1;
    int fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    a->setSocketFd(fd);

    struct sockaddr_in serv,client;

    serv.sin_family = AF_INET;
    serv.sin_port = 0;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    uint8_t buffer[1024];
    socklen_t l = sizeof(client);
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        qDebug("Unable to set reuse");
        return 0;
    }
    if( bind(fd, (struct sockaddr*)&serv, sizeof(serv) ) == -1)
    {
        qDebug("Unable to bind");
        return 0;
    }
    struct pollfd pfd;
    int res;

    pfd.fd = fd;
    pfd.events = POLLIN;
    size_t rc;
    uint64_t lastTick = get_current_ms();
    while(1){
        rc = poll(&pfd, 1, 200); // 1000 ms timeout
        if(rc > 0)
        {
            rc= recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&l);
            COAPPacket* p = COAPPacket::parse(buffer, rc, a->convertAddress(client));
            oic_server->handleMessage(p);
            delete p;
        }
        if ((get_current_ms() - lastTick) > 1000){
            lastTick = get_current_ms();
            oic_server->checkPackets();
        }
    }
}

void* Application::runDiscovery(void* param){
    Application* a = (Application*) param;
    OICServer* oic_server = a->getServer();

    const int on = 1;

    int fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    struct sockaddr_in serv,client;
    struct ip_mreq mreq;

    serv.sin_family = AF_INET;
    serv.sin_port = htons(5683);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    mreq.imr_multiaddr.s_addr=inet_addr("224.0.1.187");
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if (setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
        return 0;
    }

    uint8_t buffer[1024];
    socklen_t l = sizeof(client);
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        qDebug() << "Unable to set reuse";
        return 0;
    }
    if( bind(fd , (struct sockaddr*)&serv, sizeof(serv) ) == -1)
    {
        qDebug() << "Unable to bind";
        return 0;
    }
    struct pollfd pfd;
    int res;

    pfd.fd = fd;
    pfd.events = POLLIN;
    size_t rc;
    while(1){
        rc = poll(&pfd, 1, 200); // 1000 ms timeout
        if(rc > 0)
        {
            rc= recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&l);
            COAPPacket* p = COAPPacket::parse(buffer, rc, a->convertAddress(client));
            oic_server->handleMessage(p);
        }
    }
}
void Application::send_packet(COAPPacket* packet){
    String destination = packet->getAddress();
    size_t pos = destination.find(" ");
    String ip = destination.substr(0, pos);
    uint16_t port = atoi(destination.substr(pos).c_str());

    struct sockaddr_in client;

    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    client.sin_addr.s_addr = inet_addr(ip.c_str());

    qDebug() << "Send packet mid" << packet->getMessageId() << "dest=" << destination.c_str();
    send_packet(client, packet);
}
void Application::send_packet(sockaddr_in destination, COAPPacket* packet){
    uint8_t buffer[1024];
    size_t response_len;
    socklen_t l = sizeof(destination);
    packet->build(buffer, &response_len);
    qDebug() << "size" << response_len;
    sendto(m_socketFd, buffer, response_len, 0, (struct sockaddr*)&destination, l);
}


void Application::notifyObservers(QString name, quint8 val){
    qDebug() << "notiftyObservers";

    cbor value(CBOR_TYPE_MAP);
    value.append("rt", "oic.r.switch.binary");
    value.append("value", val);

    List<uint8_t> data;

    value.dump(&data);
    server->notify(name.toLatin1().data(), &data);
}

