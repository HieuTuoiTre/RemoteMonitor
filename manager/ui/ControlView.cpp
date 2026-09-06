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
    setFocus();
}

void ControlView::mousePressEvent(QMouseEvent* event) {
    if (enabledControl_)
        emit mouseEvent("press", event->position().x(), event->position().y(), event->button());
    QLabel::mousePressEvent(event);
}

void ControlView::mouseMoveEvent(QMouseEvent* event) {
    if (enabledControl_)
        emit mouseEvent("move", event->position().x(), event->position().y(), 0);
    QLabel::mouseMoveEvent(event);
}

void ControlView::keyPressEvent(QKeyEvent* event) {
    if (enabledControl_) emit keyEvent(event->nativeVirtualKey(), true);
    QLabel::keyPressEvent(event);
}

void ControlView::keyReleaseEvent(QKeyEvent* event) {
    if (enabledControl_) emit keyEvent(event->nativeVirtualKey(), false);
    QLabel::keyReleaseEvent(event);
}
