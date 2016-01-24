#include "application.h"
#include <QQmlApplicationEngine>
#include "OICServer.h"

Application::Application(int argc, char *argv[]) : QGuiApplication(argc, argv)
{
    OICServer* server = new OICServer();

    OICResource* front = new OICResource("/lampa/front", "oic.r.light.dimming","oic.if.rw", [&](cbor* data){


    });



    server->addResource(front);

    server->addResource(new OICResource("/lampa/back", "oic.r.light.dimming","oic.if.rw", [&](cbor* data){


    }));
    server->addResource(new OICResource("/lampa/table", "oic.r.light.dimming","oic.if.rw", [&](cbor* data){


    }));
    server->addResource(new OICResource("/lampa/ambient", "oic.r.colour.rgb","oic.if.rw", [&](cbor* data){


    }));
    server->start("fe80::225:22ff:fe9f:7040", "eth2");

    cbor* v = cbor::map();
    v->append(new cbor("rt"), new cbor("oic.r.light.dimming"));
    v->append(new cbor("dimmingSetting"), new cbor(7));
    v->append(new cbor("range"), new cbor("0,10000"));


    front->update(v);

    front->update(v);
    front->update(v);
    front->update(v);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

}

