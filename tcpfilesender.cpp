#include "tcpfilesender.h"

TcpFileSender::TcpFileSender(QWidget *parent)
    : QDialog(parent)
{

    // 原有元件
    loadSize = 1024 * 4;
    totalBytes = 0;
    bytesWritten = 0;
    bytesToWrite = 0;
    clientProgressBar = new QProgressBar;
    clientProgressBar->setTextVisible(true); // 顯示文字
    clientProgressBar->setFormat(QStringLiteral("%p%")); // 進度顯示百分比
    clientProgressBar->setAlignment(Qt::AlignCenter);

    clientStatusLabel = new QLabel(QStringLiteral("客戶端就緒"));
    clientStatusLabel->setAlignment(Qt::AlignCenter); // 居中對齊

    startButton = new QPushButton(QStringLiteral("開始"));
    quitButton = new QPushButton(QStringLiteral("退出"));
    openButton = new QPushButton(QStringLiteral("開檔"));
    startButton->setEnabled(false);

    // 新增 IP 和 Port 欄位
    ipLineEdit = new QLineEdit();
    portLineEdit = new QLineEdit();

    ipLineEdit->setPlaceholderText(tr("例如: 127.0.0.1"));
    portLineEdit->setPlaceholderText(tr("例如: 16998"));
    QRegularExpression ipRegex(R"(^(?:[0-9]{1,3}\.){0,3}[0-9]{0,3}$)");
    ipLineEdit->setValidator(new QRegularExpressionValidator(ipRegex, this));
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    QLabel *ipLabel = new QLabel(QStringLiteral("IP 位址:"));
    QLabel *portLabel = new QLabel(QStringLiteral("Port:"));

    QGroupBox *configGroupBox = new QGroupBox(QStringLiteral("伺服器設定"));
    QGridLayout *configLayout = new QGridLayout;
    configLayout->addWidget(ipLabel, 0, 0);
    configLayout->addWidget(ipLineEdit, 0, 1);
    configLayout->addWidget(portLabel, 1, 0);
    configLayout->addWidget(portLineEdit, 1, 1);
    configGroupBox->setLayout(configLayout);

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(openButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    // 主佈局
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(clientProgressBar);
    mainLayout->addWidget(clientStatusLabel);
    mainLayout->addWidget(configGroupBox);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle(QStringLiteral("檔案傳送器"));
    setFixedSize(400, 300); // 設定固定大小

    // 信號與槽連接
    connect(openButton, &QPushButton::clicked, this, &TcpFileSender::openFile);
    connect(startButton, &QPushButton::clicked, this, &TcpFileSender::start);
    connect(&tcpClient, &QTcpSocket::connected, this, &TcpFileSender::startTransfer);
    connect(&tcpClient, &QTcpSocket::bytesWritten, this, &TcpFileSender::updateClientProgress);
    connect(quitButton, &QPushButton::clicked, this, &TcpFileSender::close);
}


void TcpFileSender::openFile()
{
    fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) startButton->setEnabled(true);
}
void TcpFileSender::start()
{
    QString ipAddress = ipLineEdit->text();
    quint16 port = portLineEdit->text().toUShort();

    if (ipAddress.isEmpty() || port == 0) {
        QMessageBox::warning(this, QStringLiteral("錯誤"), QStringLiteral("請輸入有效的 IP 位址和 Port"));
        return;
    }

    startButton->setEnabled(false);
    bytesWritten = 0;
    clientStatusLabel->setText(QStringLiteral("連接中..."));

    // 嘗試連接到伺服器
    tcpClient.connectToHost(QHostAddress(ipAddress), port);
}

void TcpFileSender::startTransfer()
{
    localFile = new QFile(fileName);
    if (!localFile->open(QFile::ReadOnly))
     {
        QMessageBox::warning(this,QStringLiteral("應用程式"),
                              QStringLiteral("無法讀取 %1:\n%2.").arg(fileName)
                              .arg(localFile->errorString()));
        return;
     }

    totalBytes = localFile->size();
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_6);
    QString currentFile = fileName.right(fileName.size() -
                                         fileName.lastIndexOf("/")-1);
    sendOut <<qint64(0)<<qint64(0)<<currentFile;
    totalBytes += outBlock.size();

    sendOut.device()->seek(0);
    sendOut<<totalBytes<<qint64((outBlock.size()-sizeof(qint64)*2));
    bytesToWrite = totalBytes - tcpClient.write(outBlock);
    clientStatusLabel->setText(QStringLiteral("已連接"));
    qDebug() << currentFile <<totalBytes;
    outBlock.resize(0);
}
void TcpFileSender::updateClientProgress(qint64 numBytes)
{
    bytesWritten += (int) numBytes;
    if(bytesToWrite > 0)
    {
        outBlock = localFile->read(qMin(bytesToWrite, loadSize));
        bytesToWrite -= (int) tcpClient.write(outBlock);
        outBlock.resize(0);
    }else
    {
        localFile->close();
    }

    clientProgressBar->setMaximum(totalBytes);
    clientProgressBar->setValue(bytesWritten);
    clientStatusLabel->setText(QStringLiteral("已傳送 %1 Bytes").arg(bytesWritten));
}

TcpFileSender::~TcpFileSender()
{

}



