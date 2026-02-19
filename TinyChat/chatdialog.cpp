#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QAction>
#include "chatuserwid.h"
#include <QDebug>
#include <vector>
#include <QRandomGenerator>
#include "loadingdlg.h"
#include "global.h"
#include "ChatItemBase.h"
#include "TextBubble.h"
#include "PictureBubble.h"
#include "MessageTextEdit.h"
#include "chatuserlist.h"
#include "grouptipitem.h"
//#include "invaliditem.h"
#include "conuseritem.h"
//#include "lineitem.h"
#include "tcpmgr.h"
#include "usermgr.h"
#include <QTimer>
#include <QStandardPaths>
#include "filetcpMgr.h"

/******************************************************************************
 * @file       chatdialog.h
 * @brief      聊天对话框类实现
 *
 * @author     lueying
 * @date       2026/1/7
 * @history
 *****************************************************************************/

ChatDialog::ChatDialog(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::ChatDialog), b_loading_(false), mode_(ChatUIMode::ChatMode),
	state_(ChatUIMode::ChatMode), last_widget_(nullptr),
	cur_chat_thread_id_(0), loading_dlg_(nullptr), cur_load_chat_(nullptr){
	ui->setupUi(this);

	ui->add_btn->setState("normal", "hover", "press");
	ui->add_btn->setProperty("state", "normal");
	QAction* searchAction = new QAction(ui->search_edit);
	searchAction->setIcon(QIcon(":/res/search.png"));
	ui->search_edit->addAction(searchAction, QLineEdit::LeadingPosition);
	ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));


	// 创建一个清除动作并设置图标
	QAction* clearAction = new QAction(ui->search_edit);
	clearAction->setIcon(QIcon(":/res/close_transparent.png"));
	// 初始时不显示清除图标
	// 将清除动作添加到LineEdit的末尾位置
	ui->search_edit->addAction(clearAction, QLineEdit::TrailingPosition);

	// 当需要显示清除图标时，更改为实际的清除图标
	connect(ui->search_edit, &QLineEdit::textChanged, [clearAction](const QString& text) {
		if (!text.isEmpty()) {
			clearAction->setIcon(QIcon(":/res/close_search.png"));
		}
		else {
			clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 文本为空时，切换回透明图标
		}

		});

	// 连接清除动作的触发信号到槽函数，用于清除文本
	connect(clearAction, &QAction::triggered, [this, clearAction]() {
		ui->search_edit->clear();
		clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 清除文本后，切换回透明图标
		ui->search_edit->clearFocus();
		//清除按钮被按下则不显示搜索框
		showSearch(false);
		});

	ui->search_edit->SetMaxLength(15);

	//连接加载信号和槽
	connect(ui->chat_user_list, &ChatUserList::sig_loading_chat_user, this, &ChatDialog::slot_loading_chat_user);
	//刷新头像信号和槽函数
	connect(ui->user_info_page, &UserInfoPage::sig_reset_head, this, &ChatDialog::slot_reset_head);

	slot_reset_head();
	ui->side_chat_lb->setProperty("state", "normal");

	ui->side_chat_lb->setState("normal", "hover", "pressed", "selected_normal", "selected_hover", "selected_pressed");

	ui->side_contact_lb->setState("normal", "hover", "pressed", "selected_normal", "selected_hover", "selected_pressed");

	ui->side_settings_lb->setState("normal", "hover", "pressed", "selected_normal", "selected_hover", "selected_pressed");

	addLBGroup(ui->side_chat_lb);
	addLBGroup(ui->side_contact_lb);
	addLBGroup(ui->side_settings_lb);

	connect(ui->side_chat_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_chat);
	connect(ui->side_contact_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_contact);
	connect(ui->side_settings_lb, &StateWidget::clicked, this, &ChatDialog::slot_side_setting);

	//链接搜索框输入变化
	connect(ui->search_edit, &QLineEdit::textChanged, this, &ChatDialog::slot_text_changed);

	showSearch(false);

	//检测鼠标点击位置判断是否要清空搜索框
	this->installEventFilter(this); // 安装事件过滤器

	//设置聊天label选中状态
	ui->side_chat_lb->setSelected(true);
	//设置选中条目
	setSelectChatItem();
	//更新聊天界面信息
	setSelectChatPage();

	//连接加载联系人的信号和槽函数
	connect(ui->con_user_list, &ContactUserList::sig_loading_contact_user,
		this, &ChatDialog::slot_loading_contact_user);

	//连接联系人页面点击好友申请条目的信号
	connect(ui->con_user_list, &ContactUserList::sig_switch_apply_friend_page,
		this, &ChatDialog::slot_switch_apply_friend_page);

	//连接清除搜索框操作
	connect(ui->friend_apply_page, &ApplyFriendPage::sig_show_search, this, &ChatDialog::slot_show_search);

	//为searchlist 设置search edit
	ui->search_list->setSearchEdit(ui->search_edit);

	//连接申请添加好友信号
	connect(TcpMgr::getInstance().get(), &TcpMgr::sig_friend_apply, this, &ChatDialog::slot_apply_friend);

	//连接认证添加好友信号
	connect(TcpMgr::getInstance().get(), &TcpMgr::sig_add_auth_friend, this, &ChatDialog::slot_add_auth_friend);

	//链接自己认证回复信号
	connect(TcpMgr::getInstance().get(), &TcpMgr::sig_auth_rsp, this,
		&ChatDialog::slot_auth_rsp);

	//连接点击联系人item发出的信号和用户信息展示槽函数
	connect(ui->con_user_list, &ContactUserList::sig_switch_friend_info_page,
		this, &ChatDialog::slot_friend_info_page);

	//设置中心部件为chatpage
	ui->stackedWidget->setCurrentWidget(ui->chat_page);


	//连接searchlist跳转聊天信号
	connect(ui->search_list, &SearchList::sig_jump_chat_item, this, &ChatDialog::slot_jump_chat_item);

	//连接好友信息界面发送的点击事件
	connect(ui->friend_info_page, &FriendInfoPage::sig_jump_chat_item, this,
		&ChatDialog::slot_jump_chat_item_from_infopage);

	//连接聊天列表点击信号
	connect(ui->chat_user_list, &QListWidget::itemClicked, this, &ChatDialog::slot_item_clicked);

	//连接对端消息通知
	connect(TcpMgr::getInstance().get(), &TcpMgr::sig_text_chat_msg,
		this, &ChatDialog::slot_text_chat_msg);

	connect(TcpMgr::getInstance().get(), &TcpMgr::sig_img_chat_msg,
		this, &ChatDialog::slot_img_chat_msg);

	timer_ = new QTimer(this);
	connect(timer_, &QTimer::timeout, this, [this]() {
		auto user_info = UserMgr::getInstance()->getUserInfo();
		QJsonObject textObj;
		textObj["fromuid"] = user_info->_uid;
		QJsonDocument doc(textObj);
		QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
		emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_HEART_BEAT_REQ, jsonData);
		});

	timer_->start(10000);

	//连接tcp返回的加载聊天回复
	connect(TcpMgr::getInstance().get(), &TcpMgr::sig_load_chat_thread,
		this, &ChatDialog::slot_load_chat_thread);

	//连接tcp返回的创建私聊的回复
	connect(TcpMgr::getInstance().get(), &TcpMgr::sig_create_private_chat,
		this, &ChatDialog::slot_create_private_chat);

	// 连接加载聊天消息回复
	connect(TcpMgr::getInstance().get(), &TcpMgr::sig_load_chat_msg,
		this, &ChatDialog::slot_load_chat_msg);

	// 连接tcp返回的文字聊天信息回复
	connect(TcpMgr::getInstance().get(), &TcpMgr::sig_chat_msg_rsp, this, &ChatDialog::slot_add_chat_msg);

	//连接tcp返回的图片聊天信息回复
	connect(TcpMgr::getInstance().get(), &TcpMgr::sig_chat_img_rsp, this, &ChatDialog::slot_add_img_msg);
	//重置label icon
	connect(FileTcpMgr::getInstance().get(), &FileTcpMgr::sig_reset_label_icon, this, &ChatDialog::slot_reset_icon);
	//接收tcp返回的上传进度信息
	connect(FileTcpMgr::getInstance().get(), &FileTcpMgr::sig_update_upload_progress,
		this, &ChatDialog::slot_update_upload_progress);
	//接收tcp返回的下载进度信息
	connect(FileTcpMgr::getInstance().get(), &FileTcpMgr::sig_update_download_progress,
		this, &ChatDialog::slot_update_download_progress);

	//接收tcp返回的下载完成信息
	connect(FileTcpMgr::getInstance().get(), &FileTcpMgr::sig_download_finish,
		this, &ChatDialog::slot_download_finish);
}

