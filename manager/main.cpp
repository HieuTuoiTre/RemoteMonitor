#include "manager/ui/ManagerWindow.h"

#include <QApplication>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    ManagerWindow window;
    window.show();
    return app.exec();
}
