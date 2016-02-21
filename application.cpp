#include "application.h"
#include "OICServer.h"
#include "QQmlContext"
#include "QDebug"
#include <arpa/inet.h>
#include <net/if.h>





Application::Application(int argc, char *argv[]) : QGuiApplication(argc, argv)
{
    server = new OICServer("Button 1", [&](COAPPacket* packet, COAPResponseHandler handler){
        this->send_packet(packet, handler);
    });

    m_value = cbor::map();
    m_value->append(new cbor("rt"), new cbor("oic.r.switch.binary"));
    m_value->append(new cbor("value"), new cbor(0));

    OICResource* button = new OICResource("/switch", "oic.r.switch.binary","oic.if.r", [&](cbor* data){
        qDebug() << "Front updated";


    }, m_value);

    server->addResource(button);
    server->start();


    pthread_create(&m_thread, NULL, &Application::run, this);
    pthread_create(&m_discoveryThread, NULL, &Application::runDiscovery, server);


    engine.rootContext()->setContextProperty("app", this);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));


}
void* Application::run(void* param){
    Application* a = (Application*) param;

    OICServer* oic_server = a->getServer();
    COAPServer* coap_server = oic_server->getCoapServer();


    const int on = 1;
    int fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    a->setSocketFd(fd);

    struct sockaddr_in serv,client;
    struct ip_mreq mreq;

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

    while(1){
        size_t rc= recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&l);
        if(rc<0)
        {
            std::cout<<"ERROR READING FROM SOCKET";
        }
        COAPPacket* p = new COAPPacket(buffer, rc, oic_server->convertAddress(client).c_str());
        coap_server->handleMessage(p);
    }
}

void* Application::runDiscovery(void* param){
    OICServer* oic_server = (OICServer*) param;
    COAPServer* coap_server = oic_server->getCoapServer();

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


    while(1){
        qDebug() << "Wait for discovery request";
        size_t rc= recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&l);
        qDebug() << "discovery thread received packet";
        if(rc<0)
        {
            std::cout<<"ERROR READING FROM SOCKET";
            continue;
        }
        COAPPacket* p = new COAPPacket(buffer, rc, oic_server->convertAddress(client));

        coap_server->handleMessage(p);
    }
}
void Application::send_packet(COAPPacket* packet, COAPResponseHandler func){
    String destination = packet->getAddress();
    size_t pos = destination.find(" ");
    String ip = destination.substr(0, pos);
    uint16_t port = atoi(destination.substr(pos).c_str());

    struct sockaddr_in client;

    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    client.sin_addr.s_addr = inet_addr(ip.c_str());
    if (!packet->isValidMessageId())
        packet->setMessageId(server->getMessageId());

    qDebug() << "Send packet mid" << packet->getMessageId() << "dest=" << destination.c_str();
    send_packet(client, packet, func);
}
void Application::send_packet(sockaddr_in destination, COAPPacket* packet, COAPResponseHandler func){

    if (func != nullptr)
        server->getCoapServer()->addResponseHandler(packet->getMessageId(), func);

    uint8_t buffer[1024];
    size_t response_len;
    socklen_t l = sizeof(destination);
    packet->build(buffer, &response_len);


    sendto(m_socketFd, buffer, response_len, 0, (struct sockaddr*)&destination, l);
}


void Application::notifyObservers(QString name, quint8 value){
    qDebug() << "notiftyObservers";
    delete m_value;

    m_value = cbor::map();
    m_value->append(new cbor("rt"), new cbor("oic.r.switch.binary"));
    m_value->append(new cbor("value"), new cbor(value));

    List<uint8_t> data;

    m_value->dump(&data);
    server->getCoapServer()->notify(name.toLatin1().data(), data);
}

