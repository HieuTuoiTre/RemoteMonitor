#include "common/transport/FramedSocket.h"

#include <algorithm>
#include <QDataStream>

namespace monitor {

FramedSocket::FramedSocket(QTcpSocket* socket, QObject* parent)
    : QObject(parent), socket_(socket) {
    Q_ASSERT(socket_);
    connect(socket_, &QTcpSocket::readyRead, this, &FramedSocket::readAvailable);
    connect(socket_, &QTcpSocket::disconnected, this, &FramedSocket::disconnected);
}

void FramedSocket::send(const Message& message) {
    const QByteArray payload = message.encode();
    if (payload.size() > static_cast<int>(kMaxFrameSize)) {
        emit protocolError("outgoing message exceeds frame limit");
        return;
    }
    QByteArray frame;
    frame.resize(4 + payload.size());
    const quint32 size = static_cast<quint32>(payload.size());
    frame[0] = static_cast<char>((size >> 24) & 0xff);
    frame[1] = static_cast<char>((size >> 16) & 0xff);
    frame[2] = static_cast<char>((size >> 8) & 0xff);
    frame[3] = static_cast<char>(size & 0xff);
    std::copy(payload.begin(), payload.end(), frame.begin() + 4);
    socket_->write(frame);
}

void FramedSocket::readAvailable() {
    buffer_.append(socket_->readAll());
    processFrames();
}

void FramedSocket::processFrames() {
    while (buffer_.size() >= 4) {
        const quint32 size = (static_cast<quint8>(buffer_[0]) << 24) |
                             (static_cast<quint8>(buffer_[1]) << 16) |
                             (static_cast<quint8>(buffer_[2]) << 8) |
                             static_cast<quint8>(buffer_[3]);
        if (size == 0 || size > kMaxFrameSize) {
            emit protocolError("invalid frame size");
            socket_->disconnectFromHost();
            return;
        }
        if (buffer_.size() < 4 + static_cast<int>(size)) return;
        const QByteArray payload = buffer_.mid(4, static_cast<int>(size));
        buffer_.remove(0, 4 + static_cast<int>(size));
        Message message;
        QString error;
        if (!Message::decode(payload, message, &error)) {
            emit protocolError(error);
            socket_->disconnectFromHost();
            return;
        }
        emit messageReceived(message);
    }
}

}
