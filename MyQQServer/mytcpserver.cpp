#include "mytcpserver.h"
#include "threadhandle.h"

MyTcpServer::MyTcpServer(QObject *parent ,int numConnections ):
    QTcpServer(parent)
{
    tcpClient=new QHash<int,MyTcpSocket *>;
    tcpSocket_usrName=new QHash<int,QString>;
    setMaxPendingConnections(numConnections);
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    if (tcpClient->size() > maxPendingConnections())//继承重写此函数后，QTcpServer默认的判断最大连接数失效，自己实现
    {
        QTcpSocket tcp;
        tcp.setSocketDescriptor(socketDescriptor);
        tcp.disconnectFromHost();
        return;
    }
    MyTcpSocket * tcpTemp = new MyTcpSocket(socketDescriptor);
    auto th = ThreadHandle::getClass().getThread();
    //可以信号连接信号的，我要捕捉线程ID就独立出来函数了，使用中还是直接连接信号效率应该有优势
    connect(tcpTemp,&MyTcpSocket::receiveComplete,this,&MyTcpServer::transpond);//接受到数据
    connect(tcpTemp,&MyTcpSocket::sockDisConnect,this,&MyTcpServer::sockDisConnectSlot);//断开连接的处理，从列表移除，并释放断开的Tcpsocket
    connect(this,&MyTcpServer::sentData,tcpTemp,&MyTcpSocket::sentData);//发送数据
    //当用户连接后发来第一条消息是自己的名字,将这个名字与tcp描述符对应起来
    connect(tcpTemp,&MyTcpSocket::setUsrName,this,&MyTcpServer::setUsrName);
    tcpTemp->moveToThread(th);//把tcp类移动到新的线程
    th->start();//线程开始运行

    tcpClient->insert(socketDescriptor,tcpTemp);//插入到连接信息中
    //qDebug() <<"incomingConnection THREAD IS：" <<QThread::currentThreadId();
    //发送连接信号
    emit connectClient(tcpTemp->socketDescriptor(),tcpTemp->peerAddress().toString(),tcpTemp->peerPort());

}

void MyTcpServer::setMaxPendingConnections(int numConnections){
    this->QTcpServer::setMaxPendingConnections(numConnections);//调用Qtcpsocket函数，设置最大连接数，主要是使maxPendingConnections()依然有效
    this->maxConnections = numConnections;
}

void MyTcpServer::setData(const QByteArray &data, const int handle)
{
    emit sentData(data,handle);
}



void MyTcpServer::sockDisConnectSlot(int handle,const QString &ip, quint16 prot,QThread *th)
{
    qDebug() <<"MyTcpServer::sockDisConnectSlot thread is:"<< QThread::currentThreadId();

    tcpClient->remove(handle);//连接管理中移除断开连接的socket
    tcpSocket_usrName->remove(handle);
    ThreadHandle::getClass().removeThread(th); //告诉线程管理类那个线程里的连接断开了
    emit sockDisConnect(handle,ip,prot);
}

void MyTcpServer::transpond(const QString friendName,const QByteArray& data){
    int socketDes=tcpSocket_usrName->key(friendName);
    emit(sentData(data,socketDes));

}

void MyTcpServer::setUsrName(int socketDescriptor,QString usrName){
    tcpSocket_usrName->insert(socketDescriptor,usrName);
}


