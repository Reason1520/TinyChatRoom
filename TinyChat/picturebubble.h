#ifndef PICTUREBUBBLE_H
#define PICTUREBUBBLE_H

#include "bubbleframe.h"
#include <QHBoxLayout>
#include <QPixmap>
//#include "clickablelabel.h"
#include <QProgressBar>
#include "global.h"

/******************************************************************************
 * @file       picturebubble.h
 * @brief      聊天消息图片气泡类
 *
 * @author     lueying
 * @date       2026/1/25
 * @history
 *****************************************************************************/

class PictureBubble : public BubbleFrame
{
    Q_OBJECT
public:


    PictureBubble(const QPixmap& picture, ChatRole role, int total, QWidget* parent = nullptr);

    //void setProgress(int value, int total_value);
    //void showProgress(bool show);
    //void setState(TransferState state);
    //void resumeState();
    //void setMsgInfo(std::shared_ptr<MsgInfo> msg);
    TransferState state() const { return m_state; }
    //void setDownloadFinish(std::shared_ptr<MsgInfo> msg, QString file_path);

signals:
    void pauseRequested(QString unique_name, TransferType transfer_type);   // 请求暂停
    void resumeRequested(QString unique_name, TransferType transfer_type);  // 请求继续
    void cancelRequested(QString unique_name, TransferType transfer_type);  // 请求取消

private slots:
    //void onPictureClicked();

private:
    //void updateIconOverlay();
    //void adjustSize();

private:
    // ClickableLabel* m_picLabel;
    QProgressBar* m_progressBar;
    TransferState m_state;

    QIcon m_pauseIcon;
    QIcon m_playIcon;
    QIcon m_downloadIcon;
    QSize m_pixmapSize;
    QVBoxLayout* m_vLayout;
    int m_total_size;
    std::shared_ptr<MsgInfo> _msg_info;
};

#endif // PICTUREBUBBLE_H