ChatDialog::~ChatDialog() {
	timer_->stop();
	delete ui;
}

void ChatDialog::loadChatList() {
	showLoadingDlg(true);
	//发送请求逻辑
	QJsonObject jsonObj;
	auto uid = UserMgr::getInstance()->getUid();
	jsonObj["uid"] = uid;
	int last_chat_thread_id = UserMgr::getInstance()->getLastChatThreadId();
	jsonObj["thread_id"] = last_chat_thread_id;


	QJsonDocument doc(jsonObj);
	QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

	//发送tcp请求给chat server
	emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_THREAD_REQ, jsonData);
}


void ChatDialog::loadChatMsg() {

	//发送聊天记录请求
	cur_load_chat_ = UserMgr::getInstance()->getCurLoadData();
	if (cur_load_chat_ == nullptr) {
		return;
	}

	showLoadingDlg(true);

	//发送请求给服务器
		//发送请求逻辑
	QJsonObject jsonObj;
	jsonObj["thread_id"] = cur_load_chat_->GetThreadId();
	jsonObj["message_id"] = cur_load_chat_->GetLastMsgId();

	QJsonDocument doc(jsonObj);
	QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

	//发送tcp请求给chat server
	emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_MSG_REQ, jsonData);
}

void ChatDialog::slot_item_clicked(QListWidgetItem* item) {
	QWidget* widget = ui->chat_user_list->itemWidget(item); // 获取自定义widget对象
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
	if (itemType == ListItemType::InvalidItem
		|| itemType == ListItemType::GroupTipItem) {
		qDebug() << "slot invalid item clicked ";
		return;
	}


	if (itemType == ListItemType::ChatUserItem) {
		// 创建对话框，提示用户
		qDebug() << "contact user item clicked ";

		auto chat_wid = qobject_cast<ChatUserWid*>(customItem);
		auto chat_data = chat_wid->getChatData();
		//跳转到聊天界面
		ui->chat_page->setChatData(chat_data);
		cur_chat_thread_id_ = chat_data->GetThreadId();
		return;
	}
}

