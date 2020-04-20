#include <QApplication>
// #include <QProgressBar>
// #include <QSlider>
#include "qdemowindow.h"

int main(int argc, char **argv)
{
    QApplication app (argc, argv);

    // Create a container window
    QDemoWindow window;
    window.show();

    return app.exec();
}
