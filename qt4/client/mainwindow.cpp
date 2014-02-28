#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->lineEdit_port->setValidator(new QIntValidator(1, 65535, this));
    con = new Connection(this);
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