//添加聊天消息, 将消息放到用户区和thread_id关联
void ChatDialog::slot_text_chat_msg(std::vector<std::shared_ptr<TextChatData>> msglists) {
	for (auto& msg : msglists) {

		//更新数据
		auto thread_id = msg->GetThreadId();
		auto thread_data = UserMgr::getInstance()->getChatThreadByThreadId(thread_id);

		thread_data->AddMsg(msg);

		if (cur_chat_thread_id_ != thread_id) {
			continue;
		}

		ui->chat_page->appendChatMsg(msg);
	}

}

void ChatDialog::slot_img_chat_msg(std::shared_ptr<ImgChatData> imgchat) {
	//更新数据
	auto thread_id = imgchat->GetThreadId();
	auto thread_data = UserMgr::getInstance()->getChatThreadByThreadId(thread_id);
	thread_data->AddMsg(imgchat);
	if (cur_chat_thread_id_ != thread_id) {
		return;
	}

	ui->chat_page->appendOtherMsg(imgchat);
}


bool ChatDialog::eventFilter(QObject* watched, QEvent* event) {
	if (event->type() == QEvent::MouseButtonPress) {
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		handleGlobalMousePress(mouseEvent);
	}
	return QDialog::eventFilter(watched, event);
}

void ChatDialog::handleGlobalMousePress(QMouseEvent* event) {
	// 实现点击位置的判断和处理逻辑
	// 先判断是否处于搜索模式，如果不处于搜索模式则直接返回
	if (mode_ != ChatUIMode::SearchMode) {
		return;
	}

	// 将鼠标点击位置转换为搜索列表坐标系中的位置
	QPoint posInSearchList = ui->search_list->mapFromGlobal(event->globalPosition().toPoint());
	// 判断点击位置是否在聊天列表的范围内
	if (!ui->search_list->rect().contains(posInSearchList)) {
		// 如果不在聊天列表内，清空输入框
		ui->search_edit->clear();
		showSearch(false);
	}
}

void ChatDialog::closeFindDlg() {
	ui->search_list->closeFindDlg();
}

void ChatDialog::updateChatMsg(std::vector<std::shared_ptr<TextChatData> > msgdata) {
	for (auto& msg : msgdata) {
		if (msg->GetThreadId() != cur_chat_thread_id_) {
			break;
		}

		ui->chat_page->appendChatMsg(msg);
	}
}


void ChatDialog::slot_load_chat_thread(bool load_more, int last_thread_id,
	std::vector<std::shared_ptr<ChatThreadInfo>> chat_threads) {
	for (auto& cti : chat_threads) {
		//先处理单聊，群聊跳过，以后添加
		if (cti->_type == "group") {
			continue;
		}

		auto uid = UserMgr::getInstance()->getUid();
		auto other_uid = 0;
		if (uid == cti->_user1_id) {
			other_uid = cti->_user2_id;
		}
		else {
			other_uid = cti->_user1_id;
		}

		auto chat_thread_data = std::make_shared<ChatThreadData>(other_uid, cti->_thread_id, 0);
		UserMgr::getInstance()->addChatThreadData(chat_thread_data, other_uid);

		auto* chat_user_wid = new ChatUserWid();
		chat_user_wid->setChatData(chat_thread_data);
		QListWidgetItem* item = new QListWidgetItem;
		//qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
		item->setSizeHint(chat_user_wid->sizeHint());
		ui->chat_user_list->addItem(item);
		ui->chat_user_list->setItemWidget(item, chat_user_wid);
		chat_thread_items_.insert(cti->_thread_id, item);
	}

	UserMgr::getInstance()->setLastChatThreadId(last_thread_id);

	if (load_more) {
		//发送请求逻辑
		QJsonObject jsonObj;
		auto uid = UserMgr::getInstance()->getUid();
		jsonObj["uid"] = uid;
		jsonObj["thread_id"] = last_thread_id;


		QJsonDocument doc(jsonObj);
		QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

		//发送tcp请求给chat server
		emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_THREAD_REQ, jsonData);
		return;
	}

	showLoadingDlg(false);
	//继续加载聊天数据
	loadChatMsg();
}

void ChatDialog::slot_create_private_chat(int uid, int other_id, int thread_id) {
	auto* chat_user_wid = new ChatUserWid();
	auto chat_thread_data = std::make_shared<ChatThreadData>(other_id, thread_id, 0);
	if (chat_thread_data == nullptr) {
		return;
	}
	UserMgr::getInstance()->addChatThreadData(chat_thread_data, other_id);

	chat_user_wid->setChatData(chat_thread_data);
	QListWidgetItem* item = new QListWidgetItem;
	item->setSizeHint(chat_user_wid->sizeHint());
	qDebug() << "chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
	ui->chat_user_list->insertItem(0, item);
	ui->chat_user_list->setItemWidget(item, chat_user_wid);
	chat_thread_items_.insert(thread_id, item);

	ui->side_chat_lb->setSelected(true);
	setSelectChatItem(thread_id);
	//更新聊天界面信息
	setSelectChatPage(thread_id);
	slot_side_chat();
	return;
}

