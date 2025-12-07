#include <QApplication>
#include <QIcon>

#include "MazeWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Puzzles");
    app.setOrganizationName("Puzzles");
    
    QIcon appIcon(":/icon/logo.ico");
    if (appIcon.isNull()) {
        appIcon = QIcon(":/icon/logo.png");
    }
    app.setWindowIcon(appIcon);

    MazeWindow window;
    window.setWindowIcon(appIcon);
    window.show();

    return app.exec();
}
