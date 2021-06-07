#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QUdpSocket>
#include<QThread>
#include<QProgressBar>
#include<QLabel>

#include "mytcpsocket.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

    enum MsgType{Msg,UsrLogIn,UsrLeft,File};

public:
    Widget(QWidget *parent,QString localName1,QString friendName1,QString serverIP1,quint16 serverPort1,QUdpSocket* udp,QStringList *chat,MyTcpSocket *tcpClient1);
    ~Widget();
    //关闭事件
    void closeEvent(QCloseEvent *);

private:
    Ui::Widget *ui;

signals:
    //关闭窗口发送信息
    void closeWidget();
    //发送发送文件消息
    void sendFile(QString friendName,QString fileName);

private slots:
    void updateBar(QString,float);
    //接受到成功与否的处理函数
    void successDeal(bool);


public:
    //广播聊天信息使用UDP
    void sendMsg(MsgType type);
    //获取聊天信息
    QString getMsg();
    void receiveMsg(QString Msg); 
    QString fileName;

private:
    //udp套接字
    QUdpSocket *udpClient;
    //IP
    QString serverIP;
    //端口
    quint16 serverPort;
    //用户名
    QString localName;
    //好友名
    QString friendName;
    //接收Udp消息
    QStringList *chatHistory;
    //自定义Tcp
    MyTcpSocket *tcpClient;
    //当前传输文件的进度条
    QProgressBar *currentBar;
    QLabel *currentLabel;

};
#endif // WIDGET_H
