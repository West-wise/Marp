#include <QIcon>
#include <QDir>
#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(const QString file1, const QString file2,const QString file3,const QString file4, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , interface_list(file2)
    , arp_file(file1)
    , get_ip_info_file(file3)
    , scan_file(file4)
    , destinationDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/")
    , imgDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/img")
    , model(new QStandardItemModel(this))
{
    ui->setupUi(this);
    scan_process = nullptr;

    QScreen *screen = QGuiApplication::primaryScreen();
    this->resize(screen->availableSize().width(),screen->availableSize().height());
    this->showFullScreen();
    packet_mode = true;
    append_interface_list();
    // 파일 복사
    if (!copyFileFromAssets(file1, destinationDir, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner)) {
        qWarning() << "Failed to copy" << file1 << "to" << destinationDir;
    }
    if (!copyFileFromAssets(file2, destinationDir, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner)) {
        qWarning() << "Failed to copy" << file2 << "to" << destinationDir;
    }
    if (!copyFileFromAssets(file3, destinationDir, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner)) {
        qWarning() << "Failed to copy" << file3 << "to" << destinationDir;
    }
    if (!copyFileFromAssets(file4, destinationDir, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner)) {
        qWarning() << "Failed to copy" << file3 << "to" << destinationDir;
    }
    if (!copyImgFileFromAssets("antenna.png", imgDir, QFile::ReadOwner | QFile::WriteOwner)) {
        qWarning() << "Failed to copy" << "antenna.png" << "to" << imgDir;
    }
    if (!copyImgFileFromAssets("antenna_stop.png", imgDir, QFile::ReadOwner | QFile::WriteOwner)) {
        qWarning() << "Failed to copy" << "antenna_stop.png" << "to" << imgDir;
    }

    setupTableView();
}

Widget::~Widget()
{
    delete ui;
}

bool Widget::copyFileFromAssets(QString fileName, QString dir, QFile::Permissions permissions) {

    QString srcFileName = ":/assets/" + fileName;
    QString dstFileName = dir + fileName;

    QFile srcFile(srcFileName);
    if (!srcFile.exists()) {
        qWarning() << "Source file does not exist:" << srcFileName;
        return false;
    }
    QFile dstFile(dstFileName);
    if (dstFile.exists()) {
        if (!dstFile.remove()) {
            qWarning() << "Failed to remove existing file:" << dstFileName;
            return false;
        }
    }
    if (!srcFile.copy(dstFileName)) {
        qWarning() << "Failed to copy file from" << srcFileName << "to" << dstFileName;
        return false;
    }
    if (!dstFile.setPermissions(permissions)) {
        qWarning() << "Failed to set permissions on:" << dstFileName;
        return false;
    }
    return true;
}

bool Widget::copyImgFileFromAssets(QString fileName, QString dir, QFile::Permissions permissions) {
    QString srcFileName = ":/assets/img/" + fileName;
    QString dstFileName = dir + "/" + fileName;

    QDir targetDir(dir);
    if (!targetDir.exists()) { // 대상 디렉토리가 존재하지 않으면
        if (!targetDir.mkpath(dir)) { // 디렉토리를 생성하려고 시도
            qWarning() << "Failed to create directory:" << dir;
            return false; // 실패한 경우 함수 종료
        }
    }

    QFile srcFile(srcFileName);
    if (!srcFile.exists()) {
        qWarning() << "Source file does not exist:" << srcFileName;
        return false;
    }
    QFile dstFile(dstFileName);
    if (dstFile.exists()) {
        if (!dstFile.remove()) {
            qWarning() << "Failed to remove existing file:" << dstFileName;
            return false;
        }
    }
    if (!srcFile.copy(dstFileName)) {
        qWarning() << "Failed to copy file from" << srcFileName << "to" << dstFileName;
        return false;
    }
    if (!dstFile.setPermissions(permissions)) {
        qWarning() << "Failed to set permissions on:" << dstFileName;
        return false;
    }
    return true;
}

