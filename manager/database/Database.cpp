#include "manager/database/Database.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

Database::Database(const QString& path) {
    db_ = QSqlDatabase::addDatabase("QSQLITE", "monitor_manager");
    db_.setDatabaseName(path);
}

QByteArray Database::hashPassword(const QString& password, const QByteArray& salt) {
    QByteArray value = salt + password.toUtf8();
    for (int i = 0; i < 120000; ++i)
        value = QCryptographicHash::hash(value, QCryptographicHash::Sha256);
    return value.toHex();
}

bool Database::open(QString* error) {
    if (!db_.open()) {
        if (error) *error = db_.lastError().text();
        return false;
    }
    QSqlQuery query(db_);
    const QStringList schema = {
        "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password_hash BLOB NOT NULL, salt BLOB NOT NULL, role TEXT NOT NULL)",
        "CREATE TABLE IF NOT EXISTS agents (agent_id TEXT PRIMARY KEY, hostname TEXT, ip TEXT, os TEXT, approved INTEGER NOT NULL DEFAULT 0, last_seen TEXT, cpu REAL, memory REAL, disk REAL)",
        "CREATE TABLE IF NOT EXISTS audit_logs (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT, agent_id TEXT, action TEXT NOT NULL, created_at TEXT NOT NULL)",
    };
    for (const QString& statement : schema) {
        if (!query.exec(statement)) {
            if (error) *error = query.lastError().text();
            return false;
        }
    }
    query.prepare("SELECT COUNT(*) FROM users");
    if (!query.exec() || !query.next()) return false;
    if (query.value(0).toInt() == 0) {
        const QByteArray salt = QUuid::createUuid().toByteArray();
        query.prepare("INSERT INTO users(username,password_hash,salt,role) VALUES(?,?,?,?)");
        query.addBindValue("admin");
        query.addBindValue(hashPassword("admin", salt));
        query.addBindValue(salt);
        query.addBindValue("admin");
        if (!query.exec()) return false;
    }
    return true;
}

bool Database::authenticate(const QString& username, const QString& password, QString* role) {
    QSqlQuery query(db_);
    query.prepare("SELECT password_hash,salt,role FROM users WHERE username=?");
    query.addBindValue(username);
    if (!query.exec() || !query.next()) return false;
    if (query.value(0).toByteArray() != hashPassword(password, query.value(1).toByteArray()))
        return false;
    if (role) *role = query.value(2).toString();
    return true;
}

void Database::recordAudit(const QString& username, const QString& action, const QString& agentId) {
    QSqlQuery query(db_);
    query.prepare("INSERT INTO audit_logs(username,agent_id,action,created_at) VALUES(?,?,?,?)");
    query.addBindValue(username);
    query.addBindValue(agentId);
    query.addBindValue(action);
    query.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.exec();
}
