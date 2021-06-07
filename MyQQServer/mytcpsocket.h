#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include<QTcpSocket>
#include<QThread>
#include<QHostAddress>
#include<QDataStream>
#include<QFile>
#include<QDir>

class MyTcpSocket : public QTcpSocket

{
    Q_OBJECT
    enum MsgType{
        Name,
        File
    };
public:
    explicit MyTcpSocket(qintptr socketDescriptor,QObject *parent = 0);
    void resetParameter();

signals:
    void receiveComplete(QString ,const QByteArray &);//发送获得用户发过来的数据

    void sockDisConnect(const int ,const QString &,const quint16 ,QThread *);//断开连接的用户信息

    void setUsrName(int,QString);

public slots:

    void receiveData();//处理接收到的数据

    void sentData(const QByteArray & ,const int);//发送信号的槽

private:

    qintptr socketID;//保存id，== this->socketDescriptor()；但是this->socketDescriptor()客户端断开会被释放，断开信号用this->socketDescriptor()得不到正确值
    qint64 totalBytes;
    qint64 bytesReceived;
    qint64 msgLength;//字符串的长度
    QString usrName;
    QString friendName;
    QString fileName;
    QFile *localFile;
    QByteArray inBlock;
    qint64 msgType;
    QString currentDir;
    QByteArray file;//用来保存文件的完整信息
};


#endif // MYTCPSOCKET_H
