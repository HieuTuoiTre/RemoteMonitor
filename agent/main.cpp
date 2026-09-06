#include "agent/ui/AgentWindow.h"

#include <QApplication>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    AgentWindow window;
    window.show();
    return app.exec();
}
