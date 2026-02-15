#include "mainwindow.h"
#include "tcpmgr.h"
#include "filetcpmgr.h"
#include <QtWidgets/QApplication>
#include <QIcon>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置style
    QFile qss(":/style/style/stylesheet.qss");
    if (qss.open(QFile::ReadOnly))
    {
        qDebug("open success");
        app.setStyleSheet(qss.readAll());
        qss.close();
    }
    else {
        qDebug("Open failed");
    }

    // 获取当前应用程序的路径
    QString app_path = QCoreApplication::applicationDirPath();
    // 拼接文件名
    QString fileName = "config.ini";
    QString config_path = QDir::toNativeSeparators(app_path +
        QDir::separator() + fileName);

    QSettings settings(config_path, QSettings::IniFormat);
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();
    gate_url_prefix = "http://" + gate_host + ":" + gate_port;

    //启动tcp线程
    TcpThread tcpthread;
    //启动资源网络线程
    FileTcpThread file_tcp_thread;
    MainWindow w;
    w.show();

    return app.exec();
}
