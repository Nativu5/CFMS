#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogabout.h"
#include "settings.h"
#include "dialogusermanage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    dlgAbout = new DialogAbout(this);
    dlgHistory = new DialogHistory(this);
    dlgUserManage = new DialogUserManage(this);
    statusLblGrt = new QLabel;
    statusLblGrt->setMinimumSize(150, 20);
    statusLblGrt->setText("欢迎使用班级财务管理系统！"); // 设置状态栏文本
    ui->statusBar->addPermanentWidget(statusLblGrt);
    ui->menuBar->addAction("关于", this, SLOT(on_menuAbout_clicked()));
    init_pie_chart();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index != 0)
    {
        ui->toolBar->hide();
        if (tabModel->isDirty())
            QMessageBox::information(this, "提示", "当前有未保存的更改，统计数据可能并不准确。");
    }
    else
        ui->toolBar->show();
}

void MainWindow::on_menuAbout_clicked()
{
    dlgAbout->show();
}

void MainWindow::open_table()
{
    // 打开数据表格
    tabModel = new QSqlTableModel(this, DB);
    statisticModel = new QSqlTableModel(this, DB);
    tabModel->setTable("Record");
    statisticModel->setTable("Statistic");
    tabModel->setSort(tabModel->fieldIndex("Date"), Qt::AscendingOrder); // 设置日期排序（不立刻应用）
    tabModel->setEditStrategy(QSqlTableModel::OnManualSubmit); // 手动提交数据库更改；
    if (!tabModel->select() || !statisticModel->select())
    {
        QMessageBox::critical(this, "错误", "打开数据表失败！\n" + tabModel->lastError().text() + "\n" + statisticModel->lastError().text());
        return;
    }
    // 设置表头
    tabModel->setHeaderData(tabModel->fieldIndex("id"), Qt::Horizontal, "ID");
    tabModel->setHeaderData(tabModel->fieldIndex("date"), Qt::Horizontal, "时间");
    tabModel->setHeaderData(tabModel->fieldIndex("user"), Qt::Horizontal, "使用者");
    tabModel->setHeaderData(tabModel->fieldIndex("usage"), Qt::Horizontal, "用途");
    tabModel->setHeaderData(tabModel->fieldIndex("amount"), Qt::Horizontal, "金额");
    tabModel->setHeaderData(tabModel->fieldIndex("balance"), Qt::Horizontal, "余额");
    statisticModel->setHeaderData(statisticModel->fieldIndex("title"), Qt::Horizontal, "项目");
    statisticModel->setHeaderData(statisticModel->fieldIndex("data"), Qt::Horizontal, "值");
    // 设置selection
    theSelection = new QItemSelectionModel(tabModel);
    connect(theSelection, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(on_currentRowChanged(QModelIndex, QModelIndex)));
    connect(theSelection, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(on_selectionChanged(QItemSelection, QItemSelection)));
    // 调整表格view显示
    ui->tableData->setModel(tabModel);
    ui->tableData->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 设置允许多选
    ui->tableData->setSelectionBehavior(QAbstractItemView::SelectRows);     // 仅按行选择
    ui->tableData->setSelectionModel(theSelection);
    ui->tableData->resizeRowsToContents();
    ui->tableData->resizeColumnsToContents();
    ui->tableGeneral->setModel(statisticModel);
    ui->tableGeneral->setColumnHidden(0, true);  // 隐藏列ID
    ui->tableGeneral->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableGeneral->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableGeneral->resizeRowsToContents();
    ui->tableGeneral->resizeColumnsToContents();
    // 设置数据库数据与控件的映射
    dataMapper = new QDataWidgetMapper();
    dataMapper->setModel(tabModel);
    dataMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    dataMapper->addMapping(ui->SBMoney, tabModel->fieldIndex("amount"));
    dataMapper->addMapping(ui->DTEUseTime, tabModel->fieldIndex("date"));
    dataMapper->addMapping(ui->inUser, tabModel->fieldIndex("user"));
    dataMapper->addMapping(ui->CBUsage, tabModel->fieldIndex("usage"));
    // 找到最大ID(往后编号)
    int total = tabModel->rowCount();
    maxID = 0;
    for (int i = 0; i < total; i++)
        maxID = qMax(tabModel->record(i).value("id").toInt(), maxID);
    // 依据读取的数据库填充已知用途
    fill_usage();
    // 设置ui
    ui->actAdd->setEnabled(true);
    ui->actExp->setEnabled(true);
    ui->actSav->setEnabled(true);
    ui->tabWidget->setEnabled(true);      // 数据显示和分析页面启用
    ui->tabWidgetOpt->setEnabled(true);   // 数据操作页面启用
    ui->tabFilter->setEnabled(true);      // 数据过滤搜索页面启用
    ui->bntApply->setEnabled(true);
    ui->tabEdit->setEnabled(false);       // 默认未选中，故数据编辑区域禁用
}

