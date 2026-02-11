#include "mainwindow.h"
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

    MainWindow window;
    window.show();

    return app.exec();
}
