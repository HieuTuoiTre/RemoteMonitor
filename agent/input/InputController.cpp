#include "agent/input/InputController.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace agent_input {

bool apply(const QJsonObject& event) {
#ifdef Q_OS_WIN
    const QString kind = event.value("kind").toString();
    if (kind == "key") {
        const int key = event.value("key").toInt();
        if (key <= 0 || key > 0xff) return false;
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<WORD>(key);
        input.ki.dwFlags = event.value("pressed").toBool() ? 0 : KEYEVENTF_KEYUP;
        return SendInput(1, &input, sizeof(INPUT)) == 1;
    }
    if (kind == "press") {
        const int button = event.value("button").toInt();
        if (button != 1 && button != 2 && button != 4) return false;
        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = button == 1
                               ? MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP
                               : MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP;
        return SendInput(1, &input, sizeof(INPUT)) == 1;
    }
    if (kind == "move") {
        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dx = qBound(-32768, event.value("x").toInt(), 32767);
        input.mi.dy = qBound(-32768, event.value("y").toInt(), 32767);
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        return SendInput(1, &input, sizeof(INPUT)) == 1;
    }
    return true;
#else
    Q_UNUSED(event);
    return false;
#endif
}

}