void MainWindow::on_currentRowChanged(const QModelIndex &current, const QModelIndex &previous) //当前选择行改变
{
    Q_UNUSED(previous);
    dataMapper->setCurrentIndex(current.row());
    // 如果是 ID=0 的初始记录，不可编辑用途信息；
    ui->CBUsage->setEnabled(tabModel->record(current.row()).value("id").toInt());
}

void MainWindow::on_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
    if (tabModel->isDirty())
        ui->statusBar->showMessage("当前有未保存的更改");
    else
        ui->statusBar->clearMessage();
    if (theSelection->selectedRows().size() != 1)  // 如果选中了多行或未选中，禁用编辑区，清空输入区域的值;
    {
        ui->tabEdit->setEnabled(false);
        on_btnDefault_clicked();
    }
    else
        ui->tabEdit->setEnabled(true);
    if (theSelection->selectedRows().size() == 0)  // 如果当前未选中
        ui->actDel->setEnabled(false);
    else
        ui->actDel->setEnabled(true);     // 如果当前选择位置有效;
}

void MainWindow::on_actAdd_triggered()
{
    if(settings::curUserPermisson == 0)
    {
        QMessageBox::critical(this, "错误", "用户无权限！");
        return;
    }
    tabModel->insertRow(tabModel->rowCount(), QModelIndex());  // 第一个参数取值从0开始;
    QModelIndex curIndex = tabModel->index(tabModel->rowCount() - 1, 0);
    theSelection->clearSelection();       // 清除选择
    theSelection->setCurrentIndex(curIndex, QItemSelectionModel::Rows);
    ui->CBUsage->setEnabled(true);        // 由于直接设置selection的坐标不会发射RowChange信号，手动设置启用输入框;
    tabModel->setData(tabModel->index(curIndex.row(), 0), ++maxID);
    ui->tabWidgetOpt->setCurrentIndex(0); // 自动回到编辑页
    settings::unSubmitedLog.append(settings::username + " **添加记录** `ID: " + QString::number(maxID - 1) + '`');
}

