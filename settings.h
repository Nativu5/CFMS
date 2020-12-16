#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFile>
#include <QCoreApplication>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QTime>
#include <QTextStream>
#include <QIODevice>
#include <QByteArray>
#include <QCryptographicHash>
#include "qaesencryption.h"

namespace settings {
    // 函数原型们：
    void throwErr(QString errmsg = "软件配置信息已损坏，请删除根目录下的配置文件后重新打开软件！");
    void get_path();  // 获取可执行文件所在路径；
    void settings_init();  // 初次运行/配置文件损坏时初始化文件；
    bool check_file(const QString filename);     // 检查文件是否存在;
    void write_json(QJsonDocument &jsondoc, const QString filename);  // 按给定文件名保存json；
    QJsonDocument read_json(const QString filename);  // 按指定文件名读取json；
    void get_config();   // 得到存储用户记录的数组；
    bool check_username_legal(const QString username);
    bool check_password_legal(const QString password);
    void write_log(const QString info);   //日志
    void submit_log();
    bool check_log_md5();
    void write_log_md5();
    QByteArray encrypt(QString raw);
    QByteArray decrypt(QByteArray encoded);

    extern int curUserPermisson, usercount;
    extern QString curPath;
    extern QString username;
    extern QJsonObject configObj;
    extern QJsonArray userlist;
    extern QVector<QString> unSubmitedLog;
    extern const QString KEY;
    extern const QString IV;
}

#endif // SETTINGS_H
