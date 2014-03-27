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
    connect(ui->bn_LobbySend, SIGNAL(clicked()), this, SLOT(sendLobbyMessage()));
    connect(ui->lineEdit_lobby, SIGNAL(returnPressed()), this, SLOT(sendLobbyMessage()));

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

void MainWindow::sendLobbyMessage()
{
    if (!isConnected) return;
    socket.write(QString("501 %1").arg(ui->lineEdit_lobby->text()).toUtf8());
    ui->lineEdit_lobby->setText("");
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
    } else if (command == 101) // Server fragt nach unserem Namen
    {
        socket.write(QString("210 %1").arg(ui->lineEdit_name->text()).toUtf8());
    } else if (command == 503) // Chatnachricht für uns (Lobby)
    {
        QString absender = input.split(" ")[1].mid(0, input.split(" ")[1].length() - 1);
        QString text = ((QStringList)input.split(" ").mid(2)).join(" ");

        if (!playerColors.contains(absender))
        {
            playerColors.insert(absender, QColor::fromHsvF((float)qrand() / RAND_MAX, 1.0, 0.5));
        }

        ui->textBrowser_lobby->setTextColor(playerColors[absender]);
        ui->textBrowser_lobby->append(QString("<%1> %2").arg(absender).arg(text));
    } else if (command == 507) // Durchgabe eines verbundenen Spielers (nach 505 von uns)
    {
        // TODO Spielerliste
    } else if (command == 603) // PING des servers an uns
    {
        socket.write(QString("604").toUtf8());
    }
}
