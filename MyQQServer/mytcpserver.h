#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include<QTcpServer>
#include<QByteArray>
#include<QHostAddress>
#include<QHash>

#include"mytcpsocket.h"


class MyTcpServer : public QTcpServer

{
    Q_OBJECT
public:

    explicit MyTcpServer(QObject *parent = 0,int numConnections = 20);
    void setMaxPendingConnections(int);//重写设置最大连接数的函数

//~MyTcpServer();

signals:

    void connectClient(const int , const QString & ,const quint16 );//发送新用户连接信息

    void readData(const int,const QString &, quint16, const QByteArray &);//发送获得用户发过来的数据

    void sockDisConnect(const int ,const QString &,const quint16 );//断开连接的用户信息

    void sentData(const QByteArray &,const int);//向scoket发送消息

public slots:

    void setData(const QByteArray & data, const int  handle);//向用户发送消息

    void setUsrName(int,QString);

    void transpond(const QString,const QByteArray &);//发送获得用户发过来的数据

protected slots:

    void sockDisConnectSlot(int handle,const QString &ip, quint16 prot,QThread *th);//断开连接的用户信息



protected:

    void incomingConnection(qintptr socketDescriptor);//覆盖已获取多线程

private:

    QHash<int,MyTcpSocket *> * tcpClient;
    QHash<int,QString> * tcpSocket_usrName;
    int maxConnections;

};


#endif // MYTCPSERVER_H
