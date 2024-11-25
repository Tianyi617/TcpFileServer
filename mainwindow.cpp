#include "mainwindow.h"
#include "tcpfilesender.h"
#include "tcpfileserver.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    tabWidget(new QTabWidget(this)),
    fileSender(new TcpFileSender(this)),
    fileServer(new TcpFileServer(this))
{

    // 設定樣式表
    setStyleSheet(R"(
        QMainWindow {
            background-color: #F0F4F8;
        }
        QTabWidget::pane {
            border: 1px solid #D6D9DD;
            background: #FFFFFF;
            border-radius: 8px;
        }
        QTabBar::tab {
            background: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #FFFFFF, stop: 1 #E6EAF0
            );
            color: #004B97;
            padding: 8px 15px;
            margin: 2px;
            border: 1px solid #D6D9DD;
            border-radius: 8px;
        }
        QTabBar::tab:selected {
            background: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #D6E6FF, stop: 1 #B3D8FF
            );
            color: #003B73;
            font-weight: bold;
            border: 1px solid #9FC2F0;
        }
        QPushButton {
            background-color: #0078D7;
            color: #FFFFFF;
            border: none;
            padding: 8px 15px;
            border-radius: 5px;
        }
        QPushButton:hover {
            background-color: #005BB5;
            box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.2);
        }
        QPushButton:pressed {
            background-color: #004B97;
        }
        QLabel {
            font-size: 14px;
            color: #333333;
        }
        QProgressBar {
            text-align: center;
            color: #004B97;
            border: 1px solid #D6D9DD;
            border-radius: 5px;
            background: #F2F5F9;
        }
        QProgressBar::chunk {
            background: qlineargradient(
                x1: 0, y1: 0, x2: 1, y2: 0,
                stop: 0 #4CAF50, stop: 1 #81C784
            );
            border-radius: 5px;
        }
        QLineEdit {
            border: 1px solid #D6D9DD;
            border-radius: 5px;
            padding: 5px;
            background: #FFFFFF;
            color: #333333;
        }
        QLineEdit:focus {
            border: 1px solid #0078D7;
        }
    )");

    // 將子功能整合到標籤頁中
    tabWidget->addTab(fileSender, tr("檔案傳送器"));
    tabWidget->addTab(fileServer, tr("檔案接收器"));
    tabWidget->setStyleSheet("QTabWidget { margin: 10px; }");
    tabWidget->setTabPosition(QTabWidget::North);
    tabWidget->setDocumentMode(true);
    tabWidget->setTabShape(QTabWidget::Rounded);

    // 設定主窗口的中心元件為標籤頁
    setCentralWidget(tabWidget);

    // 設定主窗口標題與大小
    setWindowTitle(tr("檔案傳輸系統"));
    resize(500, 400);
}

MainWindow::~MainWindow() {
    // 清理指標物件
    delete fileSender;
    delete fileServer;
    delete tabWidget;
}
