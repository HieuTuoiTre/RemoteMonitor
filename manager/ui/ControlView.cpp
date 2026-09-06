#include "manager/ui/ControlView.h"

#include <QKeyEvent>
#include <QMouseEvent>

ControlView::ControlView(QWidget* parent) : QLabel(parent) {
    setAlignment(Qt::AlignCenter);
    setMinimumSize(640, 360);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setText("No screenshot");
}

void ControlView::setEnabledControl(bool enabled) {
    enabledControl_ = enabled;
    if (enabled) setFocus();
    else clearFocus();
}

void ControlView::mousePressEvent(QMouseEvent* event) {
    if (enabledControl_) {
        emit mouseEvent("press", qRound(event->position().x()), qRound(event->position().y()),
                        static_cast<int>(event->button()));
        event->accept();
        return;
    }
    QLabel::mousePressEvent(event);
}

void ControlView::mouseMoveEvent(QMouseEvent* event) {
    if (enabledControl_) {
        if (!moveClock_.isValid() || moveClock_.hasExpired(16)) {
            moveClock_.restart();
            emit mouseEvent("move", qRound(event->position().x()), qRound(event->position().y()), 0);
        }
        event->accept();
        return;
    }
    QLabel::mouseMoveEvent(event);
}

void ControlView::keyPressEvent(QKeyEvent* event) {
    if (enabledControl_) {
        const int key = static_cast<int>(event->nativeVirtualKey());
        if (key > 0 && key <= 0xff) emit keyEvent(key, true);
        event->accept();
        return;
    }
    QLabel::keyPressEvent(event);
}

void ControlView::keyReleaseEvent(QKeyEvent* event) {
    if (enabledControl_) {
        const int key = static_cast<int>(event->nativeVirtualKey());
        if (key > 0 && key <= 0xff) emit keyEvent(key, false);
        event->accept();
        return;
    }
    QLabel::keyReleaseEvent(event);
}
