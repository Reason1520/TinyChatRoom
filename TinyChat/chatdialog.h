#pragma once

#include <QDialog>
#include "global.h"
#include "ui_chatdialog.h"

/******************************************************************************
 * @file       chatdialog.h
 * @brief      聊天对话框类
 *
 * @author     lueying
 * @date       2026/1/7
 * @history
 *****************************************************************************/

class ChatDialog : public QDialog
{
	Q_OBJECT

public slots:
	void slot_loading_chat_user();									// 加载聊天用户列表槽函数
	void slot_side_chat();											// 聊天按钮点击槽函数
	void slot_side_contact();										// 联系人按钮点击槽函数
	void slot_side_setting();										// 设置按钮点击槽函数
	void slot_text_changed(const QString& str);						// 搜索框变化槽函数
	void slot_focus_out();											// 焦点离开搜索框槽函数
	void slot_loading_contact_user();								// 加载联系人列表槽函数
	void slot_switch_apply_friend_page();							// 切换到添加好友界面槽函数
	void slot_friend_info_page(std::shared_ptr<UserInfo> user_info);// 切换到好友信息界面槽函数
	void slot_show_search(bool show);								// 显示/隐藏搜索框槽函数
	void slot_apply_friend(std::shared_ptr<AddFriendApply> apply);	// 收到好友申请槽函数
	void slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info);	// 同意好友申请槽函数
	void slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp);			// 收到同意好友申请槽函数
	void slot_jump_chat_item(std::shared_ptr<SearchInfo> si);		// 从搜索列表跳转到指定的聊天条目
	void slot_jump_chat_item_from_infopage(std::shared_ptr<UserInfo> ui);// 从详情页点击“发消息”跳转到聊天窗口
	void slot_item_clicked(QListWidgetItem* item);					// 聊天列表条目被点击后的处理（如打开聊天窗）
	void slot_text_chat_msg(std::vector<std::shared_ptr<TextChatData>> msglists);	// 收到文字聊天消息槽函数
	void slot_img_chat_msg(std::shared_ptr<ImgChatData> imgchat);	// 收到图片聊天信息槽函数

	void slot_create_private_chat(int uid, int other_id, int thread_id);	// 创建私人聊天槽函数
	void slot_load_chat_thread(bool load_more, int last_thread_id,
		std::vector<std::shared_ptr<ChatThreadInfo>> chat_threads);	// 加载聊天线程列表槽函数
	void slot_load_chat_msg(int thread_id, int msg_id, bool load_more,
		std::vector<std::shared_ptr<ChatDataBase>> msglists);		// 加载聊天消息槽函数
	void slot_add_chat_msg(int thread_id, std::vector<std::shared_ptr<TextChatData>> msglists);
	void slot_add_img_msg(int thread_id, std::shared_ptr<ImgChatData> img_msg);
	void slot_reset_icon(QString path);
	void slot_update_upload_progress(std::shared_ptr<MsgInfo> msg_info);
	void slot_update_download_progress(std::shared_ptr<MsgInfo> msg_info);
	void slot_download_finish(std::shared_ptr<MsgInfo> msg_info, QString file_path);
private slots:
	void slot_reset_head();

public:
	explicit ChatDialog(QWidget *parent = nullptr);
	~ChatDialog();
	// 发送加载聊天线程列表信号
	void loadChatList();
	// 发送加载聊天信息信号
	void loadChatMsg();

protected:
	// 事件过滤器，用于捕获全局鼠标点击事件
	bool eventFilter(QObject* watched, QEvent* event) override;
	// 全局点击处理器，用于在点击非搜索区域时自动关闭搜索模式
	void handleGlobalMousePress(QMouseEvent* event);
	// 关闭添加好友对话框
	void closeFindDlg();
	// 将收到的聊天消息数据更新到当前的聊天展示页面
	void updateChatMsg(std::vector<std::shared_ptr<TextChatData> > msgdata);
	// 加载头像
	void loadHeadIcon(QString avatarPath, QLabel* icon_label, QString file_name, QString req_type);
private:
	Ui::ChatDialog* ui;
	ChatUIMode mode_;
	ChatUIMode state_;
	bool b_loading_;
	QList<StateWidget*> lb_list_;
	QWidget* last_widget_;
	QMap<int, QListWidgetItem*> chat_items_added_;
	//chat_thred_id和对应的item的映射关系。
	QMap<int, QListWidgetItem*>  chat_thread_items_;
	int cur_chat_thread_id_;
	QTimer* timer_;
	LoadingDlg* loading_dlg_;
	std::shared_ptr<ChatThreadData> cur_load_chat_;

	// 展示搜索列表
	void showSearch(bool bsearch = false);
	// 展示加载构件
	void showLoadingDlg(bool show = true);
	// 将侧边栏图标按钮添加到统一管理的群组列表中
	void addLBGroup(StateWidget* lb);
	// 清空侧边按钮状态,除了传入的
	void clearLabelState(StateWidget* lb);
	// 分页加载更多的聊天用户条目
	void loadMoreChatUser();
	// 分页加载更多的联系人条目
	void loadMoreConUser();
	// 设置聊天列表中指定的UID条目为选中状态
	void setSelectChatItem(int uid = 0);
	// 根据UID切换右侧展示的聊天对话页面
	void setSelectChatPage(int uid = 0);
};

