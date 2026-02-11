#include "TextBubble.h"
#include <QFontMetricsF>
#include <QDebug>
#include <QFont>
#include "global.h"
#include <QTimer>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextLayout>
#include <QFont>

/******************************************************************************
 * @file       textbubble.cpp
 * @brief      聊天消息文字气泡类实现
 *
 * @author     lueying
 * @date       2026/1/25
 * @history
 *****************************************************************************/

TextBubble::TextBubble(ChatRole role, const QString &text, QWidget *parent)
    :BubbleFrame(role, parent)
{
    pTextEdit_ = new QTextEdit();
    pTextEdit_->setReadOnly(true);
    pTextEdit_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pTextEdit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pTextEdit_->installEventFilter(this);
    QFont font("Microsoft YaHei");
    font.setPointSize(12);
    pTextEdit_->setFont(font);
    setPlainText(text);
    setWidget(pTextEdit_);
    initStyleSheet();
}

bool TextBubble::eventFilter(QObject *o, QEvent *e) {
    if(pTextEdit_ == o && e->type() == QEvent::Paint)
    {
        adjustTextHeight(); //PaintEvent中设置
    }
    return BubbleFrame::eventFilter(o, e);
}

void TextBubble::setPlainText(const QString &text) {
    pTextEdit_->setPlainText(text);
    //m_pTextEdit->setHtml(text);
    //找到段落中最大宽度
    qreal doc_margin = pTextEdit_->document()->documentMargin();
    int margin_left = this->layout()->contentsMargins().left();
    int margin_right = this->layout()->contentsMargins().right();

    QFontMetricsF fm(pTextEdit_->font());
    QTextDocument *doc = pTextEdit_->document();
    qreal max_width = 0;
    //遍历每一段找到 最宽的那一段
    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next())    //字体总长
    {
        qreal txtW = fm.horizontalAdvance(it.text()); // 获取精确宽度
        max_width = qMax(max_width, txtW);               //找到最长的那段
    }
    //设置这个气泡的最大宽度 只需要设置一次
    // 关键点 1：使用 qCeil 向上取整
    // 关键点 2：额外 + 3~5 像素的冗余量，防止边缘计算偏差
    int finalWidth = qCeil(max_width + doc_margin * 2 + margin_left + margin_right + 5);

    // 如果需要限制气泡最大宽度（防止一行太长），可以加入如下逻辑
    // int maxWidthLimit = 500; 
    // finalWidth = qMin(finalWidth, maxWidthLimit);

    // 建议使用 setFixedWidth 锁定宽度，防止被 Grid 布局拉伸
    setFixedWidth(finalWidth);
}

void TextBubble::adjustTextHeight() {
    qreal doc_margin = pTextEdit_->document()->documentMargin();    //字体到边框的距离默认为4
    QTextDocument *doc = pTextEdit_->document();
    qreal text_height = 0;
    //把每一段的高度相加=文本高
    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next())
    {
        QTextLayout *pLayout = it.layout();
        QRectF text_rect = pLayout->boundingRect();                             //这段的rect
        text_height += text_rect.height();
    }
    int vMargin = this->layout()->contentsMargins().top();
    //设置这个气泡需要的高度 文本高+文本边距+TextEdit边框到气泡边框的距离
    setFixedHeight(text_height + doc_margin *2 + vMargin*2 + 2);
}

void TextBubble::initStyleSheet() {
    pTextEdit_->setStyleSheet("QTextEdit{background:transparent;border:none}");
}
