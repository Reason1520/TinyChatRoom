#pragma once
#include <QLabel>
#include <QWidget>
#include <QIcon>

/******************************************************************************
 * @file       clickedlabel.h
 * @brief      可点击的label类
 *
 * @author     lueying
 * @date       2026/2/17
 * @history
 *****************************************************************************/

class ClickableLabel :
    public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget* parent = nullptr);
    void setIconOverlay(const QIcon& icon); //设置遮罩图标
    void showIconOverlay(bool show);  //显示/隐藏遮罩图标
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

signals:
    void clicked();

private:
    QIcon m_overlayIcon;
    bool m_showOverlay;
    bool m_hovered;
};

