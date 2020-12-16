#ifndef DIALOGHISTORY_H
#define DIALOGHISTORY_H

#include <QDialog>

namespace Ui {
class DialogHistory;
}

class DialogHistory : public QDialog
{
    Q_OBJECT

public:
    explicit DialogHistory(QWidget *parent = nullptr);
    ~DialogHistory();
    void reload();

private:
    Ui::DialogHistory *ui;
};

#endif // DIALOGHISTORY_H
