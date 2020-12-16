#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QEvent>
#include <QCloseEvent>
#include <QtCharts>
#include <QFileDialog>
#include <QtMath>
#include <QVector>

#include "sql.h"
#include "dialogabout.h"
#include "dialoghistory.h"
#include "dialogusermanage.h"
QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_tabWidget_currentChanged(int index);
    void on_currentRowChanged(const QModelIndex & current, const QModelIndex &previous);
    void on_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void on_menuAbout_clicked();
    void on_actAdd_triggered();
    void on_actOpen_triggered();
    void on_btnUpdate_clicked();
    void on_actDel_triggered();
    void on_actSav_triggered();
    void on_btnCancel_clicked();
    void on_btnDefault_clicked();
    void on_actHistory_triggered();
    void on_actExp_triggered();
    void on_actCalc_triggered();
    void on_actNotepad_triggered();
    void on_bntDefaultFilter_clicked();
    void on_bntApply_clicked();
    void on_PieSliceHighlight(bool show);

    void on_bntChartDisplay_clicked();

    void on_actUser_triggered();

    void on_actNew_triggered();

private:
    Ui::MainWindow *ui;
    QLabel *statusLblGrt;
    DialogAbout *dlgAbout;
    DialogHistory *dlgHistory;
    DialogUserManage *dlgUserManage;
    QSqlDatabase DB;
    QSqlTableModel *tabModel;
    QSqlTableModel *statisticModel;
    QItemSelectionModel *theSelection;
    QDataWidgetMapper *dataMapper;
    int maxID;
    void open_table();
    bool update_Rec();
    void calc_balance();
    void fill_usage();
    void calc_statistic();
    void init_pie_chart();
    void build_pie_chart();

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
