#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QString>
#include <QProcess>
#include <QMessageBox>
#include <QComboBox>
#include <QFile>
#include <QStandardPaths>
#include <QScreen>
#include <QStandardItemModel>
#include <QTableView>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(const QString file1, const QString file2, const QString file3, const QString file4, QWidget *parent = nullptr);
    ~Widget();

private slots:

    void killProcess(QProcess *process);

    void on_rb_broadcast_clicked();

    void on_rb_unicast_clicked();

    void on_cb_iflist_activated(int index);

    void handleProcessError(QProcess::ProcessError error);

    void handleScanProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void handleScanProcessOutput();

    void killAllProcess();

    void get_ip_info(QString ifname);

    void on_btn_scan_toggled(bool checked);

    void setupTableView();

    void startScanProcess();

    void adjustTableColumns();

    void send_arp(const QString sender_ip);


private:
    //function
    void append_interface_list();
    bool copyFileFromAssets(const QString fileName, const QString dir, QFile::Permissions permissions);
    bool copyImgFileFromAssets(QString fileName, QString dir, QFile::Permissions permissions);

private:
    Ui::Widget *ui;
    QStandardItemModel *model;

    bool packet_mode;
    QString dev,gatewayIp, myIp, myMac;
    QString interface_list, arp_file, get_ip_info_file, scan_file;
    QString destinationDir, imgDir;
    QProcess *scan_process;
    QList<QProcess*> processList;

    QMap<QPushButton*, QProcess*> processMap;
};
#endif // WIDGET_H
