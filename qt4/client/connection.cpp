#include "connection.h"

Connection::Connection(QObject *parent) : QObject(parent) {
    connect(&socket, SIGNAL(readyRead()), this, SLOT(sockRead()));
}

Connection::~Connection() {
  socket.close();
}

void Connection::doConnect() {
    //socket.connectToHost(, url.split(':').at(1).toUInt());
}

void Connection::sockConnected() {
    qDebug() << "Socket connected\n";
}

void Connection::sockError() {
    qDebug() << "Socket error\n";
}

void Connection::sockRead() {
    QString data;
    data.fromUtf8(socket.readAll());
    qDebug() << "Socket data:\"" << data;
}
