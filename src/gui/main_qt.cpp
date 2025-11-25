#include <QApplication>

#include "MazeWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("MazeUtilQt");
    app.setOrganizationName("MazeUtil");

    MazeWindow window;
    window.show();

    return app.exec();
}
