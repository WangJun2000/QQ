#include "socketthread.h"


SocketThread::SocketThread(Widget *w)
{
    widget=w;
    Client_isClient=false;
    Client_isServer=false;
    canRun=true;
    connect(this,&SocketThread::finished,[=](){
        disconnect(widget,&Widget::closeWidget,0,0);
    });
}
void SocketThread::setClientAsClient(){
    Client_isClient=true;
}

void SocketThread::setClientAsServer(){
    Client_isServer=true;
}

void SocketThread::run(){
    int count=0;
    connect(widget,&Widget::closeWidget,[=](){
        qDebug()<<"操作可行";
    });
    while(1){
        IP="0.0.0.0";
        port=10;
        signalsSend();
        msleep(500);
        qDebug()<<QThread::currentThreadId();
        count++;
        if(count==20) return;
    }
    if(Client_isClient){
       client_tcpSocket=new QTcpSocket();
    }
    if(Client_isServer){
       client_tcpServer=new QTcpServer();
    }
}
