#include "dialogabout.h"
#include "ui_dialogabout.h"

DialogAbout::DialogAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAbout)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint&~Qt::WindowContextHelpButtonHint&~Qt::WindowStaysOnTopHint);
    setFixedSize(this->width(), this->height());
}

DialogAbout::~DialogAbout()
{
    delete ui;
}

void DialogAbout::on_bntOK_clicked()
{
    accept();
}
