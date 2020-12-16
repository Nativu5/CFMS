#ifndef DIALOGLOGIN_H
#define DIALOGLOGIN_H

#include <QDialog>
#include <QJsonArray>
#include <QMessageBox>
#include <QCloseEvent>
#include "dialogreg.h"

namespace Ui {
class DialogLogin;
}

class DialogLogin : public QDialog
{
    Q_OBJECT

public:
    explicit DialogLogin(QWidget *parent = nullptr);
    ~DialogLogin();

private slots:
    void on_btnLogin_clicked();
    void on_btnRg_clicked();

private:
    Ui::DialogLogin *ui;
    DialogReg *dlgReg;
    bool verify_user_info(const QString inputUsr, const QString inputPwd);

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event);
};

#endif // DIALOGLOGIN_H
