#pragma once

#include "common/transport/FramedSocket.h"

#include <QTimer>

class AgentClient final : public QObject {
    Q_OBJECT
public:
    explicit AgentClient(QObject* parent = nullptr);
    void connectToManager(const QString& host, quint16 port);
    void disconnectFromManager();
    QString agentId() const { return agentId_; }

signals:
    void statusChanged(const QString& status);
    void controlRequested();
    void controlStopped();

public slots:
    void approveControl(bool approved);

private slots:
    void connected();
    void reconnect();
    void heartbeat();
    void messageReceived(const monitor::Message& message);

private:
    void sendSystemInfo(const QString& type, const QString& requestId = {});
    void clearFramedSocket();
    QTcpSocket socket_;
    monitor::FramedSocket* framed_ = nullptr;
    QTimer heartbeatTimer_;
    QTimer reconnectTimer_;
    QString host_;
    quint16 port_ = 45454;
    QString agentId_;
    bool controlEnabled_ = false;
    bool controlPromptActive_ = false;
    bool manualDisconnect_ = false;
};
