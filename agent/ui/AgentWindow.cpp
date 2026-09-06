#include "agent/ui/AgentWindow.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

AgentWindow::AgentWindow(QWidget* parent) : QMainWindow(parent) {
    auto* root = new QWidget(this);
    auto* layout = new QVBoxLayout(root);
    auto* form = new QFormLayout;
    host_ = new QLineEdit("127.0.0.1");
    form->addRow("Manager host", host_);
    layout->addLayout(form);
    identity_ = new QLabel("Agent ID: " + client_.agentId());
    status_ = new QLabel("Disconnected");
    layout->addWidget(identity_);
    layout->addWidget(status_);
    auto* connectButton = new QPushButton("Connect", root);
    auto* disconnectButton = new QPushButton("Disconnect", root);
    connect(connectButton, &QPushButton::clicked, this, &AgentWindow::connectAgent);
    connect(disconnectButton, &QPushButton::clicked, &client_, &AgentClient::disconnectFromManager);
    layout->addWidget(connectButton);
    layout->addWidget(disconnectButton);
    layout->addWidget(new QLabel("This agent is visible and requires consent before remote control."));
    layout->addStretch();
    setCentralWidget(root);
    setWindowTitle("Remote Monitor - Agent");
    resize(500, 300);
    connect(&client_, &AgentClient::statusChanged, status_, &QLabel::setText);
    connect(&client_, &AgentClient::controlRequested, this, &AgentWindow::askControlConsent);
}

void AgentWindow::connectAgent() { client_.connectToManager(host_->text(), 45454); }

void AgentWindow::askControlConsent() {
    const auto choice = QMessageBox::question(this, "Remote control request",
        "The manager requests mouse and keyboard control for this session. Allow it?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    client_.approveControl(choice == QMessageBox::Yes);
}
