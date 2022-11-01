#pragma once

// 3rdparty headers
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QSlider>
#include <QContextMenuEvent>
#include <QPushButton>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QLabel>

// local headers
#include "gstplayer/gstplayer.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    GstPlayer *player;

    QMenu *contextMenu;
    QAction *openFileAction;
    QAction *openRTSPAction;
    QAction *quitAction;

    QToolBar *controlBar;
    QAction *playAction;
    QAction *pauseAction;
    QAction *recordAction;
    QAction *stopAction;
    QLabel *positionLabel;
    QSlider *timelineSlider;
    QLabel *durationLabel;
    
    QMenu *speedMenu;
    QToolButton *speedButton;
    QAction *setSpeedX_25;
    QAction *setSpeedX_50;
    QAction *setSpeedX_75;
    QAction *setSpeedX1;
    QAction *setSpeedX2;
    QAction *setSpeedX4;
    QAction *setSpeedX8;
    QAction *setSpeedX16;
    QAction *setSpeedX32;
    QAction *setSpeedX64;

    QAction *fullscreenAction;

    void createActions();
    void createControlBar();
    void createContextMenu();
    void createSpeedMenu();
    void turnFullscreen();
};
