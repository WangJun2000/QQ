#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include<QTcpSocket>
#include<QHostAddress>
#include<QDebug>
#include<QString>
#include<QFile>
#include<QDataStream>
#include<QThread>
#include<QDir>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
    enum MsgType{
        Name,
        File
    };
public:
    MyTcpSocket();
    //读取服务器转发的数据
    void receiveFile();

signals:
    void startTransferSignal();
    void updateClientProgressSignal(QString,float);
    void isSuccess(bool);
    void receiveComplete(QString,QString);
public slots:
    //获取发送至哪个好友和文件名
    void sendFile(QString _friendName,QString _fileName);
    //开始发送
    void startTransfer();
    //更新进度条
    void updateClientProgress(qint64);
    void displayError(QAbstractSocket::SocketError);
    void sendName(QString);

private:
    QString usrName;
    QString friendName;
    QFile *localFile;
    qint64 totalBytes;
    qint64 bytesWritten;
    qint64 bytesToWrite;
    qint64 payloadSize;
    QString fileName;
    QByteArray outBlock;
    qint64 r_totalBytes;
    qint64 r_bytesReceived;
    qint64 r_msgLength;//字符串的长度
    QString r_usrName;
    QString r_friendName;
    QString r_fileName;
    QFile *r_localFile;
    QByteArray r_inBlock;
    qint64 r_msgType;
    QString currentDir;
};

#endif // MYTCPSOCKET_H
