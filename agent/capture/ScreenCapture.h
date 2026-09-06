#pragma once

#include <QByteArray>

namespace agent_capture {
QByteArray screenshotJpeg(int maxWidth = 1280, int maxHeight = 720, int quality = 55);
}
