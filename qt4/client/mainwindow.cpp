#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->lineEdit_port->setValidator(new QIntValidator(1, 65535, this));
    ui->lineEdit_name->setText(QString("Player%1").arg(qrand()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
