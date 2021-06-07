#include "myudpserver.h"

MyUdpServer::MyUdpServer()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("F:\\QTproject\\MyQQServer\\user.db");
    if(!db.open()){
        qDebug()<<"未连接上数据库"<<endl<<"错误说明:"<<db.lastError();
        return;
    }
    else{
        qDebug()<<"数据库连接成功";
    }
    //本地网络
    IP="192.168.202.132";
    port=8888;
    this->bind(QHostAddress(IP),port);
    //服务器上用QHostAddress::AnyIPv4,即0.0.0.0
    //this->bind(QHostAddress::AnyIPv4,port);
    connect(this,&MyUdpServer::readyRead,[=](){
            qDebug()<<"收到消息";
            receiveMsg();
        });
}

void MyUdpServer::sendMsg(QString _IP, quint16 _port, QByteArray _Msg){
    this->writeDatagram(_Msg,QHostAddress(_IP),_port);
    QString Msgstring=QString(_Msg);
    qDebug()<<"发送成功";
}

QStringList MyUdpServer::getFriendList(QString Name){
    QSqlQuery query;
    QStringList friendList;
    //读取好友列表
    if(!query.exec(QString("select friendName from %1的好友列表").arg(Name))){
        qDebug()<<"读取好友列表失败"<<db.lastError();
    }
    while(query.next())
    {
        QString friendName = query.value(0).toString();
        friendList.push_back(friendName);
    }
    query.clear();
    return friendList;
}

