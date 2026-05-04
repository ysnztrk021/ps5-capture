#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QIcon>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("PS5 Capture");
    setWindowIcon(QIcon(":/Ps5_Logo.png"));
    resize(960, 620);

    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setCentralWidget(central);

    // Zone vidéo
    gstWidget = new GstWidget(this);
    layout->addWidget(gstWidget, 1);

    // Barre de contrôles
    controlBar = new QWidget(this);
    controlBar->setStyleSheet("background-color: #1e1e1e;");
    QHBoxLayout *controls = new QHBoxLayout(controlBar);
    controls->setContentsMargins(10, 6, 10, 6);
    controls->setSpacing(8);

    // Sélecteur vidéo
    QLabel *videoLbl = new QLabel("Vidéo :", this);
    videoLbl->setStyleSheet("color: #aaa;");
    videoSelector = new QComboBox(this);
    videoSelector->setStyleSheet(
        "QComboBox { background: #2d2d2d; color: white; border: 1px solid #444;"
        "border-radius: 4px; padding: 4px 8px; min-width: 160px; }"
        "QComboBox QAbstractItemView { background: #2d2d2d; color: white; }"
    );

    // Sélecteur audio
    QLabel *audioLbl = new QLabel("Audio :", this);
    audioLbl->setStyleSheet("color: #aaa;");
    audioSelector = new QComboBox(this);
    audioSelector->setStyleSheet(
        "QComboBox { background: #2d2d2d; color: white; border: 1px solid #444;"
        "border-radius: 4px; padding: 4px 8px; min-width: 160px; }"
        "QComboBox QAbstractItemView { background: #2d2d2d; color: white; }"
    );

    controls->addWidget(videoLbl);
    controls->addWidget(videoSelector, 1);
    controls->addWidget(audioLbl);
    controls->addWidget(audioSelector, 1);

    QPushButton *startBtn = new QPushButton("▶  Lancer", this);
    QPushButton *stopBtn  = new QPushButton("⏹  Arrêter", this);
    fullscreenBtn         = new QPushButton("⛶  Plein écran", this);

    QString btnStyle =
        "QPushButton { background: #0078d4; color: white; border: none;"
        "border-radius: 4px; padding: 6px 18px; font-weight: bold; }"
        "QPushButton:hover { background: #1a8ae4; }"
        "QPushButton:pressed { background: #005fa3; }";

    startBtn->setStyleSheet(btnStyle);
    fullscreenBtn->setStyleSheet(btnStyle);
    stopBtn->setStyleSheet(
        "QPushButton { background: #c42b1c; color: white; border: none;"
        "border-radius: 4px; padding: 6px 18px; font-weight: bold; }"
        "QPushButton:hover { background: #d4392a; }"
        "QPushButton:pressed { background: #a02010; }"
    );

    controls->addWidget(startBtn);
    controls->addWidget(stopBtn);
    controls->addWidget(fullscreenBtn);
    layout->addWidget(controlBar);

    statusLabel = new QLabel("  Aucune vidéo active", this);
    statusLabel->setStyleSheet(
        "color: #aaa; background: #141414; padding: 4px 10px; font-size: 12px;"
    );
    statusLabel->setFixedHeight(24);
    layout->addWidget(statusLabel);

    populateVideoDevices();
    populateAudioDevices();

    connect(startBtn,      &QPushButton::clicked, this, &MainWindow::startVideo);
    connect(stopBtn,       &QPushButton::clicked, this, &MainWindow::stopVideo);
    connect(fullscreenBtn, &QPushButton::clicked, this, &MainWindow::toggleFullscreen);
    connect(gstWidget,     &GstWidget::stopped,   this, &MainWindow::stopVideo);
}

MainWindow::~MainWindow()
{
    stopVideo();
}

void MainWindow::toggleFullscreen()
{
    if (isFullScreen()) {
        showNormal();
        controlBar->show();
        statusLabel->show();
        fullscreenBtn->setText("⛶  Plein écran");
    } else {
        controlBar->hide();
        statusLabel->hide();
        fullscreenBtn->setText("✕  Quitter plein écran");
        showFullScreen();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11) {
        toggleFullscreen();
    } else if (event->key() == Qt::Key_Escape && isFullScreen()) {
        toggleFullscreen();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::populateVideoDevices()
{
    videoSelector->clear();

    QDir sysDir("/sys/class/video4linux");
    QStringList entries = sysDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &v : entries) {
        QString devPath = "/dev/" + v;
        QFile f("/sys/class/video4linux/" + v + "/name");
        QString cardName;
        if (f.open(QIODevice::ReadOnly)) {
            cardName = f.readAll().trimmed();
            f.close();
        }
        if (cardName.contains("USB3", Qt::CaseInsensitive) ||
            cardName.contains("capture", Qt::CaseInsensitive)) {
            videoSelector->addItem(QString("%1 — %2").arg(v, cardName), devPath);
        }
    }

    if (videoSelector->count() == 0) {
        for (const QString &v : entries)
            videoSelector->addItem(v, "/dev/" + v);
    }
}

void MainWindow::populateAudioDevices()
{
    audioSelector->clear();
    audioSelector->addItem("— Aucun audio —", QString());

    QProcess proc;
    proc.start("pactl", {"list", "short", "sources"});
    proc.waitForFinished();

    QString output = proc.readAllStandardOutput();
    int autoSelectIndex = 0;

    for (const QString &line : output.split('\n')) {
        if (line.trimmed().isEmpty()) continue;

        QStringList parts = line.split('\t');
        if (parts.size() < 2) continue;

        QString sourceName = parts[1].trimmed();

        // Ignorer les monitors
        if (sourceName.contains(".monitor")) continue;

        // Nom court pour l'affichage
        QString label = sourceName;
        if (sourceName.contains("UltraSemi"))
            label = "PS5 Capture — USB3 Video";
        else if (sourceName.contains("Arctis"))
            label = "SteelSeries Arctis Nova 5X";
        else if (sourceName.contains("Mic1") || sourceName.contains("Mic2"))
            label = "Micro intégré";

        audioSelector->addItem(label, sourceName);

        // Sélection automatique de la carte de capture PS5
        if (sourceName.contains("UltraSemi", Qt::CaseInsensitive) ||
            sourceName.contains("USB3_Video", Qt::CaseInsensitive)) {
            autoSelectIndex = audioSelector->count() - 1;
        }
    }

    audioSelector->setCurrentIndex(autoSelectIndex);
}

void MainWindow::startVideo()
{
    QString videoDevice = videoSelector->currentData().toString();
    QString audioDevice = audioSelector->currentData().toString();

    if (videoDevice.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Aucun périphérique vidéo sélectionné.");
        return;
    }

    gstWidget->play(videoDevice, audioDevice);
    statusLabel->setText("  ▶ Vidéo active : " + videoDevice);
}

void MainWindow::stopVideo()
{
    gstWidget->stop();
    statusLabel->setText("  ⏹ Vidéo arrêtée");
}