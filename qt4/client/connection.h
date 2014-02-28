#ifndef CONNECTION_H
#define CONNECTION_H

#include <QDebug>
#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QStringList>

class Connection : public QObject
{
    Q_OBJECT
public:
    explicit Connection(QObject *parent = 0);
    ~Connection();

signals:

public slots:
    void doConnect();
    void sockConnected();
    void sockError();
    void sockRead();

private:
    QTcpSocket socket;
};

#endif // CONNECTION_H