void MyUdpServer::receiveMsg(){
    //发送者的IP和port
    QHostAddress usrIP;
    quint16 usrPort;
    //报文长度
    qint64 size=this->pendingDatagramSize();
    QByteArray receiveData=QByteArray(size,0);
    //读取报文
    this->readDatagram(receiveData.data(),size,&usrIP,&usrPort);
    //存储发送者的IP和port
    QString ip=usrIP.toString();
    quint16 port=usrPort;
    qDebug()<<"发送者IP和端口号"<<ip<<port;
    //定义回送的数据
    QByteArray array;
    QDataStream streamout(&array,QIODevice::WriteOnly);
    //解析数据
    //第一段数据类型
    QDataStream streamin(&receiveData,QIODevice::ReadOnly);
    int msgType;
    streamin>>msgType;
    //第二段发送者的用户名
    //第三段想发给的人的用户名
    QString usrName,sendName;
    streamin>>usrName>>sendName;
    //第四段信息
    QString msg;
    streamin>>msg;
    //回送消息的结构 第一段发送者 第二段 信息
    QString match="servermatch";
    QString notMatch="servernotmatch";
    QString someoneLogIn="someoneLogIn";
    QString someoneLeft="someoneLeft";
    QByteArray array1;
    QDataStream streamout1(&array1,QIODevice::WriteOnly);
    qDebug()<<"接收数据:"<<msgType<<usrName<<sendName<<msg;
    //每次使用query都要初始化
    QSqlQuery query;
    //在switch语句外面定义变量
    QString passWord;
    QStringList friendList;
    QMap<QString,bool> friend_online;
    QString sendIP;
    quint16 sendPort;
    QList<QString>::Iterator it,itend;
    switch (msgType) {
    case Msg:
        //读取对方的在线信息
        if(!query.exec(QString("select isOnline from user_online where name='%1'").arg(sendName))){
            qDebug()<<"读取用户在线信息失败"<<db.lastError();break;
        }
        else{
            //如果对方在线，就转发
            query.next();
            if(query.value(0)==true){
                streamout<<usrName<<msg;
                if(!query.exec(QString("select IP from user_IP where name='%1'").arg(sendName))){
                    qDebug()<<"读取用户IP失败"<<db.lastError();break;
                }
                query.next();
                sendIP=query.value(0).toString();
                if(!query.exec(QString("select port from user_port where name='%1'").arg(sendName))){
                    qDebug()<<"读取用户port失败"<<db.lastError();break;
                }
                query.next();
                sendPort=query.value(0).toUInt();
                qDebug()<<"转发消息到:"<<sendName<<endl<<msg<<endl<<sendIP<<sendPort;
                sendMsg(sendIP,sendPort,array);
            }
            break;
        }


    case UsrLogIn:
        //读取密码
        if(!query.exec(QString("select pwd from user_pwd where name='%1'").arg(usrName))){
            qDebug()<<"读取用户密码失败"<<db.lastError();break;
        }
        else{
            query.next();
            passWord=query.value(0).toString();
            //密码匹配
            if(msg==passWord){
                //更新登录状态
                if(!query.exec(QString("update user_online set isOnline=true where name='%1'").arg(usrName))){
                    qDebug()<<"更新登录状态失败"<<db.lastError();break;
                }
                //更新IP
                if(!query.exec(QString("update user_IP set IP='%1' where name='%2'").arg(ip).arg(usrName))){
                    qDebug()<<"更新IP失败"<<db.lastError();break;
                }
                //更新port
                if(!query.exec(QString("update user_port set port=%1 where name='%2'").arg(port).arg(usrName))){
                    qDebug()<<"更新port失败"<<db.lastError();break;
                }
                //向发送者回送匹配消息和当前在线情况并向好友发送自己的登录信息
                streamout<<match;
                friendList=getFriendList(usrName);
                it = friendList.begin(),itend = friendList.end();
                for (;it != itend; it++){
                    //读取每一个好友的在线情况
                    if(!query.exec(QString("select isOnline from user_online where name='%1'").arg(*it))){
                        qDebug()<<"读取用户在线信息失败"<<db.lastError();break;
                    }
                    query.next();
                    friend_online[*it]=query.value(0).toBool();
                    if(friend_online[*it]){
                        if(!query.exec(QString("select IP from user_IP where name='%1'").arg(*it))){
                            qDebug()<<"读取用户IP失败"<<db.lastError();break;
                        }
                        if(!query.next()) qDebug()<<"读取用户IP没有返回数据"<<db.lastError();
                        else sendIP=query.value(0).toString();
                        if(!query.exec(QString("select port from user_port where name='%1'").arg(*it))){
                            qDebug()<<"读取用户port失败"<<db.lastError();break;
                        }
                        query.next();
                        sendPort=query.value(0).toUInt();
                        //向其它在线用户发送该用户登陆信息
                        streamout1<<someoneLogIn;
                        streamout1<<usrName;
                        qDebug()<<"发送其它用户登录信息:"<<someoneLogIn<<usrName<<endl<<sendIP<<sendPort;
                        sendMsg(sendIP,sendPort,array1);
                    }
                  }
                streamout<<friend_online;
                qDebug()<<"回复匹配信息:"<<match<<endl<<friend_online<<ip<<port;
                sendMsg(ip,port,array);
            }
            //密码不匹配
            else{
                //回送不匹配消息
                streamout<<notMatch;
                qDebug()<<"回复不匹配信息:"<<notMatch<<endl<<ip<<port;
                sendMsg(ip,port,array);
            }
            break;
        }
    case UsrLeft:
        //更新登录状态
        if(!query.exec(QString("update user_online set isOnline=false where name='%1'").arg(usrName))){
            qDebug()<<"更新登录状态失败"<<db.lastError();break;
        }
        friendList=getFriendList(usrName);
        it = friendList.begin(),itend = friendList.end();
        for (;it != itend; it++){
            //读取每一个好友的在线情况
            if(!query.exec(QString("select isOnline from user_online where name='%1'").arg(*it))){
                qDebug()<<"读取用户在线信息失败"<<db.lastError();break;
            }
            query.next();
            friend_online[*it]=query.value(0).toBool();
            if(friend_online[*it]){
                if(!query.exec(QString("select IP from user_IP where name='%1'").arg(*it))){
                    qDebug()<<"读取用户IP失败"<<db.lastError();break;
                }
                query.next();
                sendIP=query.value(0).toString();
                if(!query.exec(QString("select port from user_port where name='%1'").arg(*it))){
                    qDebug()<<"读取用户port失败"<<db.lastError();break;
                }
                query.next();
                sendPort=query.value(0).toUInt();
                //向其它在线用户发送该用户离线信息
                streamout<<someoneLeft;
                streamout<<usrName;
                qDebug()<<"发送其他用户离线信息:"<<someoneLeft<<usrName<<endl<<sendIP<<sendPort;
                sendMsg(sendIP,sendPort,array);
            }
        }
        break;

    default: break;
    }
}
