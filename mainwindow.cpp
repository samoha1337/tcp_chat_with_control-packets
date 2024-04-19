#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tableWidgetUsers->setColumnCount(1);
    ui->tableWidgetUsers->setHorizontalHeaderLabels({"Подключенные пользователи"});
    ui->tableWidgetUsers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->lnEditNick->hide();
    ui->lbNick->hide();
    ui->lbPort->hide();
    ui->lbAddress->hide();
    ui->lnEditPort->hide();
    ui->lnEditAddress->hide();
    ui->pBtnConnect->hide();
    ui->pBtnCreate->hide();
    ui->pBtnAddresses->hide();
    setWindowTitle("TCP-Chat");
    ui->lnEditMessage->setEnabled(false);
    ui->pBtnSend->setEnabled(false);
    ui->tableWidgetUsers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // Соединение сигнала нажатия на Enter с слотом отправки сообщения
    connect(ui->lnEditMessage, &QLineEdit::returnPressed, this, &MainWindow::on_pBtnSend_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//Кнопки

void MainWindow::on_rBtnServer_clicked() //режим сервера
{
    // Переключаемся в режим сервера
    ui->lbPort->show();
    ui->lbAddress->hide();
    ui->lnEditPort->show();
    ui->lnEditAddress->hide();
    ui->lnEditNick->hide();
    ui->lbNick->hide();

    ui->pBtnConnect->hide();
    ui->pBtnCreate->show();
    ui->pBtnAddresses->show();
}

void MainWindow::on_rBtnClient_clicked()
{
    // Переключаемся в режим клиента
    ui->lbPort->show();
    ui->lbAddress->show();
    ui->lnEditNick->show();
    ui->lbNick->show();
    ui->lnEditPort->show();
    ui->lnEditAddress->show();

    ui->pBtnConnect->show();
    ui->pBtnCreate->hide();
    ui->pBtnAddresses->hide();
}

void MainWindow::on_pBtnCreate_clicked()
{
    if (_server == nullptr) {
        int server_port = ui->lnEditPort->text().toInt();

        if (server_port > MIN_PORT and server_port < MAX_PORT) {
            _server = new QTcpServer(this);
            connect(_server, &QTcpServer::newConnection, this, &MainWindow::onServerConnection);

            if (_server->listen(QHostAddress::Any, quint16(server_port))) {
                ui->txtEditInput->append("Сервер открыт на порте " + QString::number(server_port));
                ui->pBtnCreate->setText("Закрыть");
            } else {
                ui->txtEditInput->append("Не удалось запустить сервер");
                // Чистка сервера
                _server->deleteLater();
                _server = nullptr;
            }
        } else {
            QMessageBox::warning(this, "Внимание!", "Некорректный порт");
        }
    } else {
        deleteServer();

        ui->txtEditInput->append("Сервер закрыт");
        ui->txtEditInput->append("СЕРВЕР НЕ В СЕТИ");
    }
}

void MainWindow::on_pBtnAddresses_clicked() //кнопка вывода адресов
{
    auto addresses = QNetworkInterface::allAddresses();
    ui->txtEditInput->append("Адреса комьютера:");

    for (auto address : addresses)
        if (address.protocol() == QAbstractSocket::IPv4Protocol)
            ui->txtEditInput->append(address.toString().split(':').last());
}

void MainWindow::on_pBtnClear_clicked() //кнопка отчисти текста
{
    ui->txtEditInput->clear();
    ui->txtEditOutput->clear();
}

void MainWindow::on_pBtnConnect_clicked() //кнопка подключения к серверу
{
    _nick = ui->lnEditNick->text();


    if (sManager.getUserCount() == 0) { // если на клиенте нет подключенных пользователей
        int port = ui->lnEditPort->text().toInt();

        if (port > MIN_PORT and port < MAX_PORT) {
            if (_nick.isEmpty()) {
                QMessageBox::warning(this, "Внимание!", "Введите ник");
                return;
            }

            auto ipAddress = ui->lnEditAddress->text();

            if (!isCorrectIP(ipAddress)) {
                QMessageBox::warning(this, "Внимание!", "Некорректный IP - адрес");
                return;
            }

            deleteServer();
            deleteSockets();

            auto socket = new QTcpSocket(this);

            socket->connectToHost(ipAddress, quint16(port));

            if (socket->waitForConnected(5000)) {
                connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onSocketDisconnected);
                connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);

                sManager.addSocket(socket);
                sManager.setNickname(socket, "Сервер");


                auto msg = _nick + "/#@:*&125";
                mSender.sendMessage(socket, msg);

                ui->txtEditInput->append("Подключение установлено");
                ui->txtEditInput->append("СЕРВЕР В СЕТИ");

                ui->txtEditInput->setEnabled(true);
                ui->txtEditOutput->setEnabled(true);
                ui->pBtnSend->setEnabled(true);
                ui->lnEditMessage->setEnabled(true);



                addNickToTable(_nick);

                msg = "Подключился новый клиент: " + _nick;
                ui->txtEditInput->append(msg);

                ui->pBtnConnect->setText("Отключиться");
            } else {
                ui->txtEditInput->append("Ошибка подключения");
                ui->txtEditInput->append("СЕРВЕР НЕ В СЕТИ");
                socket->deleteLater();
            }
        } else {
            QMessageBox::warning(this, "Внимание", "Некорректный порт");
        }
    } else { // отключение от сервера
        deleteServer();
        deleteSockets();
    }
}

