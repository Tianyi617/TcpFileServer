#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// MainWindow 類別定義
#include <QMainWindow>
#include <QTabWidget>
#include "tcpfilesender.h"
#include "tcpfileserver.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QTabWidget *tabWidget;
    TcpFileSender *fileSender;
    TcpFileServer *fileServer;
};

#endif // MAINWINDOW_H
