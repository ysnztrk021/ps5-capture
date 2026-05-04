#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include "gstwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void startVideo();
    void stopVideo();
    void toggleFullscreen();

private:
    void populateVideoDevices();
    void populateAudioDevices();

    QComboBox   *videoSelector  = nullptr;
    QComboBox   *audioSelector  = nullptr;
    QLabel      *statusLabel    = nullptr;
    GstWidget   *gstWidget      = nullptr;
    QWidget     *controlBar     = nullptr;
    QPushButton *fullscreenBtn  = nullptr;
};

#endif