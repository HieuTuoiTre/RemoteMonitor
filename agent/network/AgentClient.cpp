#include "agent/network/AgentClient.h"

#include "agent/capture/ScreenCapture.h"
#include "agent/input/InputController.h"
#include "agent/system/SystemInfo.h"

#include <QHostInfo>
#include <QSettings>
#include <QSysInfo>
#include <QUuid>

AgentClient::AgentClient(QObject* parent) : QObject(parent) {
    QSettings settings("PBL4", "RemoteMonitor");
    agentId_ = settings.value("agent_id").toString();
    if (agentId_.isEmpty()) {
        agentId_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
        settings.setValue("agent_id", agentId_);
    }
    connect(&socket_, &QTcpSocket::connected, this, &AgentClient::connected);
    connect(&socket_, &QTcpSocket::disconnected, this, &AgentClient::reconnect);
    connect(&heartbeatTimer_, &QTimer::timeout, this, &AgentClient::heartbeat);
    connect(&reconnectTimer_, &QTimer::timeout, this, &AgentClient::reconnect);
    reconnectTimer_.setInterval(3000);
}

void AgentClient::connectToManager(const QString& host, quint16 port) {
    host_ = host;
    port_ = port;
    manualDisconnect_ = false;
    reconnectTimer_.stop();
    clearFramedSocket();
    socket_.abort();
    socket_.connectToHost(host_, port_);
    emit statusChanged(QString("Connecting to %1:%2").arg(host_).arg(port_));
}

void AgentClient::disconnectFromManager() {
    manualDisconnect_ = true;
    controlEnabled_ = false;
    controlPromptActive_ = false;
    heartbeatTimer_.stop();
    reconnectTimer_.stop();
    clearFramedSocket();
    socket_.disconnectFromHost();
    emit statusChanged("Disconnected");
}

void AgentClient::connected() {
    reconnectTimer_.stop();
    clearFramedSocket();
    framed_ = new monitor::FramedSocket(&socket_, this);
    connect(framed_, &monitor::FramedSocket::messageReceived, this, &AgentClient::messageReceived);
    connect(framed_, &monitor::FramedSocket::protocolError, this, [this](const QString& error) {
        emit statusChanged("Protocol error: " + error);
    });
    framed_->send(monitor::makeMessage("agent_hello", {{"agent_id", agentId_},
        {"hostname", QHostInfo::localHostName()}, {"os", QSysInfo::prettyProductName()}}));
    heartbeatTimer_.start(5000);
    emit statusChanged("Connected");
}

void AgentClient::reconnect() {
    clearFramedSocket();
    heartbeatTimer_.stop();
    controlEnabled_ = false;
    controlPromptActive_ = false;
    if (!manualDisconnect_ && !host_.isEmpty()) {
        if (!reconnectTimer_.isActive()) reconnectTimer_.start();
        if (socket_.state() == QAbstractSocket::UnconnectedState)
            socket_.connectToHost(host_, port_);
        emit statusChanged("Offline - retrying");
        return;
    }
    emit statusChanged("Disconnected");
}

void AgentClient::heartbeat() {
    if (!framed_ || socket_.state() != QAbstractSocket::ConnectedState) return;
    framed_->send(monitor::makeMessage("heartbeat", agent_system::collect()));
}

void AgentClient::sendSystemInfo(const QString& type, const QString& requestId) {
    if (framed_) framed_->send(monitor::makeMessage(type, agent_system::collect(), requestId));
}

void AgentClient::messageReceived(const monitor::Message& message) {
    if (!framed_) return;
    if (message.type == "system_info_request") {
        sendSystemInfo("system_info_response", message.requestId);
    } else if (message.type == "process_list_request") {
        framed_->send(monitor::makeMessage("process_list_response", agent_system::processList(), message.requestId));
    } else if (message.type == "screenshot_request") {
        const int width = qBound(320, message.data.value("max_width").toInt(960), 1920);
        const int height = qBound(180, message.data.value("max_height").toInt(540), 1080);
        const int quality = qBound(20, message.data.value("jpeg_quality").toInt(55), 90);
        const QByteArray screenshot = agent_capture::screenshotJpeg(width, height, quality);
        if (screenshot.isEmpty()) {
            framed_->send(monitor::makeMessage("error", {{"error", "screen capture failed"}}, message.requestId));
            return;
        }
        framed_->send(monitor::makeMessage("screenshot_response",
            {{"jpeg", QString::fromLatin1(screenshot.toBase64())},
             {"width", width}, {"height", height}}, message.requestId));
    } else if (message.type == "control_request") {
        if (!controlEnabled_ && !controlPromptActive_) {
            controlPromptActive_ = true;
            emit controlRequested();
        }
    } else if (message.type == "control_stop") {
        controlEnabled_ = false;
        controlPromptActive_ = false;
        emit controlStopped();
    } else if (message.type == "control_event" && controlEnabled_) {
        agent_input::apply(message.data);
    } else if (message.type == "command_request") {
        const QString command = message.data.value("command").toString();
        QJsonObject result;
        if (command == "get_system_info") result = agent_system::collect();
        else if (command == "list_processes") result = agent_system::processList();
        else if (command == "get_disk_info") result = {{"disk", agent_system::collect().value("disk")}};
        else result = {{"error", "command is not allowlisted"}};
        framed_->send(monitor::makeMessage("command_result", result, message.requestId));
    }
}

void AgentClient::approveControl(bool approved) {
    controlPromptActive_ = false;
    controlEnabled_ = approved;
    if (framed_) framed_->send(monitor::makeMessage("control_approval", {{"approved", approved}}));
}

void AgentClient::clearFramedSocket() {
    if (!framed_) return;
    framed_->disconnect(this);
    delete framed_;
    framed_ = nullptr;
}
