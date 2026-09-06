#pragma once

#include "common/protocol/Message.h"

#include <QObject>
#include <QTcpSocket>

namespace monitor {

class FramedSocket final : public QObject {
    Q_OBJECT
public:
    explicit FramedSocket(QTcpSocket* socket, QObject* parent = nullptr);

    QTcpSocket* socket() const { return socket_; }
    void send(const Message& message);

signals:
    void messageReceived(const monitor::Message& message);
    void protocolError(const QString& error);
    void disconnected();

private slots:
    void readAvailable();

private:
    void processFrames();
    QTcpSocket* socket_;
    QByteArray buffer_;
};

}
