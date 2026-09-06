#include "agent/capture/ScreenCapture.h"

#include <QBuffer>
#include <QGuiApplication>
#include <QImage>
#include <QPixmap>
#include <QScreen>

namespace agent_capture {

QByteArray screenshotJpeg() {
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) return {};
    QPixmap pixmap = screen->grabWindow(0);
    QImage image = pixmap.toImage();
    image = image.scaled(1280, 720, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG", 65);
    return bytes;
}

}
