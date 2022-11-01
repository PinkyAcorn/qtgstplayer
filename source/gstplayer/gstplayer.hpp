#pragma once

// 3rdparty headers
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QTime>
#include <QUrl>

#include <gst/gst.h>
#include <gst/gl/gl.h>
#include <gst/video/video.h>

class GstPlayer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    using QOpenGLWidget::QOpenGLWidget;

    GstPlayer(QWidget *parent = nullptr);
    ~GstPlayer();
    const QUrl &getSource();
    bool getSeekable();

signals:
    void sourceChanged(const QUrl &new_source);
    void positionChanged(const QTime &new_position);
    void durationChanged(const QTime &new_duration);
    void seekableChanged(bool new_seekable);

public slots:
    void open(const QUrl& new_source);
    void play();
    void pause();
    void seek(const QTime &position);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    QUrl source;
    bool seekable;

    GstPipeline *pipeline;
    QOpenGLShaderProgram program;
    GLuint texture;

    void initializeShaders();
    void initializeGst();
    void initializePipeline();

    static inline GMutex app_lock;
    static inline GCond app_cond;

    static gboolean on_supply_gldisplay(GstBus *bus, GstMessage *msg, gpointer data);
    static gboolean on_supply_glcontext(GstBus *bus, GstMessage *msg, gpointer data);
    static gboolean on_client_draw(GstElement *glsink, GstGLContext *context, GstSample *sample, gpointer data);
    static gboolean on_redraw_widget(gpointer data);
};
