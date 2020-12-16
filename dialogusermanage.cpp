#include "dialogusermanage.h"
#include "ui_dialogusermanage.h"

DialogUserManage::DialogUserManage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogUserManage)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint&~Qt::WindowContextHelpButtonHint&~Qt::WindowStaysOnTopHint);
    setFixedSize(this->width(), this->height());
}

DialogUserManage::~DialogUserManage()
{
    delete ui;
}

void DialogUserManage::on_lineUser_editingFinished()
{
    QJsonObject cur;
    foreach(QJsonValue i, settings::userlist)
    {
        cur = i.toObject();
        if (cur["id"] == ui->lineUser->text())
        {
            ui->comboBoxPermisson->setCurrentIndex(cur["permission"].toInt());
            return;
        }
    }
}

void DialogUserManage::on_bntModify_clicked()
{
    QString username = ui->lineUser->text().trimmed();
    if (username == settings::username)
    {
        QMessageBox::critical(this, "错误", "超级管理员无法修改自身权限，若要修改需切换至其他超级管理员账户操作。");
        return;
    }
    QJsonObject cur;
    bool flag = false;
    for (auto i = settings::userlist.begin(); i != settings::userlist.end(); i++)
    {
        cur = i->toObject();
        if (cur["id"] == username)
        {
            cur["permission"] = ui->comboBoxPermisson->currentIndex();
            *i = cur;
            flag = true;
            break;
        }
    }
    if (flag)
    {
        settings::configObj["users"] = settings::userlist;
        QJsonDocument updatedDoc(settings::configObj);
        settings::write_json(updatedDoc, "config.dat");
        QMessageBox::information(this, "提示", "账户 " + username + " 的权限已修改为：" + ui->comboBoxPermisson->currentText());
        settings::write_log(QString("%1 **权限被修改为: %2** ").arg(username, ui->comboBoxPermisson->currentText()));  // 日志记录
    }
    else
        QMessageBox::critical(this, "错误", "未查询到被操作用户的记录！");
}

void DialogUserManage::on_lineUser_returnPressed()
{
    on_lineUser_editingFinished();
}
