#ifndef DIALOGUSERMANAGE_H
#define DIALOGUSERMANAGE_H

#include <QDialog>
#include <settings.h>

namespace Ui {
class DialogUserManage;
}

class DialogUserManage : public QDialog
{
    Q_OBJECT

public:
    explicit DialogUserManage(QWidget *parent = nullptr);
    ~DialogUserManage();

private slots:
    void on_lineUser_editingFinished();

    void on_bntModify_clicked();

    void on_lineUser_returnPressed();

private:
    Ui::DialogUserManage *ui;
};

#endif // DIALOGUSERMANAGE_H
