#ifndef DIALOGLIST_H
#define DIALOGLIST_H

#include <QWidget>
#include<QMap>
#include<QUdpSocket>
#include<QToolButton>
#include<widget.h>
#include<QThread>
#include"mytcpsocket.h"

namespace Ui {
class DialogList;
}

class DialogList : public QWidget
{
    Q_OBJECT
    //普通消息，用户登录，用户离开
    enum MsgType{Msg,UsrLogIn,UsrLeft};
public:
    explicit DialogList(QWidget *parent,QMap<QString,bool> U_O,QString Name,QUdpSocket *udp,QString IP ,quint16 port);
    ~DialogList();
    void closeEvent(QCloseEvent *);
    QUdpSocket *udpClient;
    QMap<QString,Widget *>User_Widget;

private:
    Ui::DialogList *ui;
    QString usrName;
    QString serverIP;
    quint16 serverPort;
    //记录是否显示聊天窗口
    QMap<QString,bool> User_Online;
    //记录对应按钮
    QMap<QString,QToolButton*>User_Button;
    //记录聊天记录
    QMap<QString,QStringList> User_Chat;
    QMap<QString,bool> isShow;
    //好友列表界面发送离开消息
    void sendMsg(MsgType type);
    void receiveMsg();
    MyTcpSocket *tcpClient;

signals:
    //关闭窗口发送信息
    void closeDialogList();
    //当Tcp连上时,发送自己的用户名
    void sendName(QString _UsrName);

public slots:
    void tcpConnected();
    void receiveComplete(QString ,QString);
};

#endif // DIALOGLIST_H