void MainWindow::on_actOpen_triggered()
{
    QString aFile = QFileDialog::getOpenFileName(this, "请选择数据库文件",  settings::curPath, "SQLite文件(*.db *.db3)");
    if (aFile.isEmpty())
    {
        QMessageBox::critical(this, "错误", "未选择有效的数据库文件！");
        return;
    }
    if (DB.isOpen())  // 将已经打开的数据库关闭
    {
        if(tabModel->isDirty() && QMessageBox::warning(this, "关闭现有数据库", "尚有未保存的更改，舍弃所有更改并打开新的数据库？", QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
            return;
        settings::unSubmitedLog.append(settings::username + " 关闭数据库 `" + DB.databaseName() + "`"); // 日志记录
        settings::submit_log();
        DB.close();
    }
    DB = QSqlDatabase::addDatabase("QSQLITE");
    DB.setDatabaseName(aFile);
    if (!DB.open())
    {
        QMessageBox::critical(this, "错误", "数据库打开失败！错误信息:\n" + DB.lastError().text());
        return;
    }
    open_table();  // 打开数据表
    settings::write_log(settings::username + " 打开数据库 `" + aFile + "`"); // 日志记录
}

void MainWindow::on_btnUpdate_clicked()
{

    if(settings::curUserPermisson == 0)
    {
        QMessageBox::critical(this, "错误", "用户无权限！");
        return;
    }
    if (ui->inUser->text() == "" || ui->CBUsage->currentText() == "")
    {
        QMessageBox::critical(this, "错误", "输入的信息不完整，请重试。");
        return;
    }
    if (tabModel->record(dataMapper->currentIndex()).value("id") != 0 && ui->CBUsage->currentText() == "初始班费")
    {
        QMessageBox::critical(this, "错误", "\"初始班费\"是为初始记录保留的关键字，不能使用。");
        return;
    }
    dataMapper->submit();
    if (tabModel->isDirty())
        ui->statusBar->showMessage("当前有未保存的更改");
}

void MainWindow::on_actDel_triggered()  // 单选/多选删除
{
    if(settings::curUserPermisson == 0)
    {
        QMessageBox::critical(this, "错误", "用户无权限！");
        return;
    }
    QModelIndexList rowList = theSelection->selectedRows();
    std::sort(rowList.begin(), rowList.end(),
              [](const QModelIndex &a, const QModelIndex &b){return a.row() > b.row();}); // 别问，问就lambda表达式
    foreach (QModelIndex index, rowList)
        tabModel->removeRow(index.row());
    settings::unSubmitedLog.append(settings::username + " **删除数据** 共 `"  + QString::number(rowList.size()) + "` 条");
}

bool MainWindow::update_Rec()  // 提交数据库更改
{
    tabModel->database().transaction();  // 开始事务操作
    if (tabModel->submitAll())
        tabModel->database().commit();   // 提交
    else
    {
        tabModel->database().rollback(); // 回滚
        QMessageBox::critical(this, "错误", QString("保存失败，数据库错误: \n%1").arg(tabModel->lastError().text()));
        return false;
    }
    return true;
}

void MainWindow::on_actSav_triggered()   // 计算余额和统计数据并提交数据库更改
{
    if(settings::curUserPermisson == 0)
    {
        QMessageBox::critical(this, "错误", "用户无权限！");
        return;
    }
    if (!tabModel->isDirty())            // 没有更改就不提交
        return;
    for (int i = tabModel->rowCount() - 1; i >= 0; i--)  // 判断记录合法性，防止空记录插入数据库；
    {
        QSqlRecord rec = tabModel->record(i);
        for (int j = 0; j < 6; j++)      // 逐 field 检查有无值
        {
            if (rec.value(j).isNull() && rec.fieldName(j) != "balance") //由于余额自动计算填写，故不检查
            {
                QMessageBox::warning(this, "警告", QString("部分记录为空，无法保存。"));
                return;
            }
        }
    }
    if (!update_Rec())  // 必须先保存一次才能获得排序后的结果
        return;
    calc_balance();                    // 检查无误后尝试计算并更新余额
    calc_statistic();                  // 记录表填写好之后计算统计数据
    on_bntApply_clicked();             // 把保存之前的过滤排序状态恢复
    fill_usage();                      // 依据读取的数据库填充已知用途 （注意执行顺序！）
    ui->statusBar->clearMessage();     // 清除状态栏提示的“未保存”信息
    ui->tableData->resizeRowsToContents();
    ui->tableData->resizeColumnsToContents();
    ui->tableGeneral->resizeRowsToContents();
    ui->tableGeneral->resizeColumnsToContents();
    settings::unSubmitedLog.append(settings::username + " **提交数据库更改** ");
    settings::submit_log();
}

void MainWindow::on_btnDefault_clicked() // 将编辑区域设置为默认内容
{
    ui->CBUsage->clearEditText();
    ui->SBMoney->clear();
    ui->inUser->clear();
    QString curDatetime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm");     // 默认返回毫秒级精度，
    ui->DTEUseTime->setDateTime(QDateTime::fromString(curDatetime, "yyyy/MM/dd hh:mm")); // 通过转换字符串以期与时间输入组件精度相符；
}

void MainWindow::on_btnCancel_clicked()
{
    theSelection->clear();
    on_btnDefault_clicked();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (DB.isOpen() && tabModel->isDirty())
    {
        if(QMessageBox::warning(this, "退出", "尚有未保存的更改，舍弃所有更改并退出？", QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
        {
            event->ignore();
            return;
        }
    }
    settings::submit_log();    // 退出之前提交未写入的日志
    settings::write_log(settings::username + " 退出系统 ");  // 日志记录
    settings::write_log_md5();
    event->accept();
}

void MainWindow::on_actHistory_triggered()  // 显示操作记录
{
    dlgHistory->reload();
    dlgHistory->show();
}

void MainWindow::calc_balance()  // 计算余额并更新
{
    QSqlQuery query("SELECT id, amount, balance FROM Record ORDER BY date ASC", DB);
    QSqlQuery update(DB);
    double balance = 0;
    while (query.next())
    {
        QString id = query.value(0).toString();
        double amount = query.value(1).toDouble();
        balance += amount;
        if (id == "0")
            continue;
        update.exec(QString("UPDATE Record SET Balance = %1 WHERE id = %2 ").arg(QString::number(balance, 10, 2), id));
    }
    tabModel->select();  // 刷新(其实这句没用)
}

void MainWindow::on_actExp_triggered()  // 导出csv文件
{
    if (tabModel->isDirty())  // 检测有未保存数据
    {
        QMessageBox::critical(this, "错误", "尚有更改未保存，无法导出。");
        return;
    }
    tabModel->select();
    QString csvFileName = QFileDialog::getSaveFileName(this, "导出文件", settings::curPath, "csv文件(*.csv);;所有文件(*.*)");
    if (csvFileName.isEmpty())
        return;
    QFile csvFile(csvFileName);
    csvFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream outStream(&csvFile);
    outStream.setAutoDetectUnicode(true);  // 防止中文乱码
    QStringList textCol;
    outStream << QString("ID, 金额, 余额, 使用者, 用途, 使用时间") << endl;  // 输出表头；
    for (int i = 0; i < tabModel->rowCount(); i++)
    {
        for (int j = 0; j < tabModel->columnCount(); j++)
        {
            QString text = tabModel->data(tabModel->index(i, j)).toString();
            textCol.append(j == 5 ? text.replace("T", " ") : text);  // 最后一列是日期时间，特殊处理；
        }
        outStream << textCol.join(", ") << endl;  // 逗号为列分隔符；
        textCol.clear();  // 每行输出完毕后清空；
    }
    settings::write_log(settings::username + "导出表格");
    csvFile.close();
}

void MainWindow::on_actCalc_triggered()
{
    QProcess process(this);
    process.startDetached("calc");
}

void MainWindow::on_actNotepad_triggered()
{
    QProcess process(this);
    process.startDetached("notepad");
}

void MainWindow::fill_usage()
{
    QStringList usage;            // 主要用来方便去重
    int comboUsageNewIndex = 0;   // 记忆原来的选择
    QSqlQuery query("SELECT usage FROM Record ORDER BY date ASC", DB);
    while (query.next())
    {
        QString use = query.value(0).toString();
        if (use != "初始班费" && !usage.contains(use))     // 去重和非法用途
        {
            usage.append(use);
            if (use == ui->comboBoxUsage->currentText())
                comboUsageNewIndex = usage.size();        // 由于本来第一行的“选择用途”不在usage里，无需 - 1;
        }
    }
    ui->CBUsage->clear();          // 初始化两个 ComboBox
    ui->comboBoxUsage->clear();
    ui->CBUsage->addItems(usage);
    ui->comboBoxUsage->addItem("选择用途");
    ui->comboBoxUsage->addItems(usage);
    ui->comboBoxUsage->setCurrentIndex(comboUsageNewIndex);  // 尝试恢复
}

void MainWindow::on_bntDefaultFilter_clicked()
{
    ui->comboBoxSort->setCurrentIndex(0);
    ui->comboBoxUsage->setCurrentIndex(0);
    ui->comboBoxBalance->setCurrentIndex(0);
    ui->comboBoxSearch->setCurrentIndex(0);
    ui->lineSearch->clear();
    tabModel->setFilter("");
    tabModel->setSort(tabModel->fieldIndex("Date"), Qt::AscendingOrder); // 设置日期排序
    if (tabModel->isDirty())
    {
        if (QMessageBox::question(this, "提示", "原始表格已被更改，保存并显示更新后的表格?", QMessageBox::Yes|QMessageBox::Cancel) == QMessageBox::Yes)
            on_actSav_triggered();
        else
            return;
    }
    tabModel->select();                                                  // 应用
}

void MainWindow::on_bntApply_clicked()  // 应用过滤条件
{
    if (tabModel->isDirty())
    {
        if (QMessageBox::question(this, "提示", "原始表格已被更改，保存并进行所选操作?", QMessageBox::Yes|QMessageBox::Cancel) == QMessageBox::Yes)
            on_actSav_triggered();
        else
            return;
    }
    QStringList filters;
    if (ui->comboBoxSearch->currentIndex() != 0 && ui->lineSearch->text().isEmpty() == false)    // 搜索
        filters.append(QString(" %1 LIKE '%%2%' ").arg(DB.record("Record").field(ui->comboBoxSearch->currentIndex() - 1).name(), ui->lineSearch->text()));
    if (ui->comboBoxBalance->currentIndex() != 0)   // 过滤收支
        filters.append(QString(" amount %1 0 ").arg((ui->comboBoxBalance->currentText() == "收入") ? ">=" : "<"));
    if (ui->comboBoxUsage->currentIndex() != 0)     // 过滤用途
        filters.append(QString(" usage = '%1' ").arg(ui->comboBoxUsage->currentText()));
    if (!filters.isEmpty())
        tabModel->setFilter(filters.join("AND"));
    else
        tabModel->setFilter("");
    if (ui->comboBoxSort->currentIndex() != 0)      // 排序
        tabModel->setSort(ui->comboBoxSort->currentIndex(), Qt::AscendingOrder);
    else
        tabModel->setSort(tabModel->fieldIndex("Date"), Qt::AscendingOrder);
    tabModel->select();
}

void MainWindow::calc_statistic()  // 统计数据并写入统计表
{
    double amountInAvg = 0, amountOutAvg = 0, amountInMax = 0, amountOutMax = 0;
    double balanceMax = -1e9, balanceMin = 1e9;
    int cntIn = 0, cntOut = 0;
    QStringList usageOutMost, usageInMost;
    QSqlQuery query("SELECT amount, balance, usage FROM Record", DB);
    while (query.next())
    {
        double amount = query.value(0).toDouble(), balance = query.value(1).toDouble();
        QString usage = query.value(2).toString();     
        if (amount >= 0)  // 收入
        {
            if (amount > amountInMax)
            {
                amountInMax = amount;
                usageInMost.clear();
                usageInMost.append(usage);
            }
            else if (amount == amountInMax && !usageInMost.contains(usage))
                usageInMost.append(usage);
            amountInAvg += amount;
            cntIn++;
        }
        else              // 支出
        {
            if (amount <= amountOutMax)
            {
                amountOutMax = amount;
                usageOutMost.clear();
                usageOutMost.append(usage);
            }
            else if (amount == amountOutMax && !usageOutMost.contains(usage))
                usageOutMost.append(usage);
            amountOutAvg += amount;
            cntOut++;
        }
        if (usage == "初始班费")
        {
            balanceMax = qMax(balanceMax, amount);
            balanceMin = qMin(balanceMin, amount);
        }
        else
        {
            balanceMax = qMax(balanceMax, balance);
            balanceMin = qMin(balanceMin, balance);
        }
    }
    amountInAvg /= cntIn;
    amountOutAvg /= cntOut;
    QSqlQuery update(DB);
    update.exec(QString("UPDATE Statistic SET data = \"%2\" WHERE title = \"%1\"").arg("收入平均值", QString::number(amountInAvg, 10, 2)));
    update.exec(QString("UPDATE Statistic SET data = \"%2\" WHERE title = \"%1\"").arg("单笔收入最大值", QString::number(amountInMax, 10, 2)));
    update.exec(QString("UPDATE Statistic SET data = \"%2\" WHERE title = \"%1\"").arg("单笔收入最大来源", usageInMost.join(" ,")));
    update.exec(QString("UPDATE Statistic SET data = \"%2\" WHERE title = \"%1\"").arg("支出平均值", QString::number(abs(amountOutAvg), 10, 2)));
    update.exec(QString("UPDATE Statistic SET data = \"%2\" WHERE title = \"%1\"").arg("单笔支出最大值", QString::number(abs(amountOutMax), 10, 2)));
    update.exec(QString("UPDATE Statistic SET data = \"%2\" WHERE title = \"%1\"").arg("单笔支出最大去向", usageOutMost.join(" ,")));
    update.exec(QString("UPDATE Statistic SET data = \"%2\" WHERE title = \"%1\"").arg("余额峰值", QString::number(balanceMax, 10, 2)));
    update.exec(QString("UPDATE Statistic SET data = \"%2\" WHERE title = \"%1\"").arg("余额谷值", QString::number(balanceMin, 10, 2)));
    statisticModel->select();
}

void MainWindow::init_pie_chart()
{
    QChart *chart = new QChart();
    chart->setTitle("请在上方选择要生成的统计图");
    chart->setAnimationOptions(QChart::AllAnimations);
    ui->chartView->setChart(chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::build_pie_chart()
{
    QChart *chart = ui->chartView->chart();
    chart->removeAllSeries();
    QPieSeries *series = new QPieSeries();
    std::map<QString, qreal> amountIn, amountOut;
    for (int i = tabModel->rowCount() - 1; i >= 0; i--)
    {
        double amount = tabModel->record(i).value("amount").toReal();
        QString usage = tabModel->record(i).value("usage").toString();
        if (ui->comboBoxType->currentIndex() == 0 && amount > 0)
            amountIn[usage] += amount;
        else if (ui->comboBoxType->currentIndex() == 1 && amount < 0)
            amountOut[usage] += amount;
    }
    if (ui->comboBoxType->currentIndex() == 0)
    {
        std::pair<QString, qreal> i;
        foreach(i, amountIn)
            series->append(i.first, i.second);
    }
    else if (ui->comboBoxType->currentIndex() == 1)
    {
        std::pair<QString, qreal> i;
        foreach(i, amountOut)
            series->append(i.first, i.second);
    }
    if (series->isEmpty())
    {
        QMessageBox::critical(this, "错误", "所选类型可用数据过少，无法生成统计图!");
        return;
    }
    QPieSlice *slice;
    for (int i = series->count() - 1; i >= 0; i--)
    {
        slice = series->slices().at(i);
        slice->setLabel(slice->label() + QString(": %1元, %2%").arg(QString::number(slice->value(), 10, 2), QString::number(slice->percentage() * 100, 10, 2)));  //设置分块的标签
        connect(slice, SIGNAL(hovered(bool)), this, SLOT(on_PieSliceHighlight(bool)));
    }
    series->setLabelsVisible(true);
    chart->addSeries(series);
    chart->setTitle(ui->comboBoxType->currentText());
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);
}

void MainWindow::on_PieSliceHighlight(bool show)  // 设置动画
{
    QPieSlice *slice;
    slice = (QPieSlice *)sender();   //鼠标移入、移出时触发hovered()信号，动态设置setExploded()效果
    slice->setExploded(show);
}

void MainWindow::on_bntChartDisplay_clicked()
{
    build_pie_chart();
    ui->chartView->chart()->setTheme(QChart::ChartTheme(ui->comboBoxTheme->currentIndex()));
}

void MainWindow::on_actUser_triggered()  //用户权限管理
{
    if(settings::curUserPermisson != 2)
    {
        QMessageBox::critical(this, "错误", "仅超级管理员可进入用户权限管理系统！");
        return;
    }
    dlgUserManage->show();
}

void MainWindow::on_actNew_triggered()
{
    if(settings::curUserPermisson == 0)
    {
        QMessageBox::critical(this, "错误", "用户无权限！");
        return;
    }
    if (DB.isOpen())  // 将已经打开的数据库关闭
    {
        if(tabModel->isDirty() && QMessageBox::warning(this, "关闭现有数据库", "尚有未保存的更改，舍弃所有更改并新建新的数据库？", QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
            return;
        settings::unSubmitedLog.append(settings::username + " 关闭数据库 `" + DB.databaseName() + "`"); // 日志记录
        settings::submit_log();
        DB.close();
    }
    QString newDBName = QFileDialog::getSaveFileName(this, "新建文件", settings::curPath, "SQLite文件(*.db);;所有文件(*.*)");
    if (newDBName.isEmpty())
    {
        QMessageBox::critical(this, "错误", "未选择有效的保存位置！");
        return;
    }
    DB = QSqlDatabase::addDatabase("QSQLITE");
    DB.setDatabaseName(newDBName);
    if (!DB.open())
    {
        QMessageBox::critical(this, "错误", "新建数据库失败！错误信息:\n" + DB.lastError().text());
        return;
    }
    QSqlQuery query(DB);
    QString sqlOpStr =
            "CREATE TABLE [Record]( \
            [id] INT, \
            [amount] DOUBLE, \
            [balance] DOUBLE, \
            [user] [VARCHAR(20)], \
            [usage] VARCHAR, \
            [date] DATETIME); \
            CREATE TABLE [Statistic]( \
            [id] INT, \
            [title] VARCHAR, \
            [data] VARCHAR); \
            INSERT INTO Record (id, amount, user, usage, date) VALUES (0, 0, \"全班同学\", \"初始班费\", \"1926-08-17T00:00:00.000\");\
            INSERT INTO Statistic (Title) VALUES (\"收入平均值\"); \
            INSERT INTO Statistic (Title) VALUES (\"单笔收入最大值\"); \
            INSERT INTO Statistic (Title) VALUES (\"单笔收入最大来源\"); \
            INSERT INTO Statistic (Title) VALUES (\"支出平均值\"); \
            INSERT INTO Statistic (Title) VALUES (\"单笔支出最大值\"); \
            INSERT INTO Statistic (Title) VALUES (\"单笔支出最大去向\"); \
            INSERT INTO Statistic (Title) VALUES (\"余额峰值\"); \
            INSERT INTO Statistic (Title) VALUES (\"余额谷值\")";
    QStringList sqlOps = sqlOpStr.split(';');
    foreach (QString op, sqlOps)
    {
        if (!query.exec(op))
        {
            QMessageBox::critical(this, "错误", "数据库错误:\n" + query.lastError().text());
            return;
        }
    }
    open_table();
    settings::write_log(settings::username + " 新建数据库 `" + newDBName + "`"); // 日志记录
}
