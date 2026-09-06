#include "common/protocol/Message.h"

#include <QJsonDocument>
#include <QUuid>

namespace monitor {

QByteArray Message::encode() const {
    QJsonObject object;
    object["type"] = type;
    object["request_id"] = requestId;
    object["version"] = version;
    object["data"] = data;
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool Message::decode(const QByteArray& bytes, Message& result, QString* error) {
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error) *error = parseError.errorString();
        return false;
    }
    const QJsonObject object = document.object();
    if (!object.value("type").isString() || !object.value("version").isDouble() ||
        !object.value("data").isObject()) {
        if (error) *error = "message requires type, version and object data";
        return false;
    }
    result.type = object.value("type").toString();
    result.requestId = object.value("request_id").toString();
    result.version = object.value("version").toInt();
    result.data = object.value("data").toObject();
    if (result.version != kProtocolVersion) {
        if (error) *error = "unsupported protocol version";
        return false;
    }
    return true;
}

Message makeMessage(const QString& type, const QJsonObject& data, const QString& requestId) {
    Message message;
    message.type = type;
    message.requestId = requestId.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                                            : requestId;
    message.data = data;
    return message;
}

}