void Widget::on_rb_broadcast_clicked()
{
    packet_mode = false;
}

void Widget::on_rb_unicast_clicked()
{
    packet_mode = true;
}

void Widget::append_interface_list()
{
    QString interface_list_FilePath = destinationDir + interface_list;
    QProcess interface_list_process;
    QFile file(interface_list_FilePath);

    if (!file.exists()) {
        qDebug() << "File does not exist at: " << interface_list_FilePath;
        return;
    }

    interface_list_process.start("su", QStringList() << "-c" << interface_list_FilePath);
    if (!interface_list_process.waitForStarted(1000)) {
        qDebug() << "Failed to start process";
        return;
    } else {
        qDebug() << "Process started successfully";
    }

    interface_list_process.waitForFinished(3000);
    QString output = interface_list_process.readAllStandardOutput();
    QString errorOutput = interface_list_process.readAllStandardError();

    if (!output.isEmpty()) {
        qDebug() << "Standard Output: " << output;
    }

    if (!errorOutput.isEmpty()) {
        qDebug() << "Error Output: " << errorOutput;
    }

    // QComboBox 초기화
    ui->cb_iflist->clear();

    // 파일에서 읽어들인 결과를 QComboBox에 추가
    QStringList lines = output.split('\n');
    for (const QString &line : lines) {
        if (!line.trimmed().isEmpty()) {
            QStringList parts = line.split(':');
            if (parts.size() == 2) {
                QString interface_name = parts[1].trimmed();
                ui->cb_iflist->addItem(interface_name);
            }
        }
    }

    dev = lines[0].split(':')[1].trimmed();
    qDebug() << "dev : " << dev;
    get_ip_info(dev);
}

void Widget::get_ip_info(QString ifname)
{
    QString FilePath = destinationDir + get_ip_info_file;
    QProcess address_info_process;
    QFile file(FilePath);

    QStringList run_with_argv;

    if (!file.exists()) {
        qDebug() << "./myinfo File does not exist at: " << FilePath;
        return;
    }

    // ifname을 프로세스의 인자로 추가
    run_with_argv << ifname;

    QStringList arguments;
    arguments << "-c" << FilePath + " " + run_with_argv.join(" ");
    address_info_process.start("su", arguments);
    if (!address_info_process.waitForStarted(1000)) {
        qDebug() << "myinfo run error ";
        return;
    }

    address_info_process.waitForFinished(1000);
    QString output = address_info_process.readAllStandardOutput();
    QString errorOutput = address_info_process.readAllStandardError();

    // myinfo 출력 파싱
    if (!output.isEmpty()) {
        QStringList lines = output.split('\n');
        for (const QString &line : lines) {
            QString trimmedLine = line.trimmed();
            if (trimmedLine.startsWith("Gateway_IP")) {
                gatewayIp = trimmedLine.split(' ')[1].trimmed();
                ui->GIp->setText(gatewayIp);
            } else if (trimmedLine.startsWith("MY_IP")) {
                myIp = trimmedLine.split(' ')[1].trimmed();
                ui->MIp->setText(myIp);
            } else if (trimmedLine.startsWith("MY_MAC")) {
                myMac = trimmedLine.split(' ')[1].trimmed();
                ui->Mmac->setText(myMac);
            }
        }
    }

    if (!errorOutput.isEmpty()) {
        qDebug() << "Address Info Error Output: " << errorOutput;
    }
}

void Widget::on_cb_iflist_activated(int index)
{
    dev = ui->cb_iflist->currentText().trimmed();
    get_ip_info(dev);
}

void Widget::killProcess(QProcess *process)
{
    if (process->state() == QProcess::Running) {
        process->terminate();
        if (!process->waitForFinished(5000)) {
            process->kill();
        }
        qDebug() << "Process terminated.";

    } else {
        qDebug() << "No process running.";
    }
}

void Widget::killAllProcess()
{
    for (QProcess *process : qAsConst(processList)) {
        killProcess(process);
    }
}

