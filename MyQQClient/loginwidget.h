#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include<QPushButton>
#include<QUdpSocket>

namespace Ui {
class LogInWidget;
}

class LogInWidget : public QWidget
{
    Q_OBJECT
    //普通消息，用户登录，用户离开
    enum MsgType{Msg,UsrLogIn,UsrLeft};
public:
    explicit LogInWidget(QWidget *parent,QUdpSocket *udp,QString IP, quint16 Port);
    ~LogInWidget();


private:
    Ui::LogInWidget *ui;

private:
    QString userName;
    QString userPwd;
    //服务器套接字
    QUdpSocket* udpClient;
    //服务器IP
    QString serverIP;
    //服务器端口
    quint16 serverPort;
    //登陆界面发消息
    void sendMsg(MsgType type);
    void receiveMsg();
};

#endif // LOGINWIDGET_H