void MainWindow::on_pBtnSend_clicked() //кнопка отправки сообщения
{
    auto msg = ui->lnEditMessage->text();

    if (!msg.isEmpty()) {
        if (_server != nullptr)
            msg = "Сервер:" + msg;
        else
            msg = _nick + ":" + msg;

        const auto allSockets = QVector <QTcpSocket *>::fromList(sManager.getAllSockets());
        mSender.sendMessageToClients(allSockets, msg);
        ui->txtEditOutput->append("<p style=\"color:blue\">"+ msg + "</p>");

        ui->lnEditMessage->clear();
        ui->lnEditMessage->setFocus();
    }
}



void MainWindow::onServerConnection() //подключение к серверу
{
    const auto new_socket = _server->nextPendingConnection();
    const auto new_addres_from_socket = new_socket->localAddress().toString().split(":").last();

    if (new_socket != nullptr and !sManager.isAddressBanned(new_addres_from_socket)) {
        connect(new_socket, &QTcpSocket::disconnected, this, &MainWindow::onSocketDisconnected);
        connect(new_socket, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);

        sManager.addSocket(new_socket);
        ui->txtEditInput->append("Клиент " + new_addres_from_socket + " подключился");

        ui->lnEditMessage->setEnabled(true);
        ui->pBtnSend->setEnabled(true);
        ui->txtEditInput->setEnabled(true);

        const auto clientsNicknames = sManager.getAllNicknames();

        mSender.sendAllNicksToNewConnectedSocket(clientsNicknames, new_socket);
    }
}

void MainWindow::onSocketDisconnected()
{
    if (_server == nullptr) {
        ui->txtEditInput->append("Вы отключились от сервера");
        ui->txtEditInput->append("СЕРВЕР НЕ В СЕТИ");

        ui->txtEditOutput->clear();
        ui->pBtnConnect->setText("Подключиться");
        deleteServer();
        deleteSockets();

        ui->lnEditMessage->setEnabled(false);
        ui->pBtnSend->setEnabled(false);
        ui->tableWidgetUsers->clear();
        ui->tableWidgetUsers->setColumnCount(1);
        ui->tableWidgetUsers->setRowCount(0);
        ui->tableWidgetUsers->setHorizontalHeaderLabels({"Подключенные пользователи"});
    } else {
        auto senderSocket = static_cast<QTcpSocket *>(sender());
        auto addres_from_socket = senderSocket->localAddress().toString().split(":").last();

        ui->txtEditInput->append("Клиент " + addres_from_socket + " отключился");
        const auto delete_nick = sManager.getNicknameBySocket(senderSocket);

        sManager.removeSocketBySocket(senderSocket);

        int counter = ui->tableWidgetUsers->rowCount();

        for (int i = 0; i < counter; ++i) {
            auto forDel = ui->tableWidgetUsers->item(i, 0)->text();
            if (forDel == delete_nick)
                ui->tableWidgetUsers->removeRow(i);
        }

        const auto clientsSockets = QVector<QTcpSocket *>::fromList(sManager.getAllSockets());

        const auto message = delete_nick + "::{}6";

        mSender.sendMessageToClients(clientsSockets, message);

        if (sManager.getUserCount() == 0) {
            ui->lnEditMessage->setEnabled(false);
            ui->pBtnSend->setEnabled(false);
        }
    }
}

