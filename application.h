#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include "QGuiApplication"

class Application : public QGuiApplication
{
    Q_OBJECT
public:
    explicit Application(int argc, char *argv[]);

signals:

public slots:
};

#endif // APPLICATION_H