void ChatDialog::slot_load_chat_msg(int thread_id, int msg_id, bool load_more,
	std::vector<std::shared_ptr<ChatDataBase>> msglists) {
	cur_load_chat_->SetLastMsgId(msg_id);
	//加载聊天信息
	for (auto& chat_msg : msglists) {
		cur_load_chat_->AppendMsg(chat_msg->GetMsgId(), chat_msg);
	}

	//还有未加载完的消息，就继续加载
	if (load_more) {
		//发送请求给服务器
			//发送请求逻辑
		QJsonObject jsonObj;
		jsonObj["thread_id"] = cur_load_chat_->GetThreadId();
		jsonObj["message_id"] = cur_load_chat_->GetLastMsgId();

		QJsonDocument doc(jsonObj);
		QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

		//发送tcp请求给chat server
		emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_MSG_REQ, jsonData);
		return;
	}

	//获取下一个chat_thread
	cur_load_chat_ = UserMgr::getInstance()->getNextLoadData();
	//都加载完了
	if (!cur_load_chat_) {
		//更新聊天界面信息
		setSelectChatItem();
		setSelectChatPage();
		showLoadingDlg(false);
		return;
	}

	//继续加载下一个聊天
	//发送请求给服务器
	//发送请求逻辑
	QJsonObject jsonObj;
	jsonObj["thread_id"] = cur_load_chat_->GetThreadId();
	jsonObj["message_id"] = cur_load_chat_->GetLastMsgId();

	QJsonDocument doc(jsonObj);
	QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

	//发送tcp请求给chat server
	emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_LOAD_CHAT_MSG_REQ, jsonData);
}


void ChatDialog::slot_add_chat_msg(int thread_id, std::vector<std::shared_ptr<TextChatData>> msglists) {
	auto chat_data = UserMgr::getInstance()->getChatThreadByThreadId(thread_id);
	if (chat_data == nullptr) {
		return;
	}

	//将消息放入数据中管理
	for (auto& msg : msglists) {
		chat_data->MoveMsg(msg);

		if (cur_chat_thread_id_ != thread_id) {
			continue;
		}
		//更新聊天界面信息
		ui->chat_page->updateChatStatus(msg);
	}
}


void ChatDialog::slot_add_img_msg(int thread_id, std::shared_ptr<ImgChatData> img_msg) {
	auto chat_data = UserMgr::getInstance()->getChatThreadByThreadId(thread_id);
	if (chat_data == nullptr) {
		return;
	}

	chat_data->MoveMsg(img_msg);

	if (cur_chat_thread_id_ != thread_id) {
		return;
	}

	//更新聊天界面信息
	ui->chat_page->updateImgChatStatus(img_msg);
}

void ChatDialog::slot_reset_icon(QString path) {
	UserMgr::getInstance()->resetLabelIcon(path);
}

void ChatDialog::showLoadingDlg(bool show) {
	if (show) {
		if (loading_dlg_) {
			loading_dlg_->deleteLater();
		}
		loading_dlg_ = new LoadingDlg(this, "正在加载聊天列表...");
		loading_dlg_->setModal(true);
		loading_dlg_->show();
		return;
	}

	if (loading_dlg_) {
		loading_dlg_->deleteLater();
		loading_dlg_ = nullptr;
	}

}

void ChatDialog::addLBGroup(StateWidget* lb) {
	lb_list_.push_back(lb);
}



//void ChatDialog::addChatUserList()
//{
//    //先按照好友列表加载聊天记录，等以后客户端实现聊天记录数据库之后再按照最后信息排序
//    auto friend_list = UserMgr::GetInstance()->GetChatListPerPage();
//    if (friend_list.empty() == false) {
//        for(auto & friend_ele : friend_list){
//            auto find_iter = _chat_items_added.find(friend_ele->_uid);
//            if(find_iter != _chat_items_added.end()){
//                continue;
//            }
//            auto *chat_user_wid = new ChatUserWid();
//            auto user_info = std::make_shared<UserInfo>(friend_ele);
//            chat_user_wid->SetInfo(user_info);
//            QListWidgetItem *item = new QListWidgetItem;
//            //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
//            item->setSizeHint(chat_user_wid->sizeHint());
//            ui->chat_user_list->addItem(item);
//            ui->chat_user_list->setItemWidget(item, chat_user_wid);
//            _chat_items_added.insert(friend_ele->_uid, item);
//        }
//
//        //更新已加载条目
//        UserMgr::GetInstance()->UpdateChatLoadedCount();
//    }
//
//    //模拟测试条目
//    // 创建QListWidgetItem，并设置自定义的widget
//    for(int i = 0; i < 13; i++){
//        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
//        int str_i = randomValue%strs.size();
//        int head_i = randomValue%heads.size();
//        int name_i = randomValue%names.size();
//
//        auto *chat_user_wid = new ChatUserWid();
//        auto user_info = std::make_shared<UserInfo>(0,names[name_i],
//                                                    names[name_i],heads[head_i],0,strs[str_i]);
//        chat_user_wid->SetInfo(user_info);
//        QListWidgetItem *item = new QListWidgetItem;
//        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
//        item->setSizeHint(chat_user_wid->sizeHint());
//        ui->chat_user_list->addItem(item);
//        ui->chat_user_list->setItemWidget(item, chat_user_wid);
//    }
//
//}

