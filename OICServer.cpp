#include "OICServer.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "log.h"
#include "CBORWriter.h"
#include <string>

OICServer::OICServer()
{


}
static bool discoveryCallback(COAPPacket* request, COAPPacket* response){
    static int temp =0;
    response->setType(COAP_TYPE_ACK);
    if (request->getHeader()->code == COAP_METHOD_GET){
        response->setResonseCode(COAP_RSPCODE_CONTENT);

        vector<uint8_t> p;
        CBORWriter writer(&p);

        writer.writeArray(1);
        {
            writer.writeMap(2);

            writer.writeString("di");
            writer.writeString("Testowy");

            writer.writeString("links");
                writer.writeArray(4);
                    writer.writeMap(3);
                        writer.writeString("href");
                            writer.writeString("/lampa/front");
                        writer.writeString("rt");
                            writer.writeString("oic.r.light.dimming");
                        writer.writeString("if");
                            writer.writeString("oic.if.a");

                    writer.writeMap(3);
                        writer.writeString("href");
                            writer.writeString("/lampa/back");
                        writer.writeString("rt");
                            writer.writeString("oic.r.light.dimming");
                        writer.writeString("if");
                            writer.writeString("oic.if.a");

                    writer.writeMap(3);
                        writer.writeString("href");
                            writer.writeString("/lampa/table");
                        writer.writeString("rt");
                            writer.writeString("oic.r.light.dimming");
                        writer.writeString("if");
                            writer.writeString("oic.if.a");


                    writer.writeMap(3);
                        writer.writeString("href");
                            writer.writeString("/lampa/ambient");
                        writer.writeString("rt");
                            writer.writeString("oic.r.colour.rgb");
                        writer.writeString("if");
                            writer.writeString("oic.if.a");

                     }


        vector<uint8_t> data;
        data.push_back(((uint16_t)COAP_CONTENTTYPE_CBOR & 0xFF00) >> 8);
        data.push_back(((uint16_t)COAP_CONTENTTYPE_CBOR & 0xFF));

        response->addOption(new COAPOption(COAP_OPTION_CONTENT_FORMAT, data));
        response->addPayload(p);

        return true;
    }
    return false;

}
static bool lightCallback(COAPPacket* request, COAPPacket* response){
    static int temp =0;
    response->setType(COAP_TYPE_ACK);
    if (request->getHeader()->code == COAP_METHOD_GET){
        response->setResonseCode(COAP_RSPCODE_CONTENT);

        string r = to_string(temp++);

        vector<uint8_t> data;
        data.push_back(((uint16_t)COAP_CONTENTTYPE_TEXT_PLAIN & 0xFF00) >> 8);
        data.push_back(((uint16_t)COAP_CONTENTTYPE_TEXT_PLAIN & 0xFF));

        response->addOption(new COAPOption(COAP_OPTION_CONTENT_FORMAT, data));
        response->addPayload(r);
        return true;

    }else if(request->getHeader()->code == COAP_METHOD_POST){
        vector<uint8_t> value = request->getPayload();
        response->setResonseCode(COAP_RSPCODE_CHANGED);
        //temp = atoi(value.c_str());
        return true;

    }
    return false;

}
void OICServer::start(){

    COAPServer coap_server;
    coap_server.addEndpoint("/lampa/light", "</lampa/light>;rt=\"Light\"", lightCallback);
    coap_server.addEndpoint("/oic/res",  "", discoveryCallback);

    pthread_create(&m_thread, NULL, &OICServer::run, &coap_server);
    pthread_join(m_thread, NULL);

}
void* OICServer::run(void* param){
    COAPServer* coap_server = (COAPServer*) param;

    int sockfd;
    sockfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    struct sockaddr_in serv,client;
    struct ip_mreq mreq;

    serv.sin_family = AF_INET;
    serv.sin_port = htons(5683);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    uint8_t buffer[1024];
    socklen_t l = sizeof(client);

    if( bind(sockfd , (struct sockaddr*)&serv, sizeof(serv) ) == -1)
    {
        log("Unable to bind");
        return 0;
    }

    mreq.imr_multiaddr.s_addr=inet_addr("224.0.1.187");
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if (setsockopt(sockfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
        return 0;
    }

    while(1){
        size_t rc= recvfrom(sockfd,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&l);
        if(rc<0)
        {
            std::cout<<"ERROR READING FROM SOCKET";
        }
        COAPPacket* p = new COAPPacket(buffer, rc);
        COAPPacket* response = coap_server->handleMessage(p);
        size_t response_len;
        response->build(buffer, &response_len);
        sendto(sockfd,buffer, response_len, 0, (struct sockaddr*)&client,l);
    }
}

