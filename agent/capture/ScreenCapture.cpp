#include "agent/capture/ScreenCapture.h"

#include <QBuffer>
#include <QGuiApplication>
#include <QImage>
#include <QPixmap>
#include <QScreen>

namespace agent_capture {

QByteArray screenshotJpeg(int maxWidth, int maxHeight, int quality) {
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) return {};
    QPixmap pixmap = screen->grabWindow(0);
    if (pixmap.isNull()) return {};
    QImage image = pixmap.toImage();
    image = image.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::FastTransformation);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    if (!image.save(&buffer, "JPG", qBound(20, quality, 90))) return {};
    return bytes;
}

}