//todo: 加载更多联系人，后期从数据库里添加
void ChatDialog::loadMoreChatUser() {
	auto friend_list = UserMgr::getInstance()->getConListPerPage();
	if (friend_list.empty() == false) {
		for (auto& friend_ele : friend_list) {
			auto* chat_user_wid = new ConUserItem();
			chat_user_wid->setInfo(friend_ele->_uid, friend_ele->_name,
				friend_ele->_icon);
			QListWidgetItem* item = new QListWidgetItem;
			//qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
			item->setSizeHint(chat_user_wid->sizeHint());
			ui->con_user_list->addItem(item);
			ui->con_user_list->setItemWidget(item, chat_user_wid);
		}

		//更新已加载条目
		UserMgr::getInstance()->updateContactLoadedCount();
	}
}


void ChatDialog::clearLabelState(StateWidget* lb)
{
	for (auto& ele : lb_list_) {
		if (ele == lb) {
			continue;
		}

		ele->clearState();
	}
}

void ChatDialog::loadMoreConUser()
{
	auto friend_list = UserMgr::getInstance()->getConListPerPage();
	if (friend_list.empty() == false) {
		for (auto& friend_ele : friend_list) {
			auto* chat_user_wid = new ConUserItem();
			chat_user_wid->setInfo(friend_ele->_uid, friend_ele->_name,
				friend_ele->_icon);
			QListWidgetItem* item = new QListWidgetItem;
			//qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
			item->setSizeHint(chat_user_wid->sizeHint());
			ui->con_user_list->addItem(item);
			ui->con_user_list->setItemWidget(item, chat_user_wid);
		}

		//更新已加载条目
		UserMgr::getInstance()->updateContactLoadedCount();
	}
}

void ChatDialog::setSelectChatItem(int thread_id) {
	if (ui->chat_user_list->count() <= 0) {
		return;
	}

	if (thread_id == 0) {
		ui->chat_user_list->setCurrentRow(0);
		QListWidgetItem* firstItem = ui->chat_user_list->item(0);
		if (!firstItem) {
			return;
		}

		//转为widget
		QWidget* widget = ui->chat_user_list->itemWidget(firstItem);
		if (!widget) {
			return;
		}

		auto con_item = qobject_cast<ChatUserWid*>(widget);
		if (!con_item) {
			return;
		}

		cur_chat_thread_id_ = con_item->getChatData()->GetThreadId();

		return;
	}

	auto find_iter = chat_thread_items_.find(thread_id);
	if (find_iter == chat_thread_items_.end()) {
		qDebug() << "thread_id [" << thread_id << "] not found, set curent row 0";
		ui->chat_user_list->setCurrentRow(0);
		return;
	}

	ui->chat_user_list->setCurrentItem(find_iter.value());

	cur_chat_thread_id_ = thread_id;
}

void ChatDialog::setSelectChatPage(int thread_id) {
	if (ui->chat_user_list->count() <= 0) {
		return;
	}

	if (thread_id == 0) {
		auto item = ui->chat_user_list->item(0);
		//转为widget
		QWidget* widget = ui->chat_user_list->itemWidget(item);
		if (!widget) {
			return;
		}

		auto con_item = qobject_cast<ChatUserWid*>(widget);
		if (!con_item) {
			return;
		}

		//设置信息
		auto chat_data = con_item->getChatData();
		ui->chat_page->setChatData(chat_data);
		return;
	}

	auto find_iter = chat_thread_items_.find(thread_id);
	if (find_iter == chat_thread_items_.end()) {
		return;
	}

	//转为widget
	QWidget* widget = ui->chat_user_list->itemWidget(find_iter.value());
	if (!widget) {
		return;
	}

	//判断转化为自定义的widget
	// 对自定义widget进行操作， 将item 转化为基类ListItemBase
	ListItemBase* customItem = qobject_cast<ListItemBase*>(widget);
	if (!customItem) {
		qDebug() << "qobject_cast<ListItemBase*>(widget) is nullptr";
		return;
	}

	auto itemType = customItem->getItemType();
	if (itemType == ListItemType::ChatUserItem) {
		auto con_item = qobject_cast<ChatUserWid*>(customItem);
		if (!con_item) {
			return;
		}

		//设置信息
		auto chat_data = con_item->getChatData();
		ui->chat_page->setChatData(chat_data);

		return;
	}

}


void ChatDialog::showSearch(bool bsearch) {
	if (bsearch) {
		ui->chat_user_list->hide();
		ui->con_user_list->hide();
		ui->search_list->show();
		mode_ = ChatUIMode::SearchMode;
	}
	else if (state_ == ChatUIMode::ChatMode) {
		ui->chat_user_list->show();
		ui->con_user_list->hide();
		ui->search_list->hide();
		mode_ = ChatUIMode::ChatMode;
		ui->search_list->closeFindDlg();
		ui->search_edit->clear();
		ui->search_edit->clearFocus();
	}
	else if (state_ == ChatUIMode::ContactMode) {
		ui->chat_user_list->hide();
		ui->search_list->hide();
		ui->con_user_list->show();
		mode_ = ChatUIMode::ContactMode;
		ui->search_list->closeFindDlg();
		ui->search_edit->clear();
		ui->search_edit->clearFocus();
	}
	else if (state_ == ChatUIMode::SettingsMode) {
		ui->chat_user_list->hide();
		ui->search_list->hide();
		ui->con_user_list->show();
		mode_ = ChatUIMode::ContactMode;
		ui->search_list->closeFindDlg();
		ui->search_edit->clear();
		ui->search_edit->clearFocus();
	}
}

