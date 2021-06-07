//入口界面
#include "dialoglist.h"
#include "ui_dialoglist.h"
#include"widget.h"
#include<QToolButton>
#include<QMessageBox>
#include<QDateTime>

#include"threadhandle.h"

DialogList::DialogList(QWidget *parent,QMap<QString,bool> U_O,QString Name,QUdpSocket*udp,QString IP,quint16 port) :
    QWidget(parent),
    ui(new Ui::DialogList)
{
    ui->setupUi(this);
    //登录的用户
    usrName=Name;
    //同步在线状态
    User_Online=U_O;
    //传入套接字 IP port
    udpClient=udp;
    serverIP=IP;
    serverPort=port;
    //标题
    setWindowTitle("MyQQ 2021");
    //图标
    setWindowIcon(QPixmap(":/images/qq.png"));

    //图片和人物名称
    QStringList imagelist,namelist;
    QMap<QString,bool>::Iterator  it;
    for(it=User_Online.begin();it != User_Online.end();++it){
        namelist.push_back(it.key());
    }
    namelist.sort();
    imagelist=namelist;
    QMap<QString,QString> Name_Image;
    for (int i=0;i<namelist.size();i++){
        Name_Image[namelist[i]]=imagelist[i];
    }
    QToolButton *btn= new QToolButton;
    User_Button[usrName]=btn;
    btn->setText(usrName+"(本人）");
    QString str=QString(":/images/%1.png").arg(usrName);
    btn->setIcon(QPixmap(str));
    //设置头像大小
    btn->setIconSize(QSize(250,100));
    //设置透明风格
    btn->setAutoRaise(true);
    //设置文字和图片同时显示
    btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    //添加到
    ui->vLayout->addWidget(btn);
    for(int i=0;i<namelist.size();i++){
        if(namelist[i]!=usrName){
            //设置头像
            QToolButton *btn= new QToolButton;
            //设置名称
            if(User_Online[namelist[i]]==true){
                btn->setText(namelist[i]+"(在线)");
            }
            else
            {
                btn->setText(namelist[i]+"(离线)");
            }
            QString str=QString(":/images/%1.png").arg(Name_Image[namelist[i]]);
            btn->setIcon(QPixmap(str));
            //设置头像大小
            btn->setIconSize(QSize(250,100));
            //设置透明风格
            btn->setAutoRaise(true);
            //设置文字和图片同时显示
            btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            //添加到
            ui->vLayout->addWidget(btn);
            User_Button[namelist[i]]=btn;
            //isShow初始化
            isShow[namelist[i]]=false;
            }
    }
    //对九个按钮添加信号槽
    for(int i=0;i<namelist.size();i++){
        if(namelist[i]!=usrName){
            connect(User_Button[namelist[i]],&QToolButton::clicked,[=](){
                //如果被打开,不重复打开
                if(User_Online[namelist[i]]==false){
                   QMessageBox::warning(this,"注意","该用户不在线");
                   return;
                }
                if(isShow[namelist[i]]){
                    QString str=QString("%1窗口已经被打开了").arg(User_Button[namelist[i]]->text());
                    QMessageBox::warning(this,"警告",str);

                    return;
                }
                isShow[namelist[i]]=true;
                //重置为在线状态
                User_Button[namelist[i]]->setText(namelist[i]+"(在线)");
                //弹出聊天对话窗口 告诉名字 参数1 以顶层的方式弹出 参数2 名字
                User_Widget[namelist[i]]=new Widget(0,usrName, namelist[i],serverIP,serverPort,udpClient,&User_Chat[namelist[i]],tcpClient);
                //设置窗口标题
                User_Widget[namelist[i]]->setWindowTitle(User_Button[namelist[i]]->text());
                User_Widget[namelist[i]]->setWindowIcon(User_Button[namelist[i]]->icon());
                User_Widget[namelist[i]]->show();
                connect(User_Widget[namelist[i]],&Widget::closeWidget,[=](){
                   isShow[namelist[i]]=false;
                });
            });
        }
    }
    //对TCP进行初始化和多线程操作
    tcpClient=new MyTcpSocket();
    auto th = ThreadHandle::getClass().getThread();

    connect(tcpClient,&MyTcpSocket::connected,this,&DialogList::tcpConnected);
    connect(tcpClient,&MyTcpSocket::receiveComplete,this,&DialogList::receiveComplete);
    connect(this,SIGNAL(sendName(QString)),tcpClient,SLOT(sendName(QString)));
    connect(this,&DialogList::closeDialogList,tcpClient,&MyTcpSocket::close);
    //关闭窗口时线程退出
    connect(this,&DialogList::closeDialogList,th,&QThread::quit);
    //接收消息的信号与槽
    connect(udpClient,&QUdpSocket::readyRead,this,&DialogList::receiveMsg);
    tcpClient->moveToThread(th);
    th->start();
}

void DialogList::closeEvent(QCloseEvent *c){
    emit  this->closeDialogList();
    sendMsg(UsrLeft);
    udpClient->close();
    udpClient->destroyed();
    QWidget::closeEvent(c);
}

void DialogList::sendMsg(MsgType type){
    QByteArray array;

    QDataStream stream(&array,QIODevice::WriteOnly);
    //第一段type第二段发送者用户名第三段接收者用户名第四段信息，内容添加到流中
    stream<<type;
    qDebug()<<type;
    stream<<usrName;
    qDebug()<<usrName;
    QString str="server";
    switch (type) {
    case(UsrLeft):
        stream<<str;
        qDebug()<<str;
        udpClient->writeDatagram(array,QHostAddress(serverIP),serverPort);
            return;
    case(UsrLogIn): break;
    case(Msg):break;
    }
}
//接收消息
void DialogList::receiveMsg(){
    qDebug()<<"好友列表收到回复";
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
    if(sender=="someoneLeft"){
        QString leftUsr;
        stream>>leftUsr;
        qDebug()<<leftUsr;
        User_Online[leftUsr]=false;
        User_Button[leftUsr]->setText(leftUsr+"(离线)");

    }
    else if(sender=="someoneLogIn"){
        QString logUsr;
        stream>>logUsr;
        qDebug()<<logUsr;
        User_Online[logUsr]=true;
        User_Button[logUsr]->setText(logUsr+"(在线)");
    }
    //不是服务器的消息，用户消息
    else{
        QString Msg;
        stream>>Msg;
        qDebug()<<Msg;
        if(isShow[sender]){
            User_Widget[sender]->receiveMsg(Msg);
        }
        else{
            User_Button[sender]->setText(sender+"(在线，有消息到了");
            QString time=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            User_Chat[sender].push_back(time);
            User_Chat[sender].push_back(Msg);
        }
    }
}

void DialogList::tcpConnected(){
    qDebug()<<"TCP已连接";
    qDebug()<<"当前线程是"<<QThread::currentThreadId();
    emit(sendName(usrName));
}

void DialogList::receiveComplete(QString sender,QString filePath){
    if(isShow[sender]){
        QString Msg="收到文件,保存在"+filePath;
        User_Widget[sender]->receiveMsg(Msg);
    }
    else{
        User_Button[sender]->setText(sender+"(在线，有消息到了");
        QString time=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        User_Chat[sender].push_back(time);
        QString Msg="收到文件,保存在"+filePath;
        User_Chat[sender].push_back(Msg);
    }

}


DialogList::~DialogList()
{
    delete ui;
}
