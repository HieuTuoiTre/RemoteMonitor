#pragma once

#include "manager/database/Database.h"
#include "manager/network/ManagerServer.h"
#include "manager/ui/ControlView.h"

#include <QMainWindow>
#include <QTableWidget>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QTimer>

class ManagerWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit ManagerWindow(QWidget* parent = nullptr);

private slots:
    void login();
    void refreshAgent(const AgentSnapshot& snapshot);
    void controlApproval(const QString& agentId, bool approved);
    void selectAgent();
    void requestScreenshot();
    void requestControl();
    void stopControl();
    void sendSafeCommand();
    void sendMouse(const QString& kind, int x, int y, int button);
    void sendKey(int key, bool pressed);
    void appendLog(const QString& message);

private:
    void buildLoginUi();
    void buildDashboardUi();
    QString selectedAgent() const;
    Database database_;
    ManagerServer server_;
    QLineEdit* username_ = nullptr;
    QLineEdit* password_ = nullptr;
    QLabel* loginStatus_ = nullptr;
    QTableWidget* agents_ = nullptr;
    ControlView* controlView_ = nullptr;
    QLabel* detail_ = nullptr;
    QComboBox* command_ = nullptr;
    QTextEdit* log_ = nullptr;
    QTimer screenshotTimer_;
    bool controlEnabled_ = false;
};
