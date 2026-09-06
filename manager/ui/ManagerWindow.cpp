#include "manager/ui/ManagerWindow.h"

#include "manager/ui/ObservationWindow.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

ManagerWindow::ManagerWindow(QWidget* parent) : QMainWindow(parent), database_("monitor_manager.sqlite") {
    QString error;
    if (!database_.open(&error)) QMessageBox::critical(this, "Database", error);
    connect(&server_, &ManagerServer::agentChanged, this, &ManagerWindow::refreshAgent);
    connect(&server_, &ManagerServer::logMessage, this, &ManagerWindow::appendLog);
    connect(&server_, &ManagerServer::controlApproved, this, &ManagerWindow::controlApproval);
    buildLoginUi();
    setWindowTitle("Remote Monitor - Login");
    resize(1100, 700);
}

void ManagerWindow::buildLoginUi() {
    auto* panel = new QWidget(this);
    auto* layout = new QVBoxLayout(panel);
    auto* form = new QFormLayout;
    username_ = new QLineEdit("admin");
    password_ = new QLineEdit("admin");
    password_->setEchoMode(QLineEdit::Password);
    form->addRow("Username", username_);
    form->addRow("Password", password_);
    layout->addStretch();
    layout->addLayout(form);
    auto* button = new QPushButton("Login", panel);
    connect(button, &QPushButton::clicked, this, &ManagerWindow::login);
    layout->addWidget(button);
    loginStatus_ = new QLabel("Default demo account: admin / admin", panel);
    layout->addWidget(loginStatus_);
    layout->addStretch();
    setCentralWidget(panel);
}

void ManagerWindow::login() {
    QString role;
    if (!database_.authenticate(username_->text(), password_->text(), &role)) {
        loginStatus_->setText("Invalid credentials");
        return;
    }
    QString error;
    if (!server_.listen(45454, &error)) {
        QMessageBox::critical(this, "Network", error);
        return;
    }
    database_.recordAudit(username_->text(), "login");
    buildDashboardUi();
    screenshotTimer_.setInterval(1000 / 30);
    connect(&screenshotTimer_, &QTimer::timeout, this, &ManagerWindow::requestScreenshot);
    screenshotTimer_.start();
    setWindowTitle("Remote Monitor - Manager");
}

void ManagerWindow::buildDashboardUi() {
    auto* root = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(root);
    auto* splitter = new QSplitter(Qt::Vertical, root);
    observation_ = new ObservationWindow(this);
    connect(observation_, &ObservationWindow::requestControl,
            this, &ManagerWindow::requestControl);
    connect(observation_, &ObservationWindow::stopControl,
            this, &ManagerWindow::stopControl);
    connect(observation_, &ObservationWindow::mouseEvent,
            this, &ManagerWindow::sendMouse);
    connect(observation_, &ObservationWindow::keyEvent,
            this, &ManagerWindow::sendKey);
    auto* upper = new QSplitter(Qt::Horizontal, splitter);
    agents_ = new QTableWidget(0, 7, upper);
    agents_->setHorizontalHeaderLabels({"Agent", "Host", "IP", "OS", "CPU", "RAM", "Status"});
    agents_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    agents_->setSelectionBehavior(QAbstractItemView::SelectRows);
    agents_->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(agents_, &QTableWidget::itemSelectionChanged, this, &ManagerWindow::selectAgent);
    auto* right = new QWidget(upper);
    auto* rightLayout = new QVBoxLayout(right);
    detail_ = new QLabel("Select an agent");
    rightLayout->addWidget(detail_);
    auto* refresh = new QPushButton("Request screenshot");
    frameRate_ = new QComboBox;
    frameRate_->addItem("30 FPS - HD (1280x720)", 30);
    frameRate_->addItem("60 FPS - HD performance (1280x720)", 60);
    auto* control = new QPushButton("Request control");
    auto* stop = new QPushButton("Stop control");
    command_ = new QComboBox;
    command_->addItems({"get_system_info", "list_processes", "get_disk_info"});
    auto* run = new QPushButton("Run safe command");
    connect(refresh, &QPushButton::clicked, this, &ManagerWindow::requestScreenshot);
    connect(control, &QPushButton::clicked, this, &ManagerWindow::requestControl);
    connect(stop, &QPushButton::clicked, this, &ManagerWindow::stopControl);
    connect(run, &QPushButton::clicked, this, &ManagerWindow::sendSafeCommand);
    connect(frameRate_, &QComboBox::currentIndexChanged, this, [this](int) {
        const int fps = frameRate_->currentData().toInt();
        screenshotTimer_.setInterval(qMax(1, 1000 / qMax(1, fps)));
    });
    rightLayout->addWidget(refresh);
    rightLayout->addWidget(new QLabel("Screen refresh profile"));
    rightLayout->addWidget(frameRate_);
    rightLayout->addWidget(control);
    rightLayout->addWidget(stop);
    rightLayout->addWidget(command_);
    rightLayout->addWidget(run);
    rightLayout->addStretch();
    upper->addWidget(agents_);
    upper->addWidget(right);
    log_ = new QTextEdit(splitter);
    log_->setReadOnly(true);
    splitter->addWidget(upper);
    splitter->addWidget(log_);
    mainLayout->addWidget(splitter);
    setCentralWidget(root);
}

