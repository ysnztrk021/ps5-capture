#include <QApplication>
#include <QIcon>
#include <gst/gst.h>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // GStreamer DOIT être initialisé avant tout
    gst_init(&argc, &argv);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/Ps5_Logo.png"));

    MainWindow window;
    window.show();

    return app.exec();
}