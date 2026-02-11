#ifndef CLICKEDBTN_H
#define CLICKEDBTN_H

#include <QPushButton>

/******************************************************************************
 * @file       clickedbtn.h
 * @brief      拥有点击效果的按钮类
 *
 * @author     lueying
 * @date       2026/1/7
 * @history
 *****************************************************************************/

class ClickedBtn :public QPushButton
{
    Q_OBJECT
public:
    ClickedBtn(QWidget* parent = nullptr);
    ~ClickedBtn();
    void setState(QString nomal, QString hover, QString press);
protected:
    virtual void enterEvent(QEnterEvent* event) override;           // 鼠标进入
    virtual void leaveEvent(QEvent* event) override;                // 鼠标离开
    virtual void mousePressEvent(QMouseEvent* event) override;      // 鼠标按下
    virtual void mouseReleaseEvent(QMouseEvent* event) override;    // 鼠标释放
private:
    // 普通、悬停、按下状态下的图片路径
    QString normal_;
    QString hover_;
    QString press_;
};

#endif // CLICKEDBTN_H