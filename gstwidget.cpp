#include "gstwidget.h"

#include <QVBoxLayout>
#include <QDebug>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>

GstWidget::GstWidget(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: black;");
    setMinimumSize(640, 360);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    videoLabel = new QLabel(this);
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setStyleSheet("background-color: black;");
    videoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(videoLabel);

    busTimer = new QTimer(this);
    busTimer->setInterval(50);
    connect(busTimer, &QTimer::timeout, this, &GstWidget::checkBus);

    connect(this, &GstWidget::frameReady,
            this, [this](QImage image) {
        if (!videoLabel) return;
        videoLabel->setPixmap(
            QPixmap::fromImage(image).scaled(
                videoLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            )
        );
    }, Qt::QueuedConnection);
}

GstWidget::~GstWidget()
{
    stop();
}

void GstWidget::play(const QString &videoDevice, const QString &audioDevice)
{
    stop();
    buildPipeline(videoDevice, audioDevice);
    if (!pipeline) return;

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qWarning() << "GStreamer: impossible de démarrer le pipeline";
        gst_object_unref(pipeline);
        pipeline = nullptr;
        return;
    }

    playing = true;
    busTimer->start();
}

void GstWidget::stop()
{
    busTimer->stop();

    if (!pipeline) return;

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    pipeline = nullptr;
    appsink  = nullptr;
    playing  = false;

    if (videoLabel)
        videoLabel->clear();
}

void GstWidget::buildPipeline(const QString &videoDevice, const QString &audioDevice)
{
    // Branche audio via PipeWire/Pulse
    QString audioPart;
    if (!audioDevice.isEmpty()) {
        audioPart = QString(
            "pulsesrc device=%1 ! "
            "audioconvert ! "
            "audioresample ! "
            "autoaudiosink "
        ).arg(audioDevice);
    }

    // Branche vidéo MJPG -> appsink
    QString videoPart = QString(
        "v4l2src device=%1 "
        "! image/jpeg,width=1920,height=1080,framerate=30/1 "
        "! jpegdec "
        "! videoconvert "
        "! video/x-raw,format=RGB "
        "! appsink name=appsink emit-signals=true max-buffers=1 drop=true sync=false"
    ).arg(videoDevice);

    QString pipeStr = audioPart.isEmpty()
        ? videoPart
        : videoPart + " " + audioPart;

    GError *err = nullptr;
    pipeline = gst_parse_launch(pipeStr.toUtf8().constData(), &err);

    if (err || !pipeline) {
        qWarning() << "Pipeline MJPG échoué, essai YUYV...";
        if (err) g_error_free(err);
        err = nullptr;

        QString fallbackVideo = QString(
            "v4l2src device=%1 "
            "! video/x-raw,format=YUYV,width=1280,height=720,framerate=30/1 "
            "! videoconvert "
            "! video/x-raw,format=RGB "
            "! appsink name=appsink emit-signals=true max-buffers=1 drop=true sync=false"
        ).arg(videoDevice);

        QString fallback = audioPart.isEmpty()
            ? fallbackVideo
            : fallbackVideo + " " + audioPart;

        pipeline = gst_parse_launch(fallback.toUtf8().constData(), &err);
        if (err || !pipeline) {
            qWarning() << "Pipeline YUYV échoué:" << (err ? err->message : "?");
            if (err) g_error_free(err);
            pipeline = nullptr;
            return;
        }
    }

    appsink = gst_bin_get_by_name(GST_BIN(pipeline), "appsink");
    if (!appsink) {
        qWarning() << "appsink introuvable";
        gst_object_unref(pipeline);
        pipeline = nullptr;
        return;
    }

    GstAppSinkCallbacks callbacks{};
    callbacks.new_sample = newSampleCallback;
    gst_app_sink_set_callbacks(GST_APP_SINK(appsink), &callbacks, this, nullptr);
}

GstFlowReturn GstWidget::newSampleCallback(GstAppSink *sink, gpointer data)
{
    GstWidget *self = static_cast<GstWidget *>(data);

    GstSample *sample = gst_app_sink_pull_sample(sink);
    if (!sample) return GST_FLOW_ERROR;

    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstCaps   *caps   = gst_sample_get_caps(sample);

    GstVideoInfo info;
    gst_video_info_from_caps(&info, caps);

    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        QImage image(
            map.data,
            GST_VIDEO_INFO_WIDTH(&info),
            GST_VIDEO_INFO_HEIGHT(&info),
            GST_VIDEO_INFO_PLANE_STRIDE(&info, 0),
            QImage::Format_RGB888
        );
        emit self->frameReady(image.copy());
        gst_buffer_unmap(buffer, &map);
    }

    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

void GstWidget::checkBus()
{
    if (!pipeline) return;

    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg;

    while ((msg = gst_bus_pop(bus)) != nullptr) {
        switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            GError *err = nullptr;
            gchar  *dbg = nullptr;
            gst_message_parse_error(msg, &err, &dbg);
            qWarning() << "[GST] Erreur:" << err->message;
            qWarning() << "[GST] Debug:" << dbg;
            g_error_free(err);
            g_free(dbg);
            stop();
            emit stopped();
            break;
        }
        case GST_MESSAGE_EOS:
            stop();
            emit stopped();
            break;
        default:
            break;
        }
        gst_message_unref(msg);
    }
    gst_object_unref(bus);
}