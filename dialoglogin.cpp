#include "dialoglogin.h"
#include "ui_dialoglogin.h"
#include "dialogreg.h"
#include "settings.h"

DialogLogin::DialogLogin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLogin)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint&~Qt::WindowContextHelpButtonHint);
    setFixedSize(this->width(), this->height());    // 设置登录窗口不显示帮助按钮、并固定窗口大小;
    ui->LineEditPwd->setEchoMode(QLineEdit::Password);  // 设置密码输入框为密码输入模式（不显示明文）;
    dlgReg = new DialogReg(this);
    if (settings::usercount == 0)
    {
        QMessageBox::information(this, u8"提示", u8"欢迎使用！\n请注册后登录！");
    }
}

DialogLogin::~DialogLogin()
{
    delete ui;
}

bool DialogLogin::verify_user_info(const QString inputUsr, const QString inputPwd)
{
    QString md5Pwd(QCryptographicHash::hash(inputPwd.toLatin1(), QCryptographicHash::Md5).toHex());
    QJsonObject cur;
    foreach(QJsonValue i, settings::userlist)
    {
        cur = i.toObject();
        if (cur["id"] == inputUsr && cur["password"] == md5Pwd)
        {
            settings::curUserPermisson = cur["permission"].toInt();
            return true;
        }
    }
    return false;
}

void DialogLogin::on_btnLogin_clicked()
{
    if (DialogLogin::verify_user_info(ui->LineEditUser->text().trimmed(), ui->LineEditPwd->text()))
    {
        settings::username = ui->LineEditUser->text().trimmed();
        settings::write_log(settings::username + " 登录系统");  // 日志记录
        accept();
        destroy();
    }
    else
    {
        QMessageBox::critical(this, u8"错误", u8"用户名或密码错误！");
        ui->LineEditPwd->clear();   // 登陆失败时弹框提示并清空密码栏、将焦点设置在密码栏上;
        if (ui->LineEditUser->text().isEmpty())
            ui->LineEditUser->setFocus();
        else
            ui->LineEditPwd->setFocus();
    }
}

void DialogLogin::on_btnRg_clicked()
{
    dlgReg->show();
}

void DialogLogin::closeEvent(QCloseEvent *event)
{
    settings::write_log_md5();
    event->accept();
}
