#include "searchlist.h"
#include<QScrollBar>
#include "adduseritem.h"
//#include "invaliditem.h"
#include "findsuccessdlg.h"
#include "tcpmgr.h"
#include "customizeedit.h"
#include "findfaildlg.h"
#include "loadingdlg.h"
#include "usermgr.h"

/******************************************************************************
 * @file       searchlist.h
 * @brief      聊天侧栏搜索列表类实现
 *
 * @author     lueying
 * @date       2026/1/27
 * @history
 *****************************************************************************/


SearchList::SearchList(QWidget* parent) :QListWidget(parent), find_dlg_(nullptr), search_edit_(nullptr), send_pending_(false) {
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    //连接点击的信号和槽
    connect(this, &QListWidget::itemClicked, this, &SearchList::slot_item_clicked);
    //添加条目
    addTipItem();
    //连接搜索条目
    connect(TcpMgr::getInstance().get(), &TcpMgr::sig_user_search, this, &SearchList::slot_user_search);
}

void SearchList::closeFindDlg() {
    if (find_dlg_) {
        find_dlg_->hide();
        find_dlg_ = nullptr;
    }
}

void SearchList::setSearchEdit(QWidget* edit) {
    search_edit_ = edit;
}

void SearchList::waitPending(bool pending) {
    if (pending) {
        loadingDialog_ = new LoadingDlg(this);
        loadingDialog_->setModal(true);
        loadingDialog_->show();
        send_pending_ = pending;
    }
    else {
        loadingDialog_->hide();
        loadingDialog_->deleteLater();
        send_pending_ = pending;
    }
}

// 添加列表条目
void SearchList::addTipItem() {
    auto* invalid_item = new QWidget();
    QListWidgetItem* item_tmp = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item_tmp->setSizeHint(QSize(250, 10));
    this->addItem(item_tmp);
    invalid_item->setObjectName("invalid_item");
    this->setItemWidget(item_tmp, invalid_item);
    item_tmp->setFlags(item_tmp->flags() & ~Qt::ItemIsSelectable);


    auto* add_user_item = new AddUserItem();
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(add_user_item->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, add_user_item);
}

// 搜索结果列表条目的点击槽函数
void SearchList::slot_item_clicked(QListWidgetItem* item) {
    QWidget* widget = this->itemWidget(item); // 获取自定义widget对象
    if (!widget) {
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }

    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    ListItemBase* customItem = qobject_cast<ListItemBase*>(widget);
    if (!customItem) {
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }

    auto itemType = customItem->getItemType();
    if (itemType == ListItemType::InvalidItem) {
        qDebug() << "slot invalid item clicked ";
        return;
    }

    if (itemType == ListItemType::AddUserTipItem) {
        if (send_pending_) {
            return;
        }

        if (!search_edit_) {
            return;
        }

        waitPending(true);
        auto search_edit = dynamic_cast<CustomizeEdit*>(search_edit_);
        auto uid_str = search_edit->text();
        QJsonObject jsonObj;
        jsonObj["uid"] = uid_str;

        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
        // 发送tcp请求给chat server
        emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_SEARCH_USER_REQ, jsonData);

        // 测试
        //emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_SEARCH_USER_REQ, jsonString);
        //find_dlg_ = std::make_shared<FindSuccessDlg>(this);
        //auto si = std::make_shared<SearchInfo>(0, "llfc", "llfc", "hello , my friend!", 0, ":/res/head_1.jpg");
        //(std::dynamic_pointer_cast<FindSuccessDlg>(find_dlg_))->setSearchInfo(si);
        //find_dlg_->show();

        return;
    }

    //清除弹出框
    closeFindDlg();
}

// 搜索完成槽函数
void SearchList::slot_user_search(std::shared_ptr<SearchInfo> si) {
    waitPending(false);
    if (si == nullptr) {
        find_dlg_ = std::make_shared<FindFailDlg>(this);
    }
    else {
        //如果是自己，暂且先直接返回，以后看逻辑扩充
        auto self_uid = UserMgr::getInstance()->getUid();
        if (si->_uid == self_uid) {
            return;
        }
        //此处分两种情况，一种是搜多到已经是自己的朋友了，一种是未添加好友
        //查找是否已经是好友
        bool bExist = UserMgr::getInstance()->checkFriendById(si->_uid);
        if (bExist) {
            //此处处理已经添加的好友，实现页面跳转
        //跳转到聊天界面指定的item中
            emit sig_jump_chat_item(si);
            return;
        }
        //此处先处理为添加的好友
        find_dlg_ = std::make_shared<FindSuccessDlg>(this);
        std::dynamic_pointer_cast<FindSuccessDlg>(find_dlg_)->setSearchInfo(si);

    }
    find_dlg_->show();
}