void Widget::on_btn_scan_toggled(bool checked) {
    if (checked) {
        ui->btn_scan->setIcon(QIcon(":/assets/img/antenna.png")); // on 아이콘으로 변경

        // // 테이블 내용 초기화
        model->removeRows(0, model->rowCount());
        ui->tl_DeviceList->reset();

        startScanProcess();
    } else {
        ui->btn_scan->setIcon(QIcon(":/assets/img/antenna_stop.png")); // off 아이콘으로 변경
        if (scan_process->state() == QProcess::Running) {
            scan_process->terminate();
            scan_process->waitForFinished();
        }
    }
}

void Widget::setupTableView() {
    // 컬럼 헤더 설정
    model->setColumnCount(3);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("IP"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("MAC"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Run"));

    ui->tl_DeviceList->setModel(model);

    // 컬럼 너비 설정
    QHeaderView *header = ui->tl_DeviceList->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);

    // 테이블의 너비를 기준으로 비율 계산
    int tableWidth = ui->tl_DeviceList->width();
    int ipColumnWidth = tableWidth * 3 / 8;
    int macColumnWidth = tableWidth * 3 / 8;
    int runColumnWidth = tableWidth * 2 / 8;

    ui->tl_DeviceList->setColumnWidth(0, ipColumnWidth);
    ui->tl_DeviceList->setColumnWidth(1, macColumnWidth);
    ui->tl_DeviceList->setColumnWidth(2, runColumnWidth);

    ui->tl_DeviceList->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // 행 높이 조정
    ui->tl_DeviceList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tl_DeviceList->setSelectionMode(QAbstractItemView::SingleSelection);

    // 테이블 크기가 변경될 때마다 열 너비를 다시 설정
    connect(ui->tl_DeviceList->horizontalHeader(), &QHeaderView::sectionResized, this, &Widget::adjustTableColumns);
}

void Widget::adjustTableColumns() {
    int tableWidth = ui->tl_DeviceList->width();
    int ipColumnWidth = tableWidth * 3 / 8;
    int macColumnWidth = tableWidth * 3 / 8;
    int runColumnWidth = tableWidth * 2 / 8;

    ui->tl_DeviceList->setColumnWidth(0, ipColumnWidth);
    ui->tl_DeviceList->setColumnWidth(1, macColumnWidth);
    ui->tl_DeviceList->setColumnWidth(2, runColumnWidth);
}
void Widget::handleProcessError(QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart:
        qDebug() << "The process failed to start.";
        break;
    case QProcess::Crashed:
        qDebug() << "The process crashed.";
        break;
    case QProcess::Timedout:
        qDebug() << "The process timed out.";
        break;
    case QProcess::WriteError:
        qDebug() << "An error occurred when attempting to write to the process.";
        break;
    case QProcess::ReadError:
        qDebug() << "An error occurred when attempting to read from the process.";
        break;
    default:
        qDebug() << "An unknown error occurred.";
    }
}

void Widget::handleScanProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        qDebug() << "Scan process finished successfully.";
    } else {
        qDebug() << "Scan process failed with exit code:" << exitCode;
    }
    scan_process->deleteLater(); // 메모리 누수 방지를 위해 프로세스 삭제
    ui->btn_scan->setChecked(false);
}

