#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QString>
#include <QProcess>
#include <QMessageBox>
#include <QComboBox>
#include <QFile>
#include <QStandardPaths>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(const QString file1, const QString file2, QWidget *parent = nullptr);
    ~Widget();

private slots:

    void press_start();

    void press_stop();

    void killProcess(QProcess *process);

    void on_rb_broadcast_clicked();

    void on_rb_unicast_clicked();

    void on_cb_iflist_activated(int index);

    void on_txt_sender_editingFinished();

    void on_txt_target_editingFinished();

    void on_btn_send_clicked();

    bool check_address_text_box();

    void on_btn_stop_clicked();

    void handleProcessError(QProcess::ProcessError error);

    void killAllProcess();

    void on_btn_clearlog_clicked();

private:
    //function
    void append_interface_list();
    bool copyFileFromAssets(const QString fileName, const QString dir, QFile::Permissions permissions);

private:
    Ui::Widget *ui;

    bool packet_mode;
    QString dev,target_ip, sender_ip;
    QString interface_list, arp_file;
    QString destinationDir;
    QList<QProcess*> processList;
};
#endif // WIDGET_H
