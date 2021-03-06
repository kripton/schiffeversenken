#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDebug>
#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QString>
#include <QStringList>
#include <QHash>
#include <cstdlib>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Ui::MainWindow *ui;

public slots:
    void doConnect();
    void sockConnected();
    void sockDisconnected();
    void sockError(QAbstractSocket::SocketError);
    void sockRead();
    void sendLobbyMessage();

private:
    QTcpSocket socket;
    void ParseResponse(QString input);
    bool isConnected;
    QHash<QString, QColor> playerColors;
};

#endif // MAINWINDOW_H
