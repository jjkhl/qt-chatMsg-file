#include "serverwidget.h"
#include "ui_serverwidget.h"
#include<QFileDialog>
#include<QDebug>
#include<QFileInfo>
#include<QDir>

serverWidget::serverWidget(QString _sender,QString _receiver,QWidget *parent)
    : QWidget(parent),ui(new Ui::serverWidget),
      sender(_sender),receiver(_receiver)

{
    ui->setupUi(this);
    tcpServer = new QTcpServer(this);
    tcpServer->listen(QHostAddress::Any,8000);
    ui->buttonFile->setEnabled(false);
    ui->buttonSend->setEnabled(false);

//    setWindowTitle("服务器端口为：8000");
    setWindowTitle("文件发送");

    connect(tcpServer,&QTcpServer::newConnection,
    [=]
    {
       tcpSocket = tcpServer->nextPendingConnection();
       QString ip = tcpSocket->peerAddress().toString();
       quint16 port = tcpSocket->peerPort();
//       QString str = QString("[%1:%2]成功连接").arg(ip).arg(port);
       QString str = QString("成功连接");
       ui->textEdit->setText(str);
       ui->buttonFile->setEnabled(true);
       ui->buttonSend->setEnabled(true);
        ui->lineEdit_sender->setText(sender);
        ui->lineEdit_receiver->setText(receiver);

    });
    connect(&timer,&QTimer::timeout,
            [=]
    {
       timer.stop();

       sendData();
    });
    QDir curDir;
    if(!curDir.exists(sender))
    {
        curDir.mkdir(sender);
    }
    if(!curDir.exists(receiver))
    {
        curDir.mkdir(receiver);
    }
}

serverWidget::~serverWidget()
{
    delete ui;
}


void serverWidget::on_buttonFile_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,"open","../");
    if(false == filePath.isEmpty()){
        fileName.clear();
        fileSize = 0;

        QFileInfo info(filePath);
        fileName = info.fileName();
        fileSize = info.size();
        sendSize = 0;

        file.setFileName(filePath);

        bool isOk = file.open(QIODevice::ReadOnly);
        if(isOk == false){
            qDebug()<<"只读方式打开文件失败 55";
        }
        ui->textEdit->setText(filePath);
        ui->buttonFile->setEnabled(false);
        ui->buttonSend->setEnabled(true);
    }
    else{
        qDebug() << "选择文件出错 62";
    }
}

void serverWidget::on_buttonSend_clicked()
{
    QString head = QString("%1##%2").arg(fileName).arg(fileSize);
    qint64 len = tcpSocket->write(head.toUtf8());
    if(len>0){
        timer.start(20);
    }
    else{
        qDebug()<<"head send failed";
        file.close();
        ui->buttonFile->setEnabled(true);
        ui->buttonSend->setEnabled(false);
    }

}

void serverWidget::sendData()
{
    qint64 len = 0;
    do
    {
        char buf[4*1024] = {0};
        len = 0;

        len = file.read(buf,sizeof(buf));

        len = tcpSocket->write(buf,len);

        sendSize += len;

    }while(len>0);

    if(sendSize == fileSize){
        ui->textEdit->append("文件发送完毕");
//        file.close();
        ui->buttonFile->setEnabled(true);
        ui->buttonSend->setEnabled(false);
//        tcpSocket->disconnectFromHost();
//        tcpSocket->close();
    }
}
void serverWidget::closeEvent(QCloseEvent *event)
{
    if(sendSize==fileSize)
    {
        file.close();
        tcpSocket->disconnectFromHost();
        tcpSocket->close();
    }
    event->accept();
}