void MainWindow::onSocketReadyRead() {
    auto senderSocket = static_cast<QTcpSocket *>(sender());
    auto senderAddress = senderSocket->localAddress().toString().split(":").last();

    if (senderSocket != nullptr and !sManager.isAddressBanned(senderAddress)) {
        const auto data = mSender.readMessage(senderSocket);

        const auto msg = QString::fromUtf8(data);

        if (msg == "delete") {
            sManager.addAddressToBanList(senderAddress);
            return;
        } else if (msg == "error") {
            return;
        } else if (msg.contains("/#@:*&125") and _server != nullptr) {
            auto get_nick = msg.split("/#@:*&125");
            auto appended_nick = get_nick[0];

            addNickToTable(appended_nick);

            sManager.setNickname(senderSocket, appended_nick);

            auto allSockets = QVector<QTcpSocket *>::fromList(sManager.getAllSockets());

            mSender.sendMessageNotToSomeone(allSockets, data, senderSocket);
            return;
        } else if (msg.contains("/#@:*&125") and _server == nullptr) {
            auto get_nick = msg.split("/#@:*&125");

            for (auto nick : get_nick)
                if (!nick.isEmpty())
                    addNickToTable(nick);
            return;
        } else if (msg.contains("::{}6") and _server == nullptr) {
            auto get_nick = msg.split("::{}6");
            auto appended_nick = get_nick[0];
            int counter = ui->tableWidgetUsers->rowCount();

            for (int i = 0; i < counter; ++i) {
                auto forDel = ui->tableWidgetUsers->item(i, 0)->text();
                if (forDel == appended_nick)
                    ui->tableWidgetUsers->removeRow(i);
            }
        } else {
            ui->txtEditOutput->append("<p style=\"color:red\">" + msg + "</p>");
        }

        if (_server != nullptr) {
            auto allSockets = QVector<QTcpSocket *>::fromList(sManager.getAllSockets());
            mSender.sendMessageNotToSomeone(allSockets, data, senderSocket);
        }
    }
}


void MainWindow::deleteServer()
{
    if (_server != nullptr) {
        if (_server->isListening()) {
            _server->close();
        }
        _server->deleteLater();
        _server = nullptr;
    }

    ui->pBtnCreate->setText("Создать");
}

void MainWindow::deleteSockets() {
    auto all_sockets = sManager.getAllSockets();

    for (auto socket : all_sockets)
    {
        if (socket->isOpen())
        {
            socket->close();
            socket->disconnectFromHost();
        }
        socket->deleteLater();
    }

    sManager.clearClients();
    ui->pBtnConnect->setText("Подключиться");
}

void MainWindow::addNickToTable(const QString &nickname)
{
    int rowCount = ui->tableWidgetUsers->rowCount();
    ui->tableWidgetUsers->setRowCount(rowCount + 1);
    auto item = new QTableWidgetItem(nickname);
    ui->tableWidgetUsers->setItem(rowCount, 0, item);
}

bool MainWindow::isCorrectIP(const QString &ipAddress)
{
    // Проверяем, соответствует ли IP-адрес шаблону X.X.X.X, где X - число от 0 до 255
    QRegExp ipPattern("^([0-9]{1,3}\\.){3}[0-9]{1,3}$");
    if (ipPattern.exactMatch(ipAddress))
    {
        // Разделяем IP-адрес на компоненты
        auto ipComponents = ipAddress.split(".");
        for (auto ipComponent : ipComponents)
        {
            // Проверяем, что каждая компонента является числом от 0 до 255
            int num = ipComponent.toInt();
            if (num < 0 or num > 255)
                return false;

        }
        return true;
    }
    return false;
}






