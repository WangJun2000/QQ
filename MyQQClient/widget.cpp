//聊天窗口
#include "widget.h"
#include "ui_widget.h"
#include<QToolButton>
#include<QDataStream>
#include<QMessageBox>
#include<QDateTime>
#include<QFileDialog>
#include<QProgressBar>

Widget::Widget(QWidget *parent,QString localName1,QString friendName1,QString serverIP1,quint16 serverPort1,QUdpSocket* udp,QStringList *chat,MyTcpSocket *tcpClient1)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    tcpClient=tcpClient1;
    ui->setupUi(this);
    ui->msgTextEdit->setFocus();
    //导入聊天
    chatHistory=chat;
    //传递套接字
    udpClient=udp;
    //本机用户名
    localName=localName1;
    //好友名
    friendName=friendName1;
    //服务器IP和端口号
    serverIP=serverIP1;
    serverPort=serverPort1;
    //把发送文件传到多线程
    connect(this,SIGNAL(sendFile(QString,QString)),tcpClient,SLOT(sendFile(QString,QString)));
    //把tcpClient的upadate信号和bar,label的更新联系起来
    connect(tcpClient,SIGNAL(updateClientProgressSignal(QString,float)),this,SLOT(updateBar(QString,float)));
    //成功发送与否
    connect(tcpClient,SIGNAL(isSuccess(bool)),this,SLOT(successDeal(bool)));
    //点击按钮发送消息
    connect(ui->sendBtn,&QPushButton::clicked,[=](){
        if(ui->msgTextEdit->toPlainText()!=""){
            sendMsg(Msg);
        }
        else if(ui->fileNameLabel->text()!="想要发送的文件"){
            sendMsg(File);
        }
        else{
            QMessageBox::warning(this,"警告","发送消息不能为空");
        }
    });
    connect(ui->cacelFileBtn,&QPushButton::clicked,[=](){
       ui->fileNameLabel->setText("想要发送的文件");
    });
    //ui->fileList->setStyleSheet("background-color: white;");
    //ui->fileNameLabel->setStyleSheet("background-color: white;");
    //点击退出按钮关闭聊天窗口
    connect(ui->exitBtn,&QPushButton::clicked,[=](){
        this->close();
    });
    //导入聊天记录
    QList<QString>::Iterator it = chatHistory->begin(),itend = chatHistory->end();
    for(;it!=itend;it++)
    {
        ui->msgBrowser->setTextColor(Qt::blue);
        ui->msgBrowser->append("["+friendName+"]"+*it);
        it++;
        ui->msgBrowser->setTextColor(Qt::gray);
        ui->msgBrowser->append(*it);
        if(it==itend)break;
    }
    chatHistory->clear();
    //设置打开文件按钮的icon
    ui->openBtn->setIcon(QPixmap(":/images/save.png"));
    //点击打开文件按钮选择文件
    connect(ui->openBtn,&QPushButton::clicked,[=](){
        fileName = QFileDialog::getOpenFileName(this,"选择要发送的文件","",nullptr);
        if(!fileName.isEmpty()){
            qDebug()<<localName<<"想要发送"<<fileName;
            ui->fileNameLabel->setText(fileName);
        }
        else{
            qDebug()<<localName<<"选择了空文件";
        }
    });
}


void Widget::closeEvent(QCloseEvent*c){
    emit  this->closeWidget();
    QWidget::closeEvent(c);
}


//发送短消息使用UDP
void Widget::sendMsg(MsgType type){

    //发送的消息做分段处理 第一段 类型化 第二段 自己用户名 第三段 发送的对象的用户名 第四段 信息

    QByteArray array;

    QDataStream stream(&array,QIODevice::WriteOnly);
    //第一段第二段第三段内容添加到流中
    stream<<type;
    stream<<localName;
    stream<<friendName;
    QString msg;
    QString time=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    if (type==Msg){//发送普通消息
        //第四段数据发送内容
        msg= getMsg();
        stream<<msg;
        //追加聊天记录
        ui->msgBrowser->setTextColor(Qt::blue);
        ui->msgBrowser->append("[我]"+time);
        ui->msgBrowser->setTextColor(Qt::gray);
        ui->msgBrowser->append(msg);
        //发送报文
        udpClient->writeDatagram(array,QHostAddress(serverIP),serverPort);
    }
    else if(type==File){//发送文件
        //重置文件框
        ui->fileNameLabel->setText("想要发送的文件");
        //添加进度条和状态栏
        currentBar=new QProgressBar();
        currentBar->setValue(0);
        ui->fileListLayout->addWidget(currentBar,0,Qt::AlignTop);
        currentLabel=new QLabel();
        currentLabel->setText("客户端就绪");
        ui->fileListLayout->addWidget(currentLabel,0,Qt::AlignTop);
        //使用多线程进行发送
        emit(sendFile(friendName,fileName));
        ui->openBtn->setDisabled(true);
    }
    else return;
}

//接收Udp消息
void Widget::receiveMsg(QString( Msg)){
    //获取当前时间
    QString time=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    //追加聊天记录
    ui->msgBrowser->setTextColor(Qt::blue);
    ui->msgBrowser->append("["+friendName+"]"+time);
    ui->msgBrowser->setTextColor(Qt::gray);
    ui->msgBrowser->append(Msg);
}

//获取聊天信息
QString Widget::getMsg(){
    QString str=ui->msgTextEdit->toPlainText();
    //清空输入框
    ui->msgTextEdit->clear();
    ui->msgTextEdit->setFocus();
    return str;
    }

void Widget::updateBar(QString label,float progress){
    if (label!=""){
        currentLabel->setText(label);
    }
    if(progress!=-1){
        currentBar->setValue(progress);
    }
}

void Widget::successDeal(bool isSuccess){
    //重置进度条和label
    currentBar=nullptr;
    currentLabel=nullptr;
    if (isSuccess){
        ui->openBtn->setEnabled(true);
        qDebug()<<"发送成功";
    }
    else{
        ui->openBtn->setEnabled(true);
        qDebug()<<"发送失败";
    }
}

Widget::~Widget()
{
    delete ui;
    disconnect(this,SIGNAL(sendFile(QString,QString)),tcpClient,SLOT(sendFile(QString,QString)));
    //把tcpClient的upadate信号和bar,label的更新联系起来
    disconnect(tcpClient,SIGNAL(updateClientProgressSignal(QString,float)),this,SLOT(updateBar(QString,float)));
    //成功发送与否
    disconnect(tcpClient,SIGNAL(isSuccess(bool)),this,SLOT(successDeal(bool)));
}

