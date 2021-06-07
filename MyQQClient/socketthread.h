ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#include<QThread>
#include<QTcpServer>
#include<QTcpSocket>
#include"widget.h"
#include<QTimer>
#include<QLabel>
#include<QProgressBar>

class SocketThread : public QThread
{
    Q_OBJECT;

public:
    SocketThread(Widget *w);
    Widget *widget;


protected:
    virtual void run();


signals:
    void sendIPandPort( QString _IP, quint16 _port);

public:
    //发送IP和port
    void signalsSend(){
        emit sendIPandPort(IP,port);
    }

private:
    QTcpServer* client_tcpServer;
    QTcpSocket* client_tcpSocket;
    bool Client_isClient;
    bool Client_isServer;
    bool canRun;
    QString IP;
    quint16 port;
    QString fullFileName;
    QTimer timer;
private:
    void getInfo(QString _IP,quint16 _port,QString _fullFileName);
    void setClientAsClient();
    void setClientAsServer();
};

#endif // SOCKETTHREAD_H
