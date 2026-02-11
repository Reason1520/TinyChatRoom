#include "findsuccessdlg.h"
#include "ui_findsuccessdlg.h"
#include <QDir>
#include "applyfriend.h"
#include <memory>

/******************************************************************************
 * @file       findsuccessdlg.cpp
 * @brief      聊天侧栏搜索成功对话框类实现
 *
 * @author     lueying
 * @date       2026/1/27
 * @history
 *****************************************************************************/

FindSuccessDlg::FindSuccessDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindSuccessDlg),parent_(parent)
{
    ui->setupUi(this);
    // 设置对话框标题
    setWindowTitle("添加");
    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("FindSuccessDlg");
    // 获取当前应用程序的路径
    QString app_path = QCoreApplication::applicationDirPath();
    QString pix_path = QDir::toNativeSeparators(app_path +
                             QDir::separator() + "static"+QDir::separator()+"head_1.jpg");
    QPixmap head_pix(pix_path);
    head_pix = head_pix.scaled(ui->head_lb->size(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->head_lb->setPixmap(head_pix);
    ui->add_friend_btn->setState("normal","hover","press");
    this->setModal(true);
}

FindSuccessDlg::~FindSuccessDlg() {
    delete ui;
}

void FindSuccessDlg::setSearchInfo(std::shared_ptr<SearchInfo> si) {
    ui->name_lb->setText(si->_name);
    si_ = si;
}

void FindSuccessDlg::on_add_friend_btn_clicked() {
   this->hide();
   //弹出加好友界面
   auto applyFriend = new ApplyFriend(parent_);
   applyFriend->setSearchInfo(si_);
   applyFriend->setModal(true);
   applyFriend->show();
}
