#include "customizeedit.h"

/******************************************************************************
 * @file       customizededeit.h
 * @brief      搜索框类实现
 *
 * @author     lueying
 * @date       2026/1/8
 * @history
 *****************************************************************************/

CustomizeEdit::CustomizeEdit(QWidget* parent) :QLineEdit(parent), _max_len(0) {
    connect(this, &QLineEdit::textChanged, this, &CustomizeEdit::limitTextLength);
}

void CustomizeEdit::SetMaxLength(int maxLen) {
    _max_len = maxLen;
}