#include "loginwidget.h"
#include "ui_loginwidget.h"
#include"dialoglist.h"
#include<QMessageBox>
LogInWidget::LogInWidget(QWidget *parent,QUdpSocket* udp,QString IP,quint16 port) :
    QWidget(parent),
    ui(new Ui::LogInWidget)
{
    ui->setupUi(this);
    //初始化套接字
    udpClient=udp;
    //初始化服务器的IP和port
    serverIP=IP;
    serverPort=port;
    //设置标题
    setWindowTitle("登录界面");
    //图标
    setWindowIcon(QPixmap(":/images/qq.png"));
    //按下登录按钮
    connect(ui->logInBtn,&QPushButton::clicked,[=](){
        sendMsg(UsrLogIn);
    });
    //接收消息的链接
    //udpClient->waitForReadyRead();
    qDebug()<<connect(udpClient,&QUdpSocket::readyRead,this,&LogInWidget::receiveMsg);

    //取消按钮
    connect(ui->cancelBtn,&QPushButton::clicked,this,&LogInWidget::close);
}

//登录界面发消息
void LogInWidget::sendMsg(MsgType type){
    QByteArray array;

    QDataStream stream(&array,QIODevice::WriteOnly);
    //第一段type第二段发送者用户名第三段接收者用户名第四段信息，内容添加到流中
    stream<<type;
    qDebug()<<type;
    QString MyName=ui->userName->text();
    stream<<MyName;
    qDebug()<<MyName;
    QString str="server";
    QString pwd=ui->userPwd->text();
    switch (type) {
    case(UsrLogIn):
        stream<<str;
        qDebug()<<str;
        stream<<pwd;
        qDebug()<<pwd;
        udpClient->writeDatagram(array,QHostAddress(serverIP),serverPort);
            return;
    case(UsrLeft): break;
    case(Msg):break;
    default: break;
    }
}

//接收消息
void LogInWidget::receiveMsg(){
    QMap<QString,bool> User_Online;
    qDebug()<<"登录界面收到回复";
    //报文长度
    qint64 size=udpClient->pendingDatagramSize();
    QByteArray receiveData=QByteArray(size,0);
    //读取报文
    udpClient->readDatagram(receiveData.data(),size);
    //解析数据
    //第一段数据类型是发送者 第二段是信息
    QDataStream stream(&receiveData,QIODevice::ReadOnly);
    QString sender;
    stream>>sender;
    qDebug()<<sender;
    if(sender=="servermatch"){
        stream>>User_Online;
        qDebug()<<User_Online;
        //打开好友列表,下面两行顺序不能错
        DialogList *list=new DialogList(0,User_Online,ui->userName->text(),udpClient,serverIP,serverPort);
        this->close();
        list->show();
        disconnect(udpClient,&QUdpSocket::readyRead,this,&LogInWidget::receiveMsg);
    }
    else if(sender=="servernotmatch"){
        QMessageBox::warning(this,"警告","密码错误");
    }
}
LogInWidget::~LogInWidget()
{
    delete ui;
}