void ChatDialog::slot_loading_chat_user() {
	if (b_loading_) {
		return;
	}

	b_loading_ = true;
	LoadingDlg* loadingDialog = new LoadingDlg(this);
	loadingDialog->setModal(true);
	loadingDialog->show();
	qDebug() << "add new data to list.....";
	loadMoreChatUser();
	// 加载完成后关闭对话框
	loadingDialog->deleteLater();

	b_loading_ = false;
}

void ChatDialog::slot_side_chat() {
	qDebug() << "receive side chat clicked";
	clearLabelState(ui->side_chat_lb);
	ui->stackedWidget->setCurrentWidget(ui->chat_page);
	state_ = ChatUIMode::ChatMode;
	showSearch(false);
	//设置选中条目
	setSelectChatItem(cur_chat_thread_id_);
	//更新聊天界面信息
	setSelectChatPage(cur_chat_thread_id_);
}

void ChatDialog::slot_side_contact() {
	qDebug() << "receive side contact clicked";
	clearLabelState(ui->side_contact_lb);
	//设置
	if (last_widget_ == nullptr) {
		ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
		last_widget_ = ui->friend_apply_page;
	}
	else {
		ui->stackedWidget->setCurrentWidget(last_widget_);
	}

	state_ = ChatUIMode::ContactMode;
	showSearch(false);
}

void ChatDialog::slot_side_setting() {
	qDebug() << "receive side setting clicked";
	clearLabelState(ui->side_settings_lb);
	//设置
	ui->stackedWidget->setCurrentWidget(ui->user_info_page);

	state_ = ChatUIMode::SettingsMode;
	showSearch(false);
}

void ChatDialog::slot_text_changed(const QString& str) {
	//qDebug()<< "receive slot text changed str is " << str;
	if (!str.isEmpty()) {
		showSearch(true);
	}
}

void ChatDialog::slot_focus_out() {
	qDebug() << "receive focus out signal";
	showSearch(false);
}

void ChatDialog::slot_loading_contact_user() {
	qDebug() << "slot loading contact user";
	if (b_loading_) {
		return;
	}

	b_loading_ = true;
	LoadingDlg* loadingDialog = new LoadingDlg(this);
	loadingDialog->setModal(true);
	loadingDialog->show();
	qDebug() << "add new data to list.....";
	loadMoreConUser();
	// 加载完成后关闭对话框
	loadingDialog->deleteLater();

	b_loading_ = false;
}

void ChatDialog::slot_switch_apply_friend_page() {
	qDebug() << "receive switch apply friend page sig";
	last_widget_ = ui->friend_apply_page;
	ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
}

void ChatDialog::slot_friend_info_page(std::shared_ptr<UserInfo> user_info) {
	qDebug() << "receive switch friend info page sig";
	last_widget_ = ui->friend_info_page;
	ui->stackedWidget->setCurrentWidget(ui->friend_info_page);
	ui->friend_info_page->SetInfo(user_info);
}



void ChatDialog::slot_show_search(bool show) {
	showSearch(show);
}

void ChatDialog::slot_apply_friend(std::shared_ptr<AddFriendApply> apply) {
	qDebug() << "receive apply friend slot, applyuid is " << apply->_from_uid << " name is "
		<< apply->_name << " desc is " << apply->_desc;

	bool b_already = UserMgr::getInstance()->alreadyApply(apply->_from_uid);
	if (b_already) {
		return;
	}

	UserMgr::getInstance()->addApplyList(std::make_shared<ApplyInfo>(apply));
	ui->side_contact_lb->showRedPoint(true);
	ui->con_user_list->showRedPoint(true);
	ui->friend_apply_page->addNewApply(apply);
}

void ChatDialog::slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info) {
	qDebug() << "receive slot_add_auth__friend uid is " << auth_info->_uid
		<< " name is " << auth_info->_name << " nick is " << auth_info->_nick;

	//判断如果已经是好友则跳过
	auto bfriend = UserMgr::getInstance()->checkFriendById(auth_info->_uid);
	if (bfriend) {
		return;
	}

	UserMgr::getInstance()->addFriend(auth_info);

	auto* chat_user_wid = new ChatUserWid();
	auto chat_thread_data = std::make_shared<ChatThreadData>(auth_info->_uid, auth_info->_thread_id, 0);
	UserMgr::getInstance()->addChatThreadData(chat_thread_data, auth_info->_uid);
	for (auto& chat_msg : auth_info->_chat_datas) {
		chat_thread_data->AppendMsg(chat_msg->GetMsgId(), chat_msg);
	}

	chat_user_wid->setChatData(chat_thread_data);
	QListWidgetItem* item = new QListWidgetItem;
	//qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
	item->setSizeHint(chat_user_wid->sizeHint());
	ui->chat_user_list->insertItem(0, item);
	ui->chat_user_list->setItemWidget(item, chat_user_wid);
	chat_thread_items_.insert(auth_info->_thread_id, item);
}

