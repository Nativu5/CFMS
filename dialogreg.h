#ifndef DIALOGREG_H
#define DIALOGREG_H

#include <QDialog>

namespace Ui {
class DialogReg;
}

class DialogReg : public QDialog
{
    Q_OBJECT

public:
    explicit DialogReg(QWidget *parent = nullptr);
    ~DialogReg();

private slots:
    void on_bntModify_clicked();
    void on_bntReg_clicked();
    void register_newuser();
    void modify_password();

private:
    Ui::DialogReg *ui;
    bool mode;
};

#endif // DIALOGREG_H
