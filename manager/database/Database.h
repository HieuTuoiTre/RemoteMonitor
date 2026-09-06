#pragma once

#include <QSqlDatabase>
#include <QString>

class Database final {
public:
    explicit Database(const QString& path);
    bool open(QString* error = nullptr);
    bool authenticate(const QString& username, const QString& password, QString* role = nullptr);
    void recordAudit(const QString& username, const QString& action, const QString& agentId = {});

private:
    static QByteArray hashPassword(const QString& password, const QByteArray& salt);
    QSqlDatabase db_;
};
