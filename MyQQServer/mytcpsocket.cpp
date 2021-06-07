#include "mytcpsocket.h"



MyTcpSocket::MyTcpSocket(qintptr socketDescriptor, QObject *parent) :

    QTcpSocket(parent),socketID(socketDescriptor)

{
    this->setSocketDescriptor(socketDescriptor);
    totalBytes=0;
    bytesReceived=0;
    msgLength=0;
    msgType=-1;
    currentDir=QDir::currentPath();

    //转换系统信号到我们需要发送的数据、直接用lambda表达式了，qdebug是输出线程信息
    connect(this,&MyTcpSocket::readyRead, //转换收到的信息，发送信号
            [this](){
                receiveData();
            });
    connect(this,&MyTcpSocket::disconnected, //断开连接的信号转换
            [&](){
                qDebug() <<"MyTcpSocket::MyTcpSocket lambda sockDisConnect thread is:" << QThread::currentThreadId();
                emit sockDisConnect(socketID,this->peerAddress().toString(),this->peerPort(),QThread::currentThread());//发送断开连接的用户信息
            });

    qDebug() <<"tcp描述符"<< this->socketDescriptor() << " IP地址" << this->peerAddress().toString()
                << " 端口" << this->peerPort() << "myTcpSocket::myTcpSocket thread is " <<QThread::currentThreadId();
}

void MyTcpSocket::sentData(const QByteArray &data, const int id)
{
    //如果是服务器判断好，直接调用write会出现跨线程的现象，所以服务器用广播，每个连接判断是否是自己要发送的信息
    if(id == socketID)//判断是否是此socket的信息
    {
        qDebug() << "MyTcpSocket::sentData" << QThread::currentThreadId();
        write(data);
    }
}

void MyTcpSocket::receiveData(){
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_5_9);
    //如果收到的数据小于8字节,保存到来的文件头,代表文件类型
    if (bytesReceived==0){
        if((this->bytesAvailable()>=sizeof(qint64))&&(msgType==-1)){
            // 接收数据类型
            in>>msgType;
            bytesReceived +=sizeof(qint64);
        }
    }
    if(msgType == Name){
        if(bytesReceived<=sizeof(qint64)*3){
            if((this->bytesAvailable()>=sizeof(qint64)*2)&&(msgLength==0)){
                // 接收数据总大小信息和字符串大小信息
                in>>totalBytes>>msgLength;
                bytesReceived +=sizeof(qint64)*2;
            }
            if((this->bytesAvailable()>=msgLength)&&(msgLength!=0)){
                // 接收用户名
                in>>usrName;
                bytesReceived+=msgLength;
            }
            else{
                return;
            }
            if(bytesReceived==totalBytes){
                //重置totalBytes和bytesReceived和msgType和msgLength
                resetParameter();
                emit setUsrName(socketID,usrName);
                qDebug()<<"接收到用户的名字"<<usrName<<"当前用户线程是"<<QThread::currentThreadId();
            }
        }
    }
    else if(msgType == File){
        if (bytesReceived<=sizeof(qint64)*2){
            if((this->bytesAvailable()>=sizeof(qint64)*2)&&(msgLength==0)){
                // 接收数据总大小信息和文件名大小信息
                in>>totalBytes>>msgLength;
                bytesReceived +=sizeof(qint64)*2;
            }
            if((this->bytesAvailable()>=msgLength)&&(msgLength!=0)){
                // 接收字符串，并建立文件
                in>>usrName>>friendName>>fileName;
                qDebug()<<"接收到文件"<<fileName;
                QDataStream sendOut(&file,QIODevice::WriteOnly);
                sendOut.setVersion(QDataStream::Qt_5_9);
                //文件类型、文件总大小、后面3个字符串的总大小、发送者姓名、接收者姓名、文件名
                sendOut<<qint64(File)<<totalBytes<<msgLength<<usrName<<friendName<<fileName;
                bytesReceived+=msgLength;
                QDir d;
                d.mkdir(currentDir+'/'+usrName);
                localFile = new QFile(currentDir+'/'+usrName+'/'+fileName);
                if (!localFile->open(QFile::WriteOnly)){
                    qDebug() << "server: open file error!";
                    return;
                }
            }
            else{
                return;
            }
        }
        //如果接收到的数据小于总数据,那么写入文件
        if(bytesReceived<totalBytes){
            bytesReceived+=this->bytesAvailable();
            inBlock=this->readAll();
            localFile->write(inBlock);
            file+=inBlock;
            inBlock.resize(0);
        }
        //接收文件完成
        if(bytesReceived==totalBytes){
            inBlock.resize(0);
            localFile->close();
            qDebug()<<"接收"<<fileName<<"完成";
            emit(receiveComplete(friendName,file));
            file.resize(0);
            resetParameter();
        }
    }

}

void MyTcpSocket::resetParameter(){
    totalBytes=0;
    bytesReceived=0;
    msgType=-1;
    msgLength=0;
    fileName="";
    friendName="";
    inBlock.resize(0);
}

