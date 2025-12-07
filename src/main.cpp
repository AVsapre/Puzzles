#include <QApplication>
#include "MazeWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Puzzles");
    app.setOrganizationName("Puzzles");
    app.setQuitOnLastWindowClosed(true);

    MazeWindow window;
    window.show();

    return app.exec();
}
