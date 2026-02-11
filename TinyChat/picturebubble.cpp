#include "picturebubble.h"
#include <QLabel>

/******************************************************************************
 * @file       picturebubble.cpp
 * @brief      聊天消息图片气泡类实现
 *
 * @author     lueying
 * @date       2026/1/25
 * @history
 *****************************************************************************/

#define PIC_MAX_WIDTH 160
#define PIC_MAX_HEIGHT 90

PictureBubble::PictureBubble(const QPixmap& picture, ChatRole role, int total, QWidget* parent)
    :BubbleFrame(role, parent), m_state(TransferState::None), m_total_size(total)
{
    // 加载图标（使用Qt内置图标或自定义图标）
    m_pauseIcon = style()->standardIcon(QStyle::SP_MediaPause);
    m_playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
    m_downloadIcon = style()->standardIcon(QStyle::SP_ArrowDown);

    // 创建容器
    QWidget* container = new QWidget();
    m_vLayout = new QVBoxLayout(container);
    m_vLayout->setContentsMargins(0, 0, 0, 0);
    m_vLayout->setSpacing(5);


    //// 创建可点击的图片标签
    //m_picLabel = new ClickableLabel();
    //m_picLabel->setScaledContents(true);
    QPixmap pix = picture.scaled(QSize(PIC_MAX_WIDTH, PIC_MAX_HEIGHT),
        Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_pixmapSize = pix.size();
    //m_picLabel->setPixmap(pix);
    //m_picLabel->setFixedSize(pix.size());

    //connect(m_picLabel, &ClickableLabel::clicked,
    //    this, &PictureBubble::onPictureClicked);

    // 创建进度条
    m_progressBar = new QProgressBar();
    m_progressBar->setFixedWidth(pix.width());
    m_progressBar->setFixedHeight(10);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    //setState(TransferState::None);

    // 样式美化
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "   border: 1px solid #ccc;"
        "   border-radius: 3px;"
        "   text-align: center;"
        "   background-color: #f0f0f0;"
        "   font-size: 10px;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "       stop:0 #4CAF50, stop:1 #45a049);"
        "   border-radius: 2px;"
        "}"
    );

    //m_vLayout->addWidget(m_picLabel);
    //m_vLayout->addWidget(m_progressBar);
    //this->setWidget(container);
    //adjustSize();
}
