#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QMetaType>
#include <QString>

namespace monitor {

constexpr int kProtocolVersion = 1;
constexpr quint32 kMaxFrameSize = 8 * 1024 * 1024;

struct Message {
    QString type;
    QString requestId;
    int version = kProtocolVersion;
    QJsonObject data;

    QByteArray encode() const;
    static bool decode(const QByteArray& bytes, Message& result, QString* error = nullptr);
};

Message makeMessage(const QString& type, const QJsonObject& data = {},
                    const QString& requestId = {});

}

Q_DECLARE_METATYPE(monitor::Message)