void Widget::handleScanProcessOutput() {
    QString output = QString(scan_process->readAllStandardOutput());
    QString errorOutput = QString(scan_process->readAllStandardError());

    qDebug() << "Standard Output:" << output;
    qDebug() << "Error Output:" << errorOutput;

    QStringList scanResults = output.split("\n", Qt::SkipEmptyParts);
    for (const QString &result : scanResults) {
        QStringList parts = result.split(" ", Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            QString ip = parts[0];
            QString mac = parts[1];

            QList<QStandardItem *> row;
            QStandardItem *ipItem = new QStandardItem(ip);
            ipItem->setTextAlignment(Qt::AlignCenter); // IP 항목 가운데 정렬
            QStandardItem *macItem = new QStandardItem(mac);
            macItem->setTextAlignment(Qt::AlignCenter); // MAC 항목 가운데 정렬
            row.append(ipItem);
            row.append(macItem);
            model->appendRow(row);

            QPushButton *runButton = new QPushButton("R");
            runButton->setCheckable(true);
            runButton->setFixedSize(24, 24); // 고정 크기 설정
            runButton->setParent(ui->tl_DeviceList); // 부모 설정하여 메모리 누수 방지

            // 버튼 클릭 시 IP를 슬롯 함수에 전달하도록 연결
            connect(runButton, &QPushButton::clicked, this, [this, runButton, ip]() {
                if (runButton->isChecked()) {
                    // 버튼이 체크된 경우 프로세스를 시작
                    send_arp(ip);
                } else {
                    // 버튼이 체크 해제된 경우 프로세스를 종료
                    if (processMap.contains(runButton)) {
                        QProcess *process = processMap.value(runButton);
                        process->terminate();
                        process->waitForFinished();
                        delete process;
                        processMap.remove(runButton);
                    }
                }
            });

            ui->tl_DeviceList->setIndexWidget(model->index(model->rowCount() - 1, 2), runButton); // 인덱스 수정
        }
    }
    ui->tl_DeviceList->update();
}

void Widget::send_arp(const QString sender_ip)
{
    QString objectFilePath = destinationDir + arp_file;
    QStringList address_args;
    QFile file(objectFilePath);

    if (!file.exists()) {
        qDebug() << "File does not exist at: " << objectFilePath;
        return;
    }
    if(packet_mode){
        address_args << dev << sender_ip << gatewayIp;
    } else {
        address_args << dev << gatewayIp;
    }

    QProcess *arp_process = new QProcess(this); // no destructor but, not create new object using global var
    connect(arp_process, &QProcess::errorOccurred, this, &Widget::handleProcessError);
    // connect(ui->btn_stop, &QPushButton::clicked, this, &Widget::on_btn_stop_clicked);

    QStringList arguments;
    arguments << "-c" << objectFilePath + " " + address_args.join(" ");
    arp_process->start("su", arguments);
    if (!arp_process->waitForStarted(5000)) {
        qDebug() << "Failed to start process";
        return;
    } else {
        qDebug() << "Process started successfully.";
        qDebug() << arp_process->arguments();
    }
    processList.append(arp_process);

    // 버튼과 프로세스를 맵에 저장
    QPushButton *senderButton = qobject_cast<QPushButton*>(sender());
    if (senderButton) {
        processMap.insert(senderButton, arp_process);
    }
}



void Widget::startScanProcess() {
    QString scanFilePath = destinationDir + scan_file;
    QStringList address_args;
    QFile file(scanFilePath);
    if (!file.exists()) {
        qDebug() << "Scan File does not exist at: " << scanFilePath;
        return;
    }
    // address_args 정의 및 값 할당
    address_args << scanFilePath << dev << gatewayIp << myIp << myMac;

    if (scan_process && scan_process->state() == QProcess::Running) {
        scan_process->terminate();
        scan_process->waitForFinished();
    }

    scan_process = new QProcess(this);

    connect(scan_process, &QProcess::errorOccurred, this, &Widget::handleProcessError);
    connect(scan_process, &QProcess::readyReadStandardOutput, this, &Widget::handleScanProcessOutput);
    connect(scan_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &Widget::handleScanProcessFinished);

    QString command = "su";
    QStringList arguments;
    arguments << "-c" << address_args.join(" ");

    qDebug() << "Command to be executed: " << command << arguments.join(" "); // 명령어 디버깅 출력

    scan_process->start(command, arguments);
    if (!scan_process->waitForStarted(1000)) {
        qDebug() << "Scan process run Failed";
    } else {
        qDebug() << "Scan Process Started Successfully.";
        qDebug() << "Arguments:" << scan_process->arguments().join(" ");
    }
}
