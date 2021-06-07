#include "mytcpsocket.h"

MyTcpSocket::MyTcpSocket()
{
    //服务器端口
    quint16 port=10086;
    payloadSize=64*1024;
    totalBytes=0;
    bytesWritten=0;
    bytesToWrite=0;
    fileName="";
    friendName="";
    r_totalBytes=0;
    r_bytesReceived=0;
    r_msgType=-1;
    r_msgLength=0;
    r_fileName="";
    r_friendName="";
    r_inBlock.resize(0);
    currentDir=QDir::currentPath();
    this->connectToHost(QHostAddress::LocalHost,port);
    connect(this,&MyTcpSocket::connected,[=](){
       qDebug()<<"已连接";
    });
    connect(this,&MyTcpSocket::startTransferSignal,this,&MyTcpSocket::startTransfer);
    connect(this,SIGNAL(bytesWritten(qint64)),this,SLOT(updateClientProgress(qint64)));
    connect(this,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(displayError(QAbstractSocket::SocketError)));
    connect(this,&MyTcpSocket::readyRead,this,&MyTcpSocket::receiveFile);
}

void MyTcpSocket::startTransfer(){
    if (this->state()!=QAbstractSocket::ConnectedState){
        emit(updateClientProgressSignal(QString("未连接"),float(0)));
        emit(isSuccess(false));
    }
    localFile=new QFile(fileName);
    if(!localFile->open(QFile::ReadOnly)){
        qDebug()<<"client：open file error!";
        return;
    }
    totalBytes=localFile->size();
    QDataStream sendOut(&outBlock,QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_7);
    QString currentFileName=fileName.right(fileName.size()-fileName.lastIndexOf('/')-1);
    //文件类型、文件总大小、后面3个字符串的总大小、发送者姓名、接收者姓名、文件名
    sendOut<<qint64(0)<<qint64(0)<<qint64(0)<<usrName<<friendName<<currentFileName;
    totalBytes+=outBlock.size();
    sendOut.device()->seek(0);
    sendOut<<qint64(File)<<totalBytes<<qint64(outBlock.size()-sizeof(qint64)*3);
    bytesToWrite=totalBytes-this->write(outBlock);
    //ui->clientStatusLabel->setText("已连接");
    emit(updateClientProgressSignal(QString("已连接"),float(0)));
    outBlock.resize(0);
}

void MyTcpSocket::sendFile(QString _friendName,QString _fileName){
    if(bytesToWrite<=0){
        qDebug()<<"收到发送文件消息";
        friendName=_friendName;
        fileName=_fileName;
        emit(startTransferSignal());
    }
    else{
        emit(isSuccess(false));
    }
}

void MyTcpSocket::updateClientProgress(qint64 numBytes){
    //判断是否是初始化的时候发的用户信息
    if(fileName==""){
        return;
    }
    bytesWritten+=(int)numBytes;
    if(bytesToWrite>0){
        QThread::msleep(1000);//阻塞延时50ms
        outBlock=localFile->read(qMin(bytesToWrite,payloadSize));
        bytesToWrite-=(int)this->write(outBlock);
        outBlock.resize(0);
    }
    else{
        localFile->close();
    }
    //ui->clientProgressBar->setMaximum(m_totalBytes);
    //ui->clientProgressBar->setValue(m_bytesWritten);
    emit(updateClientProgressSignal(QString("正在传送文件%1 ").arg(fileName.right(fileName.size()-fileName.lastIndexOf('/')-1)),100*float(bytesWritten)/float(totalBytes)));
    if(bytesWritten==totalBytes){
        //ui->clientStatusLabel->setText(QString("传送文件 %1 成功").arg(m_fileName));
        emit(updateClientProgressSignal(QString("传送文件 %1 成功").arg(fileName),float(-1)));
        //发送成功信号
        emit(isSuccess(true));
        //重置变量
        totalBytes=0;
        bytesWritten=0;
        bytesToWrite=0;
        fileName="";
        friendName="";
        localFile->close();
    }
}

void MyTcpSocket::displayError(QAbstractSocket::SocketError){
    qDebug()<<this->errorString();
    //ui->clientProgressBar->reset();
    //ui->clientStatusLabel->setText("客户端就绪");
    //ui->sendButton->setEnabled(true);
    emit(isSuccess(false));
}

void MyTcpSocket::sendName(QString name){
    usrName=name;
    qDebug()<<"发送用户名"<<name;
    qDebug()<<"当前线程是"<<QThread::currentThreadId();
    QDataStream sendOut(&outBlock,QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_7);
    //文件类型、文件总大小、后面3个字符串的总大小、发送者姓名、接收者姓名、文件名
    sendOut<<qint64(0)<<qint64(0)<<qint64(0)<<usrName;
    totalBytes=0;
    totalBytes+=outBlock.size();
    sendOut.device()->seek(0);
    sendOut<<qint64(Name)<<totalBytes<<qint64(outBlock.size()-sizeof(qint64)*3);
    qDebug()<<QString(outBlock);
    write(outBlock);
    //重置变量
    outBlock.resize(0);
    totalBytes=0;
    bytesWritten=0;
    bytesToWrite=0;
    fileName="";
    friendName="";
}


void MyTcpSocket::receiveFile(){
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_5_9);
    //如果收到的数据小于8字节,保存到来的文件头,代表文件类型
    if (r_bytesReceived==0){
        if((this->bytesAvailable()>=sizeof(qint64))&&(r_msgType==-1)){
            // 接收数据类型
            in>>r_msgType;
            r_bytesReceived +=sizeof(qint64);
        }
    }
    if(r_msgType == Name){
        qDebug()<<"接受到错误信息";
        return;
    }
    else if(r_msgType == File){
        if (r_bytesReceived<=sizeof(qint64)*2){
            if((this->bytesAvailable()>=sizeof(qint64)*2)&&(r_msgLength==0)){
                // 接收数据总大小信息和文件名大小信息
                in>>r_totalBytes>>r_msgLength;
                r_bytesReceived +=sizeof(qint64)*2;
            }
            if((this->bytesAvailable()>=r_msgLength)&&(r_msgLength!=0)){
                // 接收字符串，并建立文件
                in>>r_usrName>>r_friendName>>r_fileName;
                qDebug()<<"接收到文件"<<r_fileName;
                r_bytesReceived+=r_msgLength;
                QDir d;
                d.mkdir(currentDir+'/'+r_usrName);
                r_localFile = new QFile(currentDir+'/'+r_usrName+'/'+r_fileName);
                if (!r_localFile->open(QFile::WriteOnly)){
                    qDebug() << "server: open file error!";
                    return;
                }
            }
            else{
                return;
            }
        }
        //如果接收到的数据小于总数据,那么写入文件
        if(r_bytesReceived<r_totalBytes){
            r_bytesReceived+=this->bytesAvailable();
            r_inBlock=this->readAll();
            r_localFile->write(r_inBlock);
            r_inBlock.resize(0);
        }
        //接收文件完成
        if(r_bytesReceived==r_totalBytes){
            r_inBlock.resize(0);
            r_localFile->close();
            qDebug()<<"接收"<<r_fileName<<"完成";
            emit(receiveComplete(r_usrName,currentDir+'/'+r_usrName+'/'+r_fileName));
            r_totalBytes=0;
            r_bytesReceived=0;
            r_msgType=-1;
            r_msgLength=0;
            r_fileName="";
            r_friendName="";
            r_inBlock.resize(0);

        }
    }
}
