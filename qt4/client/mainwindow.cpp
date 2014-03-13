#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->lineEdit_port->setValidator(new QIntValidator(1, 65535, this));
    QString randName = QString("Player%1").arg(qrand());
    randName.truncate(14);
    ui->lineEdit_name->setText(randName);

    ui->lineEdit_name->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9]{3,14}")));

    connect(ui->bn_Connect, SIGNAL(clicked()), this, SLOT(doConnect()));

    isConnected = false;

    connect(&socket, SIGNAL(readyRead()), this, SLOT(sockRead()));
    connect(&socket, SIGNAL(connected()), this, SLOT(sockConnected()));
    connect(&socket, SIGNAL(disconnected()), this, SLOT(sockDisconnected()));
    connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(sockError(QAbstractSocket::SocketError)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sockConnected()
{
    qDebug() << "Socket connected\n";
    ui->lineEdit_name->setDisabled(true);
    ui->bn_Connect->setText("Trennen");
    isConnected = true;
}

void MainWindow::sockDisconnected()
{
    qDebug() << "Socket disconnected\n";
    ui->lineEdit_name->setEnabled(true);
    ui->bn_Connect->setText("Verbinden");
    isConnected = false;
}

void MainWindow::sockError(QAbstractSocket::SocketError error)
{
    qDebug() << "Socket error: " << error;
    socket.close();
    sockDisconnected();
}

void MainWindow::sockRead()
{
    while (socket.canReadLine())
    {
        QByteArray bytes = socket.readLine(1024);
        QString data = QString::fromUtf8(bytes).trimmed();
        ParseResponse(data);
    }
}

void MainWindow::doConnect()
{
    if (isConnected)
    {
        socket.disconnectFromHost();
    } else {
        socket.connectToHost(ui->lineEdit_host->text(), ui->lineEdit_port->text().toInt());
    }
}

void MainWindow::ParseResponse(QString input)
{
    input = input.split(" -- ")[0]; // kommentare weg
    qDebug() << "W/O comments: " << input;

    int command = input.split(" ")[0].toInt();
    if (command < 100)
    {
        qDebug() << "Command \"" << command << "\" invalid (<100)";
    } else if (command == 101)
    {
        // Name angefragt
        socket.write(QString("210 %1").arg(ui->lineEdit_name->text()).toUtf8());
    }
}
