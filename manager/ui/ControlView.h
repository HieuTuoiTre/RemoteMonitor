#pragma once

#include <QLabel>
#include <QElapsedTimer>

class ControlView final : public QLabel {
    Q_OBJECT
public:
    explicit ControlView(QWidget* parent = nullptr);
    void setEnabledControl(bool enabled);

signals:
    void mouseEvent(const QString& kind, int x, int y, int button);
    void keyEvent(int key, bool pressed);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    bool enabledControl_ = false;
    QElapsedTimer moveClock_;
};
