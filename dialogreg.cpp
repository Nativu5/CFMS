#include "dialogreg.h"
#include "ui_dialogreg.h"
#include "settings.h"

DialogReg::DialogReg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogReg)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint&~Qt::WindowContextHelpButtonHint);
    setFixedSize(this->width(), this->height());    // 设置窗口不显示帮助按钮、并固定窗口大小;
    ui->lineEditCon->setEchoMode(QLineEdit::Password);  // 设置密码输入框为密码输入模式（不显示明文）;
    ui->lineEditPwd->setEchoMode(QLineEdit::Password);
    ui->lblUsercount->setText("现有用户数：" + QString("%1").arg(settings::usercount));
    mode = 0;
}

DialogReg::~DialogReg()
{
    delete ui;
}

void DialogReg::on_bntModify_clicked()
{
    if (mode == 0) // 修改密码；
    {
        ui->bntModify->setText("返回注册");
        ui->lblConfir->setText("新密码");
        ui->lblPwd->setText("旧密码");
        mode = 1;
    }
    else           // 注册用户；
    {
         ui->bntModify->setText("修改密码");
         ui->lblConfir->setText("确认密码");
         ui->lblPwd->setText("密码");
         mode = 0;
    }
}

void DialogReg::modify_password()
{
    QString username = ui->lineEditUser->text().trimmed();
    QString md5OldPwd = QString(QCryptographicHash::hash(ui->lineEditPwd->text().trimmed().toLatin1(), QCryptographicHash::Md5).toHex());
    QString md5NewPwd = QString(QCryptographicHash::hash(ui->lineEditCon->text().trimmed().toLatin1(), QCryptographicHash::Md5).toHex());
    bool flag = 0;
    QJsonObject cur;
    for (auto i = settings::userlist.begin(); i != settings::userlist.end(); i++)
    {
        cur = i->toObject();
        if (cur["id"] == username && cur["password"] == md5OldPwd)
        {
            if (!settings::check_password_legal(ui->lineEditCon->text().trimmed()))
            {
                ui->lineEditCon->clear();
                QMessageBox::critical(this, u8"错误", u8"您输入的新密码不合法。请重试。");
                return;
            }
            cur["password"] = md5NewPwd;
            *i = cur;
            flag = 1;
            break;
        }
    }
    if (flag)
    {
        settings::configObj["users"] = settings::userlist;
        QJsonDocument updatedDoc(settings::configObj);
        settings::write_json(updatedDoc, "config.dat");
        QMessageBox::information(this, u8"提示", u8"您的密码已更改为：\n" + ui->lineEditCon->text());
        settings::write_log(username + " **修改密码**");  // 日志记录
    }
    else  // 查找不到对应的用户信息
        QMessageBox::critical(this, u8"错误", u8"您输入的用户名与密码不匹配。请重试。");
    ui->lineEditPwd->clear();
    ui->lineEditCon->clear();
}

void DialogReg::register_newuser()
{
    QString newPwd = ui->lineEditPwd->text().trimmed();
    QString md5Pwd(QCryptographicHash::hash(newPwd.toLatin1(), QCryptographicHash::Md5).toHex());
    QString username = ui->lineEditUser->text().trimmed();
    if (newPwd != ui->lineEditCon->text().trimmed())
    {
        ui->lineEditCon->clear();
        QMessageBox::critical(this, u8"错误", u8"您两次输入的密码不一致。请重试。");
        return;
    }
    if (!settings::check_username_legal(username))
    {
        ui->lineEditUser->clear();
        QMessageBox::critical(this, u8"错误", u8"您输入的用户名不合法。请重试。");
        return;
    }
    if (!settings::check_password_legal(newPwd))
    {
        ui->lineEditPwd->clear();
        ui->lineEditCon->clear();
        QMessageBox::critical(this, u8"错误", u8"您输入的密码不合法。请重试。");
        return;
    }
    QJsonObject cur;
    foreach(QJsonValue i, settings::userlist)
    {
        cur = i.toObject();
        if (cur["id"] == username)
        {
            ui->lineEditUser->clear();
            ui->lineEditPwd->clear();
            ui->lineEditCon->clear();
            QMessageBox::critical(this, u8"错误", u8"此用户名已被注册。请重试。");
            return;
        }
    }
    QJsonObject newUser
    {
        {"id", username},
        {"password", md5Pwd},
        {"permission", 0}
    };
    if (settings::usercount == 0) // 第一位注册者获得最高权限；
        newUser["permission"] = 2;
    settings::userlist.append(newUser);
    settings::configObj["usercount"] = ++settings::usercount; // 更新用户数目；
    settings::configObj["users"] = settings::userlist;
    QJsonDocument updatedDoc(settings::configObj);
    settings::write_json(updatedDoc, "config.dat");
    ui->lineEditUser->clear();
    ui->lineEditPwd->clear();
    ui->lineEditCon->clear();
    settings::write_log(username + " **账户注册**" );  // 日志记录
    QMessageBox::information(this, u8"提示", u8"你注册的用户信息：\n用户名：" + username + u8"\n密码：" + newPwd);
    ui->lblUsercount->setText("现有用户数：" + QString("%1").arg(settings::usercount)); // 更新显示的用户数量;
}

void DialogReg::on_bntReg_clicked()
{
    if (mode)
        DialogReg::modify_password();
    else
        DialogReg::register_newuser();
}
