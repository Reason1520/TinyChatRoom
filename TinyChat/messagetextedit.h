#ifndef MESSAGETEXTEDIT_H
#define MESSAGETEXTEDIT_H

#include <QObject>
#include <QTextEdit>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMimeType>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QPainter>
#include <QVector>
#include "global.h"

/******************************************************************************
 * @file       messagetextedit.h
 * @brief      聊天消息输入框类
 *
 * @author     lueying
 * @date       2026/1/25
 * @history
 *****************************************************************************/

class MessageTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit MessageTextEdit(QWidget* parent = nullptr);

    ~MessageTextEdit();

    // 解析混合消息
    QVector<std::shared_ptr<MsgInfo>> getMsgList();

    void insertFileFromUrl(const QStringList& urls);
signals:
    void send();

protected:
    // 图片和文件拖拽支持
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    // 允许快捷键
    void keyPressEvent(QKeyEvent* e);

private:
    // 图片缩放与插入
    void insertImages(const QString& url);
    // 文件插入
    void insertFiles(const QString& url);
    // 判断当前剪贴板或拖拽的数据是否可以插入
    bool canInsertFromMimeData(const QMimeData* source) const;
    // 提取其中的文件路径（URL），并根据文件类型分流给图片处理或文件处理函数
    void insertFromMimeData(const QMimeData* source);

private:
    bool isImage(QString url);//判断文件是否为图片
    // 向 MsgInfo 结构体填充数据并添加到列表
    void insertMsgList(QVector<std::shared_ptr<MsgInfo>>& list, MsgType msgtype,
        QString text_or_url, QPixmap preview_pix,
        QString unique_name, uint64_t total_size, QString md5);

    QStringList getUrl(QString text);
    QPixmap getFileIconPixmap(const QString& url);//获取文件图标及大小信息，并转化成图片
    QString getFileSize(qint64 size);//获取文件大小

private slots:
    void textEditChanged();

private:
    QVector<std::shared_ptr<MsgInfo>> img_or_file_list_;
    QVector<std::shared_ptr<MsgInfo>> total_msg_list_;
};

#endif // MESSAGETEXTEDIT_H