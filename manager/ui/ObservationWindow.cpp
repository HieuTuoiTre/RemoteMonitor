#include "manager/ui/ObservationWindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

ObservationWindow::ObservationWindow(QWidget* parent) : QMainWindow(parent) {
    auto* root = new QWidget(this);
    auto* layout = new QVBoxLayout(root);
    view_ = new ControlView(root);
    view_->setMinimumSize(960, 540);
    auto* controls = new QHBoxLayout;
    auto* request = new QPushButton("Request control", root);
    auto* stop = new QPushButton("Stop control", root);
    status_ = new QLabel("Control is disabled until the Agent approves it", root);
    controls->addWidget(request);
    controls->addWidget(stop);
    controls->addWidget(status_, 1);
    layout->addWidget(view_, 1);
    layout->addLayout(controls);
    setCentralWidget(root);
    setWindowFlag(Qt::Window);
    resize(1280, 800);
    setWindowTitle("Remote Monitor - Observation");

    connect(request, &QPushButton::clicked, this, &ObservationWindow::requestControl);
    connect(stop, &QPushButton::clicked, this, &ObservationWindow::stopControl);
    connect(view_, &ControlView::mouseEvent, this, &ObservationWindow::mouseEvent);
    connect(view_, &ControlView::keyEvent, this, &ObservationWindow::keyEvent);
}

void ObservationWindow::setAgentId(const QString& agentId) {
    if (agentId_ == agentId) return;
    agentId_ = agentId;
    setWindowTitle(agentId.isEmpty() ? "Remote Monitor - Observation"
                                     : "Remote Monitor - Observation - " + agentId);
    latestFrame_ = {};
    view_->setText(agentId.isEmpty() ? "Select an Agent" : "Waiting for HD frame...");
}

void ObservationWindow::setFrame(const QByteArray& jpeg) {
    QPixmap frame;
    if (!frame.loadFromData(jpeg, "JPG")) return;
    latestFrame_ = frame;
    renderFrame();
}

void ObservationWindow::setControlEnabled(bool enabled) {
    view_->setEnabledControl(enabled);
    status_->setText(enabled ? "Control approved by Agent"
                             : "Control is disabled until the Agent approves it");
}

void ObservationWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    renderFrame();
}

void ObservationWindow::renderFrame() {
    if (latestFrame_.isNull() || !view_) return;
    view_->setPixmap(latestFrame_.scaled(view_->size(), Qt::KeepAspectRatio,
                                         Qt::FastTransformation));
}
