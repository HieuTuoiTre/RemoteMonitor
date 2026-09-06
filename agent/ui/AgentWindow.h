#pragma once

#include "agent/network/AgentClient.h"

#include <QMainWindow>

class QLineEdit;
class QLabel;

class AgentWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit AgentWindow(QWidget* parent = nullptr);

private slots:
    void connectAgent();
    void askControlConsent();

private:
    AgentClient client_;
    QLineEdit* host_ = nullptr;
    QLabel* status_ = nullptr;
    QLabel* identity_ = nullptr;
};
