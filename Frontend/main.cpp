#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("NetworkRecoverySimulator");
    QCoreApplication::setOrganizationName("NOC Engineering");
    QCoreApplication::setApplicationVersion("2.0.0");

    MainWindow window;
    window.show();

    return app.exec();
}
