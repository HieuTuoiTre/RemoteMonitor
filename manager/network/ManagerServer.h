#pragma once

#include "common/transport/FramedSocket.h"

#include <QHash>
#include <QJsonObject>
#include <QTcpServer>

struct AgentSnapshot {
    QString id;
    QString hostname;
    QString ip;
    QString os;
    double cpu = 0;
    double memory = 0;
    double disk = 0;
    bool online = false;
    QByteArray screenshot;
};

class ManagerServer final : public QObject {
    Q_OBJECT
public:
    explicit ManagerServer(QObject* parent = nullptr);
    bool listen(quint16 port, QString* error = nullptr);
    bool requestScreenshot(const QString& agentId, int maxWidth = 960,
                           int maxHeight = 540, int quality = 55);
    void requestControl(const QString& agentId);
    void stopControl(const QString& agentId);
    void requestSystemInfo(const QString& agentId);
    void requestProcesses(const QString& agentId);
    void sendCommand(const QString& agentId, const QString& command);
    void sendControlEvent(const QString& agentId, const QJsonObject& event);

    QList<AgentSnapshot> agents() const { return agents_.values(); }

signals:
    void agentChanged(const AgentSnapshot& snapshot);
    void logMessage(const QString& message);
    void controlApproved(const QString& agentId, bool approved);

private slots:
    void acceptConnection();
    void handleMessage(const monitor::Message& message);
    void handleDisconnected();

private:
    struct Connection {
        QTcpSocket* socket = nullptr;
        monitor::FramedSocket* framed = nullptr;
        QString agentId;
        bool screenshotRequestPending = false;
    };
    Connection* currentConnection(const QString& agentId) const;
    QTcpServer server_;
    QList<Connection*> connections_;
    QHash<QString, AgentSnapshot> agents_;
};
