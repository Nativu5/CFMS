#include "settings.h"
namespace settings
{
    int curUserPermisson, usercount;
    QVector<QString> unSubmitedLog;
    QString curPath, username;
    QJsonArray userlist;
    QJsonObject configObj;
    const QString KEY("23oSY4IXbIaTjNem");
    const QString IV("S1UwUus0JoP688ZW");

    void throwErr(QString errmsg)
    {
        QMessageBox::critical(NULL, "错误", errmsg);
        exit(0);
    }
    void get_path()
    {
        curPath = QCoreApplication::applicationDirPath();
    }
    bool check_file(const QString filename)
    {
        if (!QFile::exists(curPath + "/" + filename))
            return 0;
        return 1;
    }
    void settings_init()
    {
        get_path();
        curUserPermisson = 0;
        usercount = -1;
        if (check_file("config.dat"))  // 只要文件不存在就新建并初始化；
        {
            get_config();
            if (!check_log_md5())
                throwErr("日志文件被篡改或删除，请注意数据安全!");
            return;
        }
        QString initJsonStr = "{\"usercount\": 0,\"users\": [], \"lastLogMd5\": \"D41D8CD98F00B204E9800998ECF8427E\"}"; // 构建初始Json
        QJsonDocument jsonDoc = QJsonDocument::fromJson(initJsonStr.toLocal8Bit().data());
        write_json(jsonDoc, "config.dat");
        settings_init();
    }
    void write_json(QJsonDocument &jsondoc, const QString filename)
    {
        QFile f(curPath + "/" + filename);
        f.open(QFile::WriteOnly);
        f.write(encrypt(jsondoc.toJson()));
        f.close();
    }
    QJsonDocument read_json(const QString filename)
    {
        QJsonParseError parseJsonErr;
        QFile f(curPath + "/" + filename);
        f.open(QFile::ReadOnly);
        QByteArray jsonData = decrypt(f.readAll());
        f.close();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseJsonErr);
        if (parseJsonErr.error != QJsonParseError::NoError || jsonDoc.isEmpty())
            throwErr();
        return jsonDoc;
    }
    void get_config() // 为 configObj/userlist/usercount 提供初值；
    {
        QJsonDocument configDoc = settings::read_json("config.dat");
        if (!configDoc.isObject())    // 最外层应为对象, 如果不是直接报错；
            throwErr();
        configObj = configDoc.object(); // 检测 usercount 合法性，不合法直接报错
        if (!configObj["usercount"].isDouble() || configObj["usercount"].toInt() < 0)
            throwErr();
        usercount = configObj["usercount"].toInt();
        if (usercount == 0)
            return;
        if (!configObj["users"].isArray())
            throwErr();
        userlist = configObj["users"].toArray();
        if (userlist.count() != usercount)
            throwErr("注册用户数与实际用户数不相符，软件数据可能被篡改。");
        if (!configObj["lastLogMd5"].isString())
            throwErr("检测到日志文件被篡改，请注意数据安全！");
    }
    bool check_password_legal(const QString password)
    {
        QRegExp userRegExp("^[A-Za-z0-9_-]{6,16}$");
        if (userRegExp.exactMatch(password))
            return true;
        return false;
    }
    bool check_username_legal(const QString username)
    {
        QRegExp userRegExp("^[A-Za-z0-9_-]{3,10}$");
        if (userRegExp.exactMatch(username))
            return true;
        return false;
    }
    void write_log(const QString info)//日志增添
    {
        QString logDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss ddd");
        QFile journal(curPath + "/" + "journal.log");
        journal.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append); // 读写文件
        QTextStream logOutput(&journal);
        logOutput.setCodec("UTF-8");
        logOutput << logDateTime + " " + info << endl << endl;  // 时间 + 内容
        journal.close();
    }
    void submit_log()
    {
        foreach(QString i, unSubmitedLog)
            write_log(i);
        unSubmitedLog.clear();
    }
    bool check_log_md5()
    {
        QFile journal(curPath + "/" + "journal.log");
        journal.open(QIODevice::ReadOnly);
        QString fileMd5 = QCryptographicHash::hash(journal.readAll(), QCryptographicHash::Md5).toHex();
        if (configObj["lastLogMd5"].toString().toUpper() != fileMd5.toUpper())
            return false;
        return true;
    }
    void write_log_md5()
    {
        QFile journal(curPath + "/" + "journal.log");
        journal.open(QIODevice::ReadOnly);
        QString newMd5 = QCryptographicHash::hash(journal.readAll(), QCryptographicHash::Md5).toHex();
        configObj["lastLogMd5"] = newMd5.toUpper();
        QJsonDocument configDoc(configObj);
        write_json(configDoc, "config.dat");
    }
    QByteArray encrypt(QString raw)
    {
        QByteArray hashKey = QCryptographicHash::hash(KEY.toLocal8Bit(), QCryptographicHash::Sha256);
        QByteArray hashIV = QCryptographicHash::hash(IV.toLocal8Bit(), QCryptographicHash::Md5);
        return QAESEncryption::Crypt(QAESEncryption::AES_256, QAESEncryption::CBC, raw.toLocal8Bit(), hashKey, hashIV);
    }
    QByteArray decrypt(QByteArray encoded)
    {
        QByteArray hashKey = QCryptographicHash::hash(KEY.toLocal8Bit(), QCryptographicHash::Sha256);
        QByteArray hashIV = QCryptographicHash::hash(IV.toLocal8Bit(), QCryptographicHash::Md5);
        QByteArray decoded = QAESEncryption::Decrypt(QAESEncryption::AES_256, QAESEncryption::CBC, encoded, hashKey, hashIV);
        return QAESEncryption::RemovePadding(decoded);
    }
} // namespace settings
