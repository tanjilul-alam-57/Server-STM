#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->connectButton->setEnabled(true);


    connect(&tcpSocket, SIGNAL(connected()), this, SLOT(connectedToServer()));
    connect(&tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
    connect(&tcpSocket, SIGNAL(disconnected()), this, SLOT(connectionClosed()));

    QShortcut *shortcut = new QShortcut(QKeySequence("Return"), this);
    connect(shortcut, SIGNAL(activated()), ui->sendButton, SLOT(click()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_exitButton_clicked()
{
    this->close();
}

void MainWindow::on_connectButton_clicked()
{
    QString hostName = ui->serverIPEdit->text();
    QString port_q = ui->serverPortEdit->text();
    quint16 port = port_q.toInt();


    tcpSocket.connectToHost(hostName,port);

    qDebug() << "Connected";
    ui->msgEdit->setEnabled(true);
    ui->msgEdit->setFocus();
    ui->sendButton->setEnabled(true);
    ui->msgBrowser->clear();
}

void MainWindow::receiveMessage()
{
    QByteArray message = tcpSocket.readAll();    // Read message
    QString name("");
    QString msg("");

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(message, &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains("name"))
            {
                QJsonValue name_value = obj.take("name");
                if(name_value.isString())
                {
                    name = name_value.toString();
                }
            }
            if(obj.contains("msg"))
            {
                QJsonValue msg_value = obj.take("msg");
                if(msg_value.isString())
                {
                    msg = msg_value.toString();
                }
            }
        }
    }

    ui->msgBrowser->setTextColor(QColor("blue"));
    ui->msgBrowser->append(QString("[%1] %2 : ")
                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                           .arg(name));
    ui->msgBrowser->setTextColor(QColor("black"));
    ui->msgBrowser->append(QString(msg));
}

void MainWindow::connectedToServer()
{
    ui->msgEdit->setEnabled(true);
    ui->msgEdit->setFocus();
    ui->sendButton->setEnabled(true);
    ui->msgBrowser->clear();
}

void MainWindow::on_sendButton_clicked()
{
    QString message = ui->msgEdit->text();
    QString username = ui->userNameEdit->text();
    if (!message.isEmpty())
    {
        QJsonObject send_json;
        send_json.insert("name", username);
        send_json.insert("msg", message.toLocal8Bit().constData());
        QJsonDocument document;
        document.setObject(send_json);
        QByteArray byte_array = document.toJson(QJsonDocument::Compact);

        tcpSocket.write(byte_array);
        ui->msgEdit->clear();
        ui->msgEdit->setFocus();
    }
}

void MainWindow::connectionClosed()
{
    qDebug() << "connection close.";
}
