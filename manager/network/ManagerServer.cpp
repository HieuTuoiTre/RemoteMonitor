#include "manager/network/ManagerServer.h"

#include <QHostAddress>
#include <QJsonDocument>

ManagerServer::ManagerServer(QObject* parent) : QObject(parent) {
    connect(&server_, &QTcpServer::newConnection, this, &ManagerServer::acceptConnection);
}

bool ManagerServer::listen(quint16 port, QString* error) {
    if (!server_.listen(QHostAddress::Any, port)) {
        if (error) *error = server_.errorString();
        return false;
    }
    emit logMessage(QString("Manager listening on TCP %1").arg(port));
    return true;
}

void ManagerServer::acceptConnection() {
    while (server_.hasPendingConnections()) {
        auto* socket = server_.nextPendingConnection();
        auto* connection = new Connection;
        connection->socket = socket;
        connection->framed = new monitor::FramedSocket(socket, this);
        connections_.append(connection);
        connect(connection->framed, &monitor::FramedSocket::messageReceived,
                this, &ManagerServer::handleMessage);
        connect(connection->framed, &monitor::FramedSocket::disconnected,
                this, &ManagerServer::handleDisconnected);
        emit logMessage(QString("Agent connection from %1").arg(socket->peerAddress().toString()));
    }
}

ManagerServer::Connection* ManagerServer::currentConnection(const QString& agentId) const {
    for (Connection* connection : connections_)
        if (connection->agentId == agentId) return connection;
    return nullptr;
}

void ManagerServer::handleMessage(const monitor::Message& message) {
    auto* framed = qobject_cast<monitor::FramedSocket*>(sender());
    if (!framed) return;
    Connection* connection = nullptr;
    for (Connection* candidate : connections_)
        if (candidate->framed == framed) connection = candidate;
    if (!connection) return;

    if (message.type == "agent_hello") {
        const auto data = message.data;
        connection->agentId = data.value("agent_id").toString();
        AgentSnapshot& snapshot = agents_[connection->agentId];
        snapshot.id = connection->agentId;
        snapshot.hostname = data.value("hostname").toString();
        snapshot.os = data.value("os").toString();
        snapshot.ip = connection->socket->peerAddress().toString();
        snapshot.online = true;
        framed->send(monitor::makeMessage("agent_approved", {{"approved", true}}));
        emit agentChanged(snapshot);
        return;
    }
    if (connection->agentId.isEmpty()) return;
    AgentSnapshot& snapshot = agents_[connection->agentId];
    if (message.type == "heartbeat") {
        snapshot.cpu = message.data.value("cpu").toDouble();
        snapshot.memory = message.data.value("memory").toDouble();
        snapshot.disk = message.data.value("disk").toDouble();
        snapshot.online = true;
        emit agentChanged(snapshot);
    } else if (message.type == "system_info_response") {
        snapshot.hostname = message.data.value("hostname").toString();
        snapshot.os = message.data.value("os").toString();
        snapshot.cpu = message.data.value("cpu").toDouble();
        snapshot.memory = message.data.value("memory").toDouble();
        snapshot.disk = message.data.value("disk").toDouble();
        emit agentChanged(snapshot);
    } else if (message.type == "screenshot_response") {
        snapshot.screenshot = QByteArray::fromBase64(message.data.value("jpeg").toString().toLatin1());
        connection->screenshotRequestPending = false;
        emit agentChanged(snapshot);
    } else if (message.type == "control_approval") {
        emit controlApproved(connection->agentId, message.data.value("approved").toBool());
    } else if (message.type == "command_result" || message.type == "process_list_response") {
        emit logMessage(QString("[%1] %2: %3").arg(connection->agentId, message.type,
            QString::fromUtf8(QJsonDocument(message.data).toJson(QJsonDocument::Compact))));
    } else if (message.type == "audit_event") {
        emit logMessage(QString("[%1] %2").arg(connection->agentId, message.data.value("action").toString()));
    } else if (message.type == "error") {
        connection->screenshotRequestPending = false;
        emit logMessage(QString("[%1] error: %2").arg(connection->agentId,
            message.data.value("error").toString()));
    }
}

void ManagerServer::handleDisconnected() {
    auto* framed = qobject_cast<monitor::FramedSocket*>(sender());
    for (int i = connections_.size() - 1; i >= 0; --i) {
        if (connections_[i]->framed != framed) continue;
        const QString id = connections_[i]->agentId;
        if (!id.isEmpty()) {
            agents_[id].online = false;
            emit agentChanged(agents_[id]);
        }
        connections_[i]->socket->deleteLater();
        connections_[i]->framed->deleteLater();
        delete connections_.takeAt(i);
    }
}

bool ManagerServer::requestScreenshot(const QString& agentId, int maxWidth,
                                      int maxHeight, int quality) {
    auto* connection = currentConnection(agentId);
    if (!connection || !connection->framed || connection->screenshotRequestPending ||
        connection->socket->state() != QAbstractSocket::ConnectedState)
        return false;
    connection->screenshotRequestPending = true;
    connection->framed->send(monitor::makeMessage("screenshot_request", {
        {"max_width", maxWidth}, {"max_height", maxHeight}, {"jpeg_quality", quality}}));
    return true;
}

void ManagerServer::requestControl(const QString& agentId) {
    if (auto* connection = currentConnection(agentId);
        connection && connection->framed && connection->socket->state() == QAbstractSocket::ConnectedState)
        connection->framed->send(monitor::makeMessage("control_request"));
}

void ManagerServer::stopControl(const QString& agentId) {
    if (auto* connection = currentConnection(agentId);
        connection && connection->framed && connection->socket->state() == QAbstractSocket::ConnectedState)
        connection->framed->send(monitor::makeMessage("control_stop"));
}

void ManagerServer::requestSystemInfo(const QString& agentId) {
    if (auto* connection = currentConnection(agentId))
        connection->framed->send(monitor::makeMessage("system_info_request"));
}

void ManagerServer::requestProcesses(const QString& agentId) {
    if (auto* connection = currentConnection(agentId))
        connection->framed->send(monitor::makeMessage("process_list_request"));
}

void ManagerServer::sendCommand(const QString& agentId, const QString& command) {
    if (auto* connection = currentConnection(agentId))
        connection->framed->send(monitor::makeMessage("command_request", {{"command", command}}));
}

void ManagerServer::sendControlEvent(const QString& agentId, const QJsonObject& event) {
    if (auto* connection = currentConnection(agentId);
        connection && connection->framed && connection->socket->state() == QAbstractSocket::ConnectedState)
        connection->framed->send(monitor::makeMessage("control_event", event));
}
