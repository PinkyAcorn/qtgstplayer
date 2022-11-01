// Сделать стандартный плеер:
// - пауза/старт/стоп
// - выбор файла
// - изменение от 0,25 до 64x.
// - При высоких скоростях процессор не должен грузиться.
// - Сделать превью при ключевом кадре, на выбор, либо каждый ключевой кадр, либо на выбор. Выбирается файл видео, рядом создаётся папка с таким же именем и скрины ключевых кадров в порядке таймстампов или 1, 2, 3
// - RTSP потоки
// - запись видео

// 3rdparty headers
#include <QApplication>
#include <QHBoxLayout>
#include <QSurfaceFormat>
#include <QSurfaceFormat>
#include <QCommandLineOption>
#include <QCommandLineParser>

// local headers
#include "mainwindow.hpp"
#include "gstplayer/gstplayer.hpp"

int main(int argc, char *argv[])
{
    // https://doc.qt.io/qt-6/qsurfaceformat.html#setDefaultFormat
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::RenderableType::OpenGLES);
    format.setVersion(2, 0);
    QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc, argv);

    MainWindow window;
    window.show();
    return app.exec();
}