void ChatDialog::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp) {
	qDebug() << "receive slot_auth_rsp uid is " << auth_rsp->_uid
		<< " name is " << auth_rsp->_name << " nick is " << auth_rsp->_nick;

	//判断如果已经是好友则跳过
	auto bfriend = UserMgr::getInstance()->checkFriendById(auth_rsp->_uid);
	if (bfriend) {
		return;
	}

	UserMgr::getInstance()->addFriend(auth_rsp);
	int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
	int str_i = randomValue % strs.size();
	int head_i = randomValue % heads.size();
	int name_i = randomValue % names.size();

	auto* chat_user_wid = new ChatUserWid();
	auto chat_thread_data = std::make_shared<ChatThreadData>(auth_rsp->_uid, auth_rsp->_thread_id, 0);
	UserMgr::getInstance()->addChatThreadData(chat_thread_data, auth_rsp->_uid);
	for (auto& chat_msg : auth_rsp->_chat_datas) {
		chat_thread_data->AppendMsg(chat_msg->GetMsgId(), chat_msg);
	}
	chat_user_wid->setChatData(chat_thread_data);
	QListWidgetItem* item = new QListWidgetItem;
	//qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
	item->setSizeHint(chat_user_wid->sizeHint());
	ui->chat_user_list->insertItem(0, item);
	ui->chat_user_list->setItemWidget(item, chat_user_wid);
	chat_thread_items_.insert(auth_rsp->_thread_id, item);
}

//todo: 点击搜索的联系人聊天，跳转到聊天界面，之后添加申请创建私聊或者查找已有 私聊消息
void ChatDialog::slot_jump_chat_item(std::shared_ptr<SearchInfo> si) {
	qDebug() << "slot jump chat item ";
	auto chat_thread_data = UserMgr::getInstance()->getChatThreadByUid(si->_uid);
	if (chat_thread_data) {
		auto find_iter = chat_thread_items_.find(chat_thread_data->GetThreadId());
		if (find_iter != chat_thread_items_.end()) {
			qDebug() << "jump to chat item , uid is " << si->_uid;
			ui->chat_user_list->scrollToItem(find_iter.value());
			ui->side_chat_lb->setSelected(true);
			setSelectChatItem(chat_thread_data->GetThreadId());
			//更新聊天界面信息
			setSelectChatPage(chat_thread_data->GetThreadId());
			slot_side_chat();
			return;
		} //说明之前有缓存过聊天列表，只是被删除了，那么重新加进来即可
		else {
			auto* chat_user_wid = new ChatUserWid();
			chat_user_wid->setChatData(chat_thread_data);
			QListWidgetItem* item = new QListWidgetItem;
			qDebug() << "chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
			ui->chat_user_list->insertItem(0, item);
			ui->chat_user_list->setItemWidget(item, chat_user_wid);
			chat_thread_items_.insert(chat_thread_data->GetThreadId(), item);
			ui->side_chat_lb->setSelected(true);
			setSelectChatItem(chat_thread_data->GetThreadId());
			//更新聊天界面信息
			setSelectChatPage(chat_thread_data->GetThreadId());
			slot_side_chat();
			return;
		}
	}

	//如果没找到，则发送创建请求
	auto uid = UserMgr::getInstance()->getUid();
	QJsonObject jsonObj;
	jsonObj["uid"] = uid;
	jsonObj["other_id"] = si->_uid;

	QJsonDocument doc(jsonObj);
	QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

	//发送tcp请求给chat server
	emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_CREATE_PRIVATE_CHAT_REQ, jsonData);

}

void ChatDialog::slot_jump_chat_item_from_infopage(std::shared_ptr<UserInfo> user_info) {
	qDebug() << "slot jump chat item ";
	auto chat_thread_data = UserMgr::getInstance()->getChatThreadByUid(user_info->_uid);
	if (chat_thread_data) {
		auto find_iter = chat_thread_items_.find(chat_thread_data->GetThreadId());
		if (find_iter != chat_thread_items_.end()) {
			qDebug() << "jump to chat item , uid is " << user_info->_uid;
			ui->chat_user_list->scrollToItem(find_iter.value());
			ui->side_chat_lb->setSelected(true);
			setSelectChatItem(chat_thread_data->GetThreadId());
			//更新聊天界面信息
			setSelectChatPage(chat_thread_data->GetThreadId());
			slot_side_chat();
			return;
		} //说明之前有缓存过聊天列表，只是被删除了，那么重新加进来即可
		else {
			auto* chat_user_wid = new ChatUserWid();
			chat_user_wid->setChatData(chat_thread_data);
			QListWidgetItem* item = new QListWidgetItem;
			qDebug() << "chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
			ui->chat_user_list->insertItem(0, item);
			ui->chat_user_list->setItemWidget(item, chat_user_wid);
			chat_thread_items_.insert(chat_thread_data->GetThreadId(), item);
			ui->side_chat_lb->setSelected(true);
			setSelectChatItem(chat_thread_data->GetThreadId());
			//更新聊天界面信息
			setSelectChatPage(chat_thread_data->GetThreadId());
			slot_side_chat();
			return;
		}
	}

	//如果没找到，则发送创建请求
	auto uid = UserMgr::getInstance()->getUid();
	QJsonObject jsonObj;
	jsonObj["uid"] = uid;
	jsonObj["other_id"] = user_info->_uid;

	QJsonDocument doc(jsonObj);
	QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

	//发送tcp请求给chat server
	emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_CREATE_PRIVATE_CHAT_REQ, jsonData);
}


