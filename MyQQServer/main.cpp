#include <QCoreApplication>

#include"myudpserver.h"
#include"mytcpserver.h"
#include"mytcpsocket.h"

/*//连接数据库
QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

//数据库连接函数
bool connectMySQL(QSqlDatabase database,QString host,int port,QString databaseName,QString userName,QString password){
    database.setHostName(host);
    database.setPort(port);
    database.setDatabaseName(databaseName);
    database.setUserName(userName);
    database.setPassword(password);
    return database.open();
}


//定义套接字
QUdpSocket* udpServer;
//记录用户名
QStringList nameList;
//记录用户密码
QStringList pwdList;

*/
/*//记录密码和用户的对应关系
QMap<QString,QString> User_Pwd;

//记录用户的在线情况
QMap<QString,bool> User_Online;

//记录用户的IP和port
QMap<QString,QString> User_IP;
QMap<QString,quint16> User_Port;
*/
/*
//普通消息，用户登录，用户离开
enum MsgType{Msg,UsrLogIn,UsrLeft};

//发送消息给客户端
void sendMsg(QString IP,quint16 port,QByteArray Msg){
    //udpServer->writeDatagram(Msg,QHostAddress(IP),port);
    udpServer->writeDatagram(Msg,QHostAddress(IP),port);
    QString Msgstring=QString(Msg);
    qDebug()<<"发送成功";
}


QStringList getFriendList(QString Name){
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

//接收消息的操作
void receiveMsg(QUdpSocket* udp){
    //发送者的IP和port
    QHostAddress usrIP;
    quint16 usrPort;
    //报文长度
    qint64 size=udp->pendingDatagramSize();
    QByteArray receiveData=QByteArray(size,0);
    //读取报文
    udp->readDatagram(receiveData.data(),size,&usrIP,&usrPort);
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

*/

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    /*if(!connectMySQL(db,"localhost",3306,"user","wangjun","20001122")){
        qDebug()<<"未连接上数据库"<<endl<<"错误说明:"<<db.lastError();
        return 0;
    }
    else{
        qDebug()<<"数据库连接成功";
    }*/
    /*
    //使用sqlite试试
    db.setDatabaseName("F:\\QTproject\\MyQQServer\\user.db");
    if(!db.open()){
        qDebug()<<"未连接上数据库"<<endl<<"错误说明:"<<db.lastError();
        return 0;
    }
    else{
        qDebug()<<"数据库连接成功";
    }
    */
    /*QSqlQuery query;
    //测试数据库用例
    if(!query.exec(QString("select IP from user_IP where name='%1'").arg("老大"))){
        qDebug()<<"读取用户IP失败"<<db.lastError();
    }
    query.next();
    QString sendIP=query.value(0).toString();
    qDebug()<<sendIP;
    if(!query.exec(QString("select IP from user_IP where name='%1'").arg("老大"))){
        qDebug()<<"查询失败"<<query.lastError();
    }
    else{
        qDebug()<<"查询成功";
        //query.next()指向查找到的第一条记录，然后每次后移一条记录
        while(query.next())
        {
            QString value0 = query.value(0).toString();
            QString value1 = query.value(1).toString();
            qDebug()<<value0<<value1<<"结束";
        }
    }*/


    /*//记录用户名和密码
    nameList<<"老大"<<"老二"<<"老三"<<"老四"<<"老五"<<"老六"<<"老七"<<"老八"<<"老九";
    pwdList<<"1"<<"2"<<"3"<<"4"<<"5"<<"6"<<"7"<<"8"<<"9";
    //创建用户名和密码对应关系
    for(int i=0;i<nameList.size();i++){
        User_Pwd[nameList[i]]=pwdList[i];
    }
    //初始化所有人不在线
    for(int i=0;i<nameList.size();i++){
        User_Online[nameList[i]]=false;
    }
    */
    /*
    //创建服务器Udp套接字
    udpServer=new QUdpSocket(0);
    qint64 port=8888;
    //本地网络测试
    QString IP="192.168.202.132";
    udpServer->bind(QHostAddress(IP),port);
    //服务器上用QHostAddress::AnyIPv4,即0.0.0.0
    //udpServer->bind(QHostAddress::AnyIPv4,port);

    QObject::connect(udpServer,&QUdpSocket::readyRead,[=](){
        qDebug()<<"收到消息";
        receiveMsg(udpServer);
    });
    */
    qDebug()<<"主线程是"<<QThread::currentThreadId();
    MyUdpServer *udpServer=new MyUdpServer();
    MyTcpServer *tcpServer=new MyTcpServer();
    if(!tcpServer->listen(QHostAddress::LocalHost,10086)){
        qDebug()<<"tcp监听失败";
    }
    else{
        qDebug()<<"tcp开始监听";
    }
    return a.exec();
}
