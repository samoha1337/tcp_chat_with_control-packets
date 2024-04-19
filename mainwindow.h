#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QDebug>

#include <socket_manager.h>
#include <message_sender.h>


#define MIN_PORT 0
#define MAX_PORT 65536


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_rBtnServer_clicked();//выбор режима сервера

    void on_rBtnClient_clicked();//выбор режима клиента

    void on_pBtnCreate_clicked();//создать

    void on_pBtnAddresses_clicked();//вывод адрессов компа

    void on_pBtnClear_clicked();//очистка виджетов

    void on_pBtnConnect_clicked();//подключиться

    void on_pBtnSend_clicked();//кнопка отправить



    void onServerConnection(); //подключение к серверу

    void onSocketDisconnected(); //отключение от сервера

    void onSocketReadyRead(); //считывание сообщений

    void deleteServer(); //удаление сервера

    void deleteSockets();//удаление сокетов


    void addNickToTable(const QString &nickname);//ф-ция для вывода ников

    bool isCorrectIP(const QString &ipAddress);//ф-ция на проверку ip


private:
    Ui::MainWindow *ui;

    QTcpServer *_server = nullptr; // переменная, отвечающая за сервер

    QString _nick;

    socket_manager sManager;
    message_sender mSender;

};

#endif // MAINWINDOW_H