void ChatDialog::slot_update_upload_progress(std::shared_ptr<MsgInfo> msg_info) {
	auto chat_data = UserMgr::getInstance()->getChatThreadByThreadId(msg_info->_thread_id);
	if (chat_data == nullptr) {
		return;
	}

	//更新消息，其实不用更新，都是共享msg_info的一块内存，这里为了安全还是再次更新下

	chat_data->UpdateProgress(msg_info);

	if (cur_chat_thread_id_ != msg_info->_thread_id) {
		return;
	}


	//更新聊天界面信息
	ui->chat_page->updateFileProgress(msg_info);
}


void ChatDialog::slot_update_download_progress(std::shared_ptr<MsgInfo> msg_info) {
	auto chat_data = UserMgr::getInstance()->getChatThreadByThreadId(msg_info->_thread_id);
	if (chat_data == nullptr) {
		return;
	}

	//更新消息，其实不用更新，都是共享msg_info的一块内存，这里为了安全还是再次更新下

	chat_data->UpdateProgress(msg_info);

	if (cur_chat_thread_id_ != msg_info->_thread_id) {
		return;
	}


	//更新聊天界面信息
	ui->chat_page->updateFileProgress(msg_info);
}
void ChatDialog::slot_download_finish(std::shared_ptr<MsgInfo> msg_info, QString file_path) {
	auto chat_data = UserMgr::getInstance()->getChatThreadByThreadId(msg_info->_thread_id);
	if (chat_data == nullptr) {
		return;
	}

	//更新消息，其实不用更新，都是共享msg_info的一块内存，这里为了安全还是再次更新下

	chat_data->UpdateProgress(msg_info);

	if (cur_chat_thread_id_ != msg_info->_thread_id) {
		return;
	}


	//更新聊天界面信息
	ui->chat_page->downloadFileFinished(msg_info, file_path);
}

void ChatDialog::slot_reset_head() {
	//模拟加载自己头像
	QString head_icon = UserMgr::getInstance()->getIcon();
	//使用正则表达式检查是否使用默认头像
	QRegularExpression regex("^:/res/head_(\\d+)\\.jpg$");
	QRegularExpressionMatch match = regex.match(head_icon);
	if (match.hasMatch()) {
		// 如果是默认头像（:/res/head_X.jpg 格式）
		QPixmap pixmap(head_icon); // 加载默认头像图片
		QPixmap scaledPixmap = pixmap.scaled(ui->side_head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		ui->side_head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
		ui->side_head_lb->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小
	}
	else {
		// 如果是用户上传的头像，获取存储目录
		QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
		auto uid = UserMgr::getInstance()->getUid();
		QDir avatarsDir(storageDir + "/user/" + QString::number(uid) + "/avatars");

		// 确保目录存在
		if (avatarsDir.exists()) {
			auto file_name = QFileInfo(head_icon).fileName();
			QString avatarPath = avatarsDir.filePath(file_name); // 获取上传头像的完整路径
			QPixmap pixmap(avatarPath); // 加载上传的头像图片
			if (!pixmap.isNull()) {
				QPixmap scaledPixmap = pixmap.scaled(ui->side_head_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
				ui->side_head_lb->setPixmap(scaledPixmap);
				ui->side_head_lb->setScaledContents(true);
			}
			else {
				qWarning() << "无法加载上传的头像：" << avatarPath;
				loadHeadIcon(avatarPath, ui->side_head_lb, file_name, "self_icon");
			}
		}
		else {
			qWarning() << "头像存储目录不存在：" << avatarsDir.path();
			QString avatarPath = avatarsDir.filePath(QFileInfo(head_icon).fileName());
			avatarsDir.mkpath(".");
			loadHeadIcon(avatarPath, ui->side_head_lb, head_icon, "self_icon");
		}
	}
}


void ChatDialog::loadHeadIcon(QString avatarPath, QLabel* icon_label, QString file_name, QString req_type) {
	UserMgr::getInstance()->addLabelToReset(avatarPath, icon_label);
	//先加载默认的
	QPixmap pixmap(":/res/head_1.jpg");
	QPixmap scaledPixmap = pixmap.scaled(icon_label->size(),
		Qt::KeepAspectRatio, Qt::SmoothTransformation); // 将图片缩放到label的大小
	icon_label->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
	icon_label->setScaledContents(true); // 设置QLabel自动缩放图片内容以适应大小

	//判断是否正在下载
	bool is_loading = UserMgr::getInstance()->isDownLoading(file_name);
	if (is_loading) {
		qWarning() << "正在下载: " << file_name;
	}
	else {
		//发送请求获取资源
		auto download_info = std::make_shared<DownloadInfo>();
		download_info->_name = file_name;
		download_info->_current_size = 0;
		download_info->_seq = 1;
		download_info->_total_size = 0;
		download_info->_client_path = avatarPath;
		//添加文件到管理者
		UserMgr::getInstance()->addDownloadFile(file_name, download_info);
		//发送消息
		FileTcpMgr::getInstance()->SendDownloadInfo(download_info, req_type);
	}
}