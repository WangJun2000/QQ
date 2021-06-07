#ifndef MYUDPSERVER_H
#define MYUDPSERVER_H

#include<QUdpSocket>
#include<QDataStream>
#include<QDateTime>
#include<QHostAddress>
#include<QObject>
#include<QMap>
#include<QSqlDatabase>
#include<QtSql>
#include <QSqlRecord>//数据库记录相关
#include <QTextCodec>

class MyUdpServer :public QUdpSocket
{
    Q_OBJECT;
    //普通消息，用户登录，用户离开
    enum MsgType{Msg,UsrLogIn,UsrLeft};
public:
    explicit MyUdpServer();

signals:

public slots:

public:
    QSqlDatabase db;
    void sendMsg(QString,quint16,QByteArray);
    QStringList getFriendList(QString);
    void receiveMsg();

private:
    //记录用户名
    QStringList nameList;
    //记录用户密码
    QStringList pwdList;
    //IP
    QString IP;
    //端口号
    quint16 port;

};

#endif // MYUDPSERVER_H
