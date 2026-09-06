#include "common/protocol/Message.h"

#include <QTest>

class MessageTests final : public QObject {
    Q_OBJECT
private slots:
    void roundTrip() {
        const auto original = monitor::makeMessage("heartbeat", {{"cpu", 12.5}});
        monitor::Message decoded;
        QVERIFY(monitor::Message::decode(original.encode(), decoded));
        QCOMPARE(decoded.type, original.type);
        QCOMPARE(decoded.data.value("cpu").toDouble(), 12.5);
    }

    void rejectsMalformed() {
        monitor::Message decoded;
        QString error;
        QVERIFY(!monitor::Message::decode("not-json", decoded, &error));
        QVERIFY(!error.isEmpty());
    }
};

QTEST_MAIN(MessageTests)
#include "MessageTests.moc"
