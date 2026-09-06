#include "agent/system/SystemInfo.h"

#include <QHostInfo>
#include <QProcess>
#include <QStorageInfo>
#include <QSysInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace agent_system {

static double cpuUsage() {
#ifdef Q_OS_WIN
    FILETIME idle{}, kernel{}, user{};
    if (!GetSystemTimes(&idle, &kernel, &user)) return 0;
    static quint64 previousIdle = 0, previousKernel = 0, previousUser = 0;
    auto value = [](const FILETIME& time) {
        ULARGE_INTEGER result;
        result.LowPart = time.dwLowDateTime;
        result.HighPart = time.dwHighDateTime;
        return result.QuadPart;
    };
    const quint64 currentIdle = value(idle), currentKernel = value(kernel), currentUser = value(user);
    const quint64 idleDelta = currentIdle - previousIdle;
    const quint64 totalDelta = (currentKernel - previousKernel) + (currentUser - previousUser);
    previousIdle = currentIdle; previousKernel = currentKernel; previousUser = currentUser;
    return totalDelta ? (100.0 * (totalDelta - idleDelta) / totalDelta) : 0.0;
#else
    return 0.0;
#endif
}

static double memoryUsage() {
#ifdef Q_OS_WIN
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) return status.dwMemoryLoad;
#endif
    return 0.0;
}

static double diskUsage() {
    const QStorageInfo storage = QStorageInfo::root();
    if (storage.isValid() && storage.bytesTotal() > 0)
        return 100.0 * (1.0 - static_cast<double>(storage.bytesAvailable()) / storage.bytesTotal());
    return 0.0;
}

QJsonObject collect() {
    return {{"hostname", QHostInfo::localHostName()}, {"os", QSysInfo::prettyProductName()},
            {"cpu", cpuUsage()}, {"memory", memoryUsage()}, {"disk", diskUsage()}};
}

QJsonObject processList() {
    QProcess process;
#ifdef Q_OS_WIN
    process.start("tasklist", {"/fo", "csv", "/nh"});
#else
    process.start("ps", {"-e", "-o", "pid,comm"});
#endif
    process.waitForFinished(3000);
    return {{"text", QString::fromLocal8Bit(process.readAllStandardOutput())}};
}

}
