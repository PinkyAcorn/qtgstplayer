// local headers
#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    player = new GstPlayer(this);

    createActions();
    createContextMenu();
    createSpeedMenu();
    createControlBar();

    setWindowTitle("QtGstPlayer");
    setMinimumSize(640, 480);
    setCentralWidget(player);
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    contextMenu->exec(event->globalPos());
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *e)
{
    turnFullscreen();
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    //
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space)
        pauseAction->trigger();
}

void MainWindow::showEvent(QShowEvent *event)
{
    connect(player, &GstPlayer::sourceChanged, this, [&](auto src){setWindowTitle(src.path());});
    player->open(QUrl("videotestsrc"));
    player->play();
}

void MainWindow::createActions()
{
    openFileAction = new QAction(QIcon::fromTheme("document-open"), "Open file...", this);
    // connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    openRTSPAction = new QAction(QIcon::fromTheme("network-transmit-receive"), "Open RTSP stream...", this);
    // connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    quitAction = new QAction(QIcon::fromTheme("application-exit"), "Quit", this);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    playAction = new QAction(QIcon::fromTheme("media-playback-start"), "Play", this);
    connect(playAction, &QAction::triggered, player, &GstPlayer::play);

    pauseAction = new QAction(QIcon::fromTheme("media-playback-pause"), "Pause", this);
    connect(pauseAction, &QAction::triggered, player, &GstPlayer::pause);

    recordAction = new QAction(QIcon::fromTheme("media-record"), "Record", this);
    // connect(recordAction, &QAction::triggered, this, &MainWindow::showSettingsMenu);

    stopAction = new QAction(QIcon::fromTheme("media-playback-stop"), "Stop", this);
    // connect(stopAction, &QAction::triggered, this, &MainWindow::close);

    fullscreenAction = new QAction(QIcon::fromTheme("view-fullscreen"), "Full screen", this);
    connect(fullscreenAction, &QAction::triggered, this, &MainWindow::turnFullscreen);

    setSpeedX_25 = new QAction(QIcon::fromTheme("media-playback-pause"), "x0.25", this);
    setSpeedX_50 = new QAction(QIcon::fromTheme("media-playback-pause"), "x0.50", this);
    setSpeedX_75 = new QAction(QIcon::fromTheme("media-playback-pause"), "x0.75", this);
    setSpeedX1 = new QAction(QIcon::fromTheme("media-playback-start"), "Normal", this);
    setSpeedX2 = new QAction(QIcon::fromTheme("media-seek-forward"), "x2", this);
    setSpeedX4 = new QAction(QIcon::fromTheme("media-seek-forward"), "x4", this);
    setSpeedX8 = new QAction(QIcon::fromTheme("media-seek-forward"), "x8", this);
    setSpeedX16 = new QAction(QIcon::fromTheme("media-seek-forward"), "x16", this);
    setSpeedX32 = new QAction(QIcon::fromTheme("media-seek-forward"), "x32", this);
    setSpeedX64 = new QAction(QIcon::fromTheme("media-seek-forward"), "x64", this);
}

void MainWindow::createControlBar()
{
    controlBar = new QToolBar(this);
    positionLabel = new QLabel("-:-:-", this);
    timelineSlider = new QSlider(Qt::Horizontal, controlBar);
    durationLabel = new QLabel("-:-:-", this);

    connect(player, &GstPlayer::seekableChanged, positionLabel, &QLabel::setEnabled);
    connect(player, &GstPlayer::seekableChanged, timelineSlider, &QSlider::setEnabled);
    connect(player, &GstPlayer::seekableChanged, durationLabel, &QLabel::setEnabled);

    connect(player, &GstPlayer::positionChanged, positionLabel, [&](auto pos){positionLabel->setText(pos.toString());});
    connect(player, &GstPlayer::durationChanged, durationLabel, [&](auto dur){durationLabel->setText(dur.toString());});
    
    speedButton = new QToolButton(controlBar);
    speedButton->setMenu(speedMenu);
    speedButton->setPopupMode(QToolButton::InstantPopup);
    speedButton->setDefaultAction(setSpeedX1);

    controlBar->setFloatable(false);
    controlBar->setMovable(false);
    controlBar->addAction(playAction);
    controlBar->addAction(stopAction);
    controlBar->addAction(recordAction);
    controlBar->addWidget(positionLabel);
    controlBar->addWidget(timelineSlider);
    controlBar->addWidget(durationLabel);
    controlBar->addWidget(speedButton);
    controlBar->addAction(fullscreenAction);

    addToolBar(Qt::BottomToolBarArea, controlBar);
}

void MainWindow::createContextMenu()
{
    contextMenu = new QMenu(this);
    contextMenu->addAction(openFileAction);
    contextMenu->addAction(openRTSPAction);
    contextMenu->addSeparator();
    contextMenu->addAction(quitAction);
}

void MainWindow::createSpeedMenu()
{
    speedMenu = new QMenu(this);
    speedMenu->addAction(setSpeedX_25);
    speedMenu->addAction(setSpeedX_50);
    speedMenu->addAction(setSpeedX_75);
    speedMenu->addAction(setSpeedX1);
    speedMenu->addAction(setSpeedX2);
    speedMenu->addAction(setSpeedX4);
    speedMenu->addAction(setSpeedX8);
    speedMenu->addAction(setSpeedX16);
    speedMenu->addAction(setSpeedX32);
    speedMenu->addAction(setSpeedX64);
}

void MainWindow::turnFullscreen()
{
    isFullScreen() ? showNormal() : showFullScreen();
}
