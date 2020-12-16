#include "settings.h"
#include "mainwindow.h"
#include "dialoglogin.h"
#include "dialogabout.h"
#include "sql.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    settings::settings_init(); // 软件启动时必须使用进行初始化获取必要的信息;
    MainWindow w;
    DialogLogin dlgLogin;
    if (dlgLogin.exec() == QDialog::Accepted)
        w.show();
    else
        exit(0);
    return a.exec();
}
