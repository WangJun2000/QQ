#include "widget.h"
#include"dialoglist.h"
#include"loginwidget.h"
#include <QApplication>
#include<QtGlobal>
#include<QDebug>
#include<QTime>
#include<QObject>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QUdpSocket *udpClient=new QUdpSocket(0);
    //随机端口
    udpClient->bind(QHostAddress::AnyIPv4);
    quint16 port;
    port=udpClient->localPort();
    qDebug()<<port;
    //用于网络测试
    //udpClient->bind(QHostAddress::LocalHost,port);
    //QString serverIP="106.13.121.233";
    QString serverIP="192.168.202.132";
    quint16 serverPort=8888;
    /*qDebug()<<QObject::connect(udpClient,&QUdpSocket::readyRead,[=](){
        qDebug()<<"主函数收到回复";
    });*/


    LogInWidget w(0,udpClient,serverIP,serverPort);
    w.show();
    return a.exec();
}
