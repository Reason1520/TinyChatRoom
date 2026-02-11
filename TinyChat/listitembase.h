#ifndef LISTITEMBASE_H
#define LISTITEMBASE_H
#include <QWidget>
#include "global.h"

/******************************************************************************
 * @file       listitembase.h
 * @brief      列表元素基类
 *
 * @author     lueying
 * @date       2026/1/18
 * @history
 *****************************************************************************/

class ListItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit ListItemBase(QWidget *parent = nullptr);
    void setItemType(ListItemType itemType);

    ListItemType getItemType();

private:
    ListItemType itemType_;

protected:
    void paintEvent(QPaintEvent* event) override;

public slots:

signals:


};

#endif // LISTITEMBASE_H
