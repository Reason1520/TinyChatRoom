#include "global.h"
#include <QEventLoop>
#include <QTimer>
#include <QUuid>
#include <QPainter>

/******************************************************************************
 * @file       global.cpp
 * @brief      全局通用函数实现
 *
 * @author     lueying
 * @date       2025/12/8
 * @history
 *****************************************************************************/

std::function<void(QWidget*)> repolish = [](QWidget* widget) {
	widget->style()->unpolish(widget);
	widget->style()->polish(widget);
};

// 密码加密
QString xorString(const QString& src) {
	QString result = src;
	int keyLen = XOR_KEY.length();

	for (int i = 0; i < src.length(); ++i) {
		// 将字符的 unicode 编码与密钥对应位置的 unicode 编码进行异或
		// 使用取余运算 % 循环使用密钥字符
		ushort code = src[i].unicode() ^ XOR_KEY[i % keyLen].unicode();
		result[i] = QChar(code);
	}

	return result;
}

void delay_run(int msecs) {
    QEventLoop loop;
    // singleShot 到时后会触发 loop.quit()，从而退出事件循环
    QTimer::singleShot(msecs, &loop, &QEventLoop::quit);
    loop.exec();
}

QString generateUniqueFileName(const QString& originalName) {
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QFileInfo fileInfo(originalName);
    QString extension = fileInfo.suffix();
    return uuid + (extension.isEmpty() ? "" : "." + extension);
}

QString generateUniqueIconName() {
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return uuid + ".png";
}

QString calculateFileHash(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    QCryptographicHash hash(QCryptographicHash::Md5);

    // 分块计算哈希，避免大文件占用过多内存
    const qint64 chunkSize = 1024 * 1024; // 1MB
    while (!file.atEnd())
    {
        hash.addData(file.read(chunkSize));
    }
    file.close();

    return hash.result().toHex();
}

QPixmap CreateLoadingPlaceholder(int width, int height) {
    QPixmap placeholder(width, height);
    placeholder.fill(QColor(240, 240, 240)); // 浅灰色背景

    QPainter painter(&placeholder);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制边框
    painter.setPen(QPen(QColor(200, 200, 200), 2));
    painter.drawRect(1, 1, width - 2, height - 2);

    // 绘制加载图标（简单的旋转圆圈或文字）
    QFont font;
    font.setPointSize(12);
    painter.setFont(font);
    painter.setPen(QColor(150, 150, 150));
    painter.drawText(placeholder.rect(), Qt::AlignCenter, "加载中...");

    // 可选：添加图片图标
    painter.setPen(QColor(180, 180, 180));
    QRect iconRect(width / 2 - 20, height / 2 - 40, 40, 30);
    painter.drawRect(iconRect);
    painter.drawLine(iconRect.topLeft(), iconRect.bottomRight());
    painter.drawLine(iconRect.topRight(), iconRect.bottomLeft());

    return placeholder;
}