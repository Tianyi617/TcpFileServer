#include "tcpfileserver.h"
#define tr QStringLiteral

TcpFileServer::TcpFileServer(QWidget *parent)
    : QDialog(parent),
    totalBytes(0),
    byteReceived(0),
    fileNameSize(0),
    localFile(nullptr),
    tcpServerConnection(nullptr)
{
    // 新增 IP 和 Port 的輸入框
    ipLabel = new QLabel(tr("IP 地址:"));
    ipLineEdit = new QLineEdit();
    ipLineEdit->setPlaceholderText(tr("例如: 127.0.0.1"));

    portLabel = new QLabel(tr("Port:"));
    portLineEdit = new QLineEdit();
    portLineEdit->setPlaceholderText(tr("例如: 16998"));

    serverProgressBar = new QProgressBar;
    serverStatusLabel = new QLabel(tr("伺服器端就緒"));
    startButton = new QPushButton(tr("接收"));
    quitButton = new QPushButton(tr("退出"));
    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    // 排版
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *ipPortLayout = new QHBoxLayout;
    ipPortLayout->addWidget(ipLabel);
    ipPortLayout->addWidget(ipLineEdit);
    ipPortLayout->addWidget(portLabel);
    ipPortLayout->addWidget(portLineEdit);

    mainLayout->addLayout(ipPortLayout);
    mainLayout->addWidget(serverProgressBar);
    mainLayout->addWidget(serverStatusLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("接收檔案"));

    // 連接信號和槽
    connect(startButton, &QPushButton::clicked, this, &TcpFileServer::start);
    connect(quitButton, &QPushButton::clicked, this, &TcpFileServer::close);
    connect(&tcpServer, &QTcpServer::newConnection, this, &TcpFileServer::acceptConnection);
    connect(&tcpServer, QOverload<QAbstractSocket::SocketError>::of(&QTcpServer::acceptError),
            this, &TcpFileServer::displayError);
}

TcpFileServer::~TcpFileServer()
{
    if (localFile) {
        delete localFile;
    }
}

void TcpFileServer::start()
{
    startButton->setEnabled(false);
    byteReceived = 0;
    fileNameSize = 0;

    // 獲取使用者輸入的 Port
    bool ok;
    quint16 port = portLineEdit->text().toUShort(&ok);
    if (!ok || port == 0) {
        QMessageBox::warning(this, tr("錯誤"), tr("請輸入有效的 Port！"));
        startButton->setEnabled(true);
        return;
    }

    // 顯示輸入的 IP（僅作展示）
    QString ipAddress = ipLineEdit->text().isEmpty() ? tr("0.0.0.0") : ipLineEdit->text();

    while (!tcpServer.isListening() &&
           !tcpServer.listen(QHostAddress::AnyIPv4, port)) {
        QMessageBox::StandardButton ret = QMessageBox::critical(
            this, tr("迴圈"),
            tr("無法啟動伺服器: %1.").arg(tcpServer.errorString()),
            QMessageBox::Retry | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) {
            startButton->setEnabled(true);
            return;
        }
    }

    serverStatusLabel->setText(tr("正在監聽 %1:%2").arg(ipAddress).arg(port));
}

void TcpFileServer::acceptConnection()
{
    tcpServerConnection = tcpServer.nextPendingConnection();
    connect(tcpServerConnection, &QTcpSocket::readyRead, this, &TcpFileServer::updateServerProgress);
    connect(tcpServerConnection, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &TcpFileServer::displayError);
    serverStatusLabel->setText(tr("已接受連線"));
    tcpServer.close(); // 暫停接受客戶端連線
}

void TcpFileServer::updateServerProgress()
{
    QDataStream in(tcpServerConnection);
    in.setVersion(QDataStream::Qt_4_6);

    // 更新接收進度
    if (byteReceived <= sizeof(qint64) * 2) {
        if ((fileNameSize == 0) && (tcpServerConnection->bytesAvailable() >= sizeof(qint64) * 2)) {
            in >> totalBytes >> fileNameSize;
            byteReceived += sizeof(qint64) * 2;
        }
        if ((fileNameSize != 0) && (tcpServerConnection->bytesAvailable() >= fileNameSize)) {
            in >> fileName;
            byteReceived += fileNameSize;
            localFile = new QFile(fileName);
            if (!localFile->open(QFile::WriteOnly)) {
                QMessageBox::warning(this, tr("錯誤"),
                                     tr("無法讀取檔案 %1：\n%2.").arg(fileName).arg(localFile->errorString()));
                return;
            }
        } else {
            return;
        }
    }

    if (byteReceived < totalBytes) {
        byteReceived += tcpServerConnection->bytesAvailable();
        inBlock = tcpServerConnection->readAll();
        localFile->write(inBlock);
        inBlock.resize(0);
    }

    serverProgressBar->setMaximum(totalBytes);
    serverProgressBar->setValue(byteReceived);
    serverStatusLabel->setText(tr("已接收 %1 Bytes").arg(byteReceived));

    if (byteReceived == totalBytes) {
        tcpServerConnection->close();
        startButton->setEnabled(true);
        localFile->close();
        start();
    }
}

void TcpFileServer::displayError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QMessageBox::information(this, tr("網絡錯誤"),
                             tr("產生如下錯誤: %1.").arg(tcpServerConnection->errorString()));
    tcpServerConnection->close();
    serverProgressBar->reset();
    serverStatusLabel->setText(tr("伺服器就緒"));
    startButton->setEnabled(true);
}
