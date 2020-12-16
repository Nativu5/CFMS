#include "dialoghistory.h"
#include "ui_dialoghistory.h"
#include "settings.h"

DialogHistory::DialogHistory(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogHistory)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint&~Qt::WindowContextHelpButtonHint&~Qt::WindowStaysOnTopHint);
    ui->historyBrowser->setSource("file:///" + settings::curPath + "/" + "journal.log", QTextDocument::MarkdownResource);
}

DialogHistory::~DialogHistory()
{
    delete ui;
}

void DialogHistory::reload()
{
    ui->historyBrowser->reload();
}