QString ManagerWindow::selectedAgent() const {
    if (!agents_ || agents_->currentRow() < 0) return {};
    auto* item = agents_->item(agents_->currentRow(), 0);
    return item ? item->text() : QString{};
}

void ManagerWindow::refreshAgent(const AgentSnapshot& snapshot) {
    if (!agents_) return;
    int row = -1;
    for (int i = 0; i < agents_->rowCount(); ++i)
        if (agents_->item(i, 0)->text() == snapshot.id) row = i;
    if (row < 0) { row = agents_->rowCount(); agents_->insertRow(row); }
    const QStringList values = {snapshot.id, snapshot.hostname, snapshot.ip, snapshot.os,
                                QString::number(snapshot.cpu, 'f', 1),
                                QString::number(snapshot.memory, 'f', 1),
                                snapshot.online ? "Online" : "Offline"};
    for (int i = 0; i < values.size(); ++i) agents_->setItem(row, i, new QTableWidgetItem(values[i]));
    if (agents_->currentRow() < 0 && snapshot.online) agents_->selectRow(row);
    if (!snapshot.screenshot.isEmpty() && observation_ && snapshot.id == observation_->agentId())
        observation_->setFrame(snapshot.screenshot);
    detail_->setText(QString("Agent: %1\nHost: %2\nCPU: %3%\nRAM: %4%\nStatus: %5")
                         .arg(snapshot.id, snapshot.hostname)
                         .arg(snapshot.cpu, 0, 'f', 1)
                         .arg(snapshot.memory, 0, 'f', 1)
                         .arg(snapshot.online ? "Online" : "Offline"));
}

void ManagerWindow::selectAgent() {
    controlEnabled_ = false;
    const QString id = selectedAgent();
    if (observation_) {
        observation_->setControlEnabled(false);
        observation_->setAgentId(id);
        if (!id.isEmpty()) {
            observation_->show();
            observation_->raise();
            observation_->activateWindow();
        }
    }
}
void ManagerWindow::controlApproval(const QString& agentId, bool approved) {
    if (agentId != selectedAgent()) return;
    controlEnabled_ = approved;
    if (observation_) observation_->setControlEnabled(approved);
    appendLog(approved ? "Agent approved control." : "Agent denied control.");
}
void ManagerWindow::requestScreenshot() {
    const int fps = frameRate_ ? frameRate_->currentData().toInt() : 30;
    const bool performanceMode = fps >= 60;
    server_.requestScreenshot(selectedAgent(), 1280, 720, performanceMode ? 45 : 55);
}
void ManagerWindow::requestControl() {
    const QString id = selectedAgent();
    if (id.isEmpty()) return;
    database_.recordAudit(username_->text(), "control_request", id);
    server_.requestControl(id);
}
void ManagerWindow::stopControl() {
    const QString id = selectedAgent();
    if (id.isEmpty()) return;
    database_.recordAudit(username_->text(), "control_stop", id);
    server_.stopControl(id);
    controlEnabled_ = false;
    if (observation_) observation_->setControlEnabled(false);
}
void ManagerWindow::sendSafeCommand() {
    const QString id = selectedAgent();
    if (id.isEmpty()) return;
    const QString command = command_->currentText();
    database_.recordAudit(username_->text(), "command:" + command, id);
    server_.sendCommand(id, command);
}

void ManagerWindow::sendMouse(const QString& kind, int x, int y, int button) {
    if (!controlEnabled_ || !observation_ || observation_->agentId() != selectedAgent()) return;
    server_.sendControlEvent(selectedAgent(), {{"kind", kind}, {"x", x}, {"y", y}, {"button", button}});
}

void ManagerWindow::sendKey(int key, bool pressed) {
    if (!controlEnabled_ || !observation_ || observation_->agentId() != selectedAgent()) return;
    server_.sendControlEvent(selectedAgent(), {{"kind", "key"}, {"key", key}, {"pressed", pressed}});
}

void ManagerWindow::appendLog(const QString& message) {
    if (log_) log_->append(message);
}
