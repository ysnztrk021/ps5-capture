#ifndef GSTWIDGET_H
#define GSTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class GstWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GstWidget(QWidget *parent = nullptr);
    ~GstWidget();

    void play(const QString &videoDevice, const QString &audioDevice = QString());
    void stop();
    bool isPlaying() const { return playing; }

signals:
    void stopped();
    void frameReady(QImage image);

private:
    GstElement *pipeline   = nullptr;
    GstElement *appsink    = nullptr;
    bool        playing    = false;
    QLabel     *videoLabel = nullptr;
    QTimer     *busTimer   = nullptr;

    void buildPipeline(const QString &videoDevice, const QString &audioDevice);
    void checkBus();

    static GstFlowReturn newSampleCallback(GstAppSink *sink, gpointer data);
};

#endif