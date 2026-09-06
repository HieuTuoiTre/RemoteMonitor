#pragma once

#include "manager/ui/ControlView.h"

#include <QMainWindow>
#include <QPixmap>

class QLabel;

class ObservationWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit ObservationWindow(QWidget* parent = nullptr);

    QString agentId() const { return agentId_; }
    void setAgentId(const QString& agentId);
    void setFrame(const QByteArray& jpeg);
    void setControlEnabled(bool enabled);

signals:
    void requestControl();
    void stopControl();
    void mouseEvent(const QString& kind, int x, int y, int button);
    void keyEvent(int key, bool pressed);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void renderFrame();

    QString agentId_;
    QPixmap latestFrame_;
    ControlView* view_ = nullptr;
    QLabel* status_ = nullptr;
};
