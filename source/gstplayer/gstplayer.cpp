#include <iostream>
#include <string>

// local headers
#include "gstplayer.hpp" // qurl.h conflicts with X.h

// 3rdparty headers
#include <QtGui/private/qeglplatformcontext_p.h>
#include <gst/gl/egl/gstgldisplay_egl.h>


GstPlayer::GstPlayer(QWidget *parent) : QOpenGLWidget(parent), source("/home/pinkyacorn/Videos/Амели.mkv"), seekable(false)
{
    setAttribute(Qt::WA_NoSystemBackground);
}

GstPlayer::~GstPlayer()
{
    makeCurrent();
    doneCurrent();
}

const QUrl &GstPlayer::getSource()
{
    return source;
}

bool GstPlayer::getSeekable()
{
    return seekable;
}

void GstPlayer::open(const QUrl& new_source)
{
    source = new_source;
    // initializePipeline(new_source);

    pause();

    // checkSeekable
    seekable = gst_element_seek_simple(GST_ELEMENT(pipeline), GST_FORMAT_TIME, (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), 0);
    // emit seekableChanged(seekable);
    emit seekableChanged(false);

    // checkDuration
    gint64 duration;
    gst_element_query_duration(GST_ELEMENT(pipeline), GST_FORMAT_TIME, &duration);
    emit durationChanged(QTime::fromMSecsSinceStartOfDay(duration / GST_MSECOND));

    emit sourceChanged(new_source);
}

void GstPlayer::play()
{
    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
}

void GstPlayer::pause()
{
    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PAUSED);
}

void GstPlayer::seek(const QTime &position)
{
    gint64 pos = position.msecsSinceStartOfDay() * GST_MSECOND;
    gst_element_seek_simple(GST_ELEMENT(pipeline), GST_FORMAT_TIME, (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), pos);
}

void GstPlayer::initializeGL()
{
    initializeOpenGLFunctions();
    initializeShaders();
    initializeGst();
}

void GstPlayer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

// https://doc.qt.io/qt-6/qopenglwidget.html#threading
void GstPlayer::paintGL()
{
    GLushort indices[] = {0, 1, 2, 0, 2, 3};
    GLfloat vVertices[] = {
        -1.0f, +1.0f, +0.0f, // Position 0
        +0.0f, +0.0f,        // TexCoord 0
        -1.0f, -1.0f, +0.0f, // Position 1
        +0.0f, +1.0f,        // TexCoord 1
        +1.0f, -1.0f, +0.0f, // Position 2
        +1.0f, +1.0f,        // TexCoord 2
        +1.0f, +1.0f, +0.0f, // Position 3
        +1.0f, +0.0f         // TexCoord 3
    };

    glVertexAttribPointer(program.attributeLocation("a_position"), 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vVertices);
    glVertexAttribPointer(program.attributeLocation("a_texcoord"), 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3]);
    glEnableVertexAttribArray(program.attributeLocation("a_position"));
    glEnableVertexAttribArray(program.attributeLocation("a_texcoord"));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(program.uniformLocation("s_texture"), 0);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    
    glDisableVertexAttribArray(program.attributeLocation("a_position"));
    glDisableVertexAttribArray(program.attributeLocation("a_texcoord"));
}

void GstPlayer::initializeShaders()
{
    program.addShaderFromSourceCode(
        QOpenGLShader::Vertex,
        "#ifdef GL_ES \n"
        "precision highp int; \n"
        "precision highp float; \n"
        "#endif \n"
        "attribute vec4 a_position; \n"
        "attribute vec2 a_texcoord; \n"
        "varying vec2 v_texcoord; \n"
        "void main() \n"
        "{ \n"
        "    gl_Position = a_position; \n"
        "    v_texcoord = a_texcoord; \n"
        "}");

    program.addShaderFromSourceCode(
        QOpenGLShader::Fragment,
        "#ifdef GL_ES \n"
        "precision highp int; \n"
        "precision highp float; \n"
        "#endif \n"
        "uniform sampler2D s_texture; \n"
        "varying vec2 v_texcoord; \n"
        "void main() \n"
        "{ \n"
        "    gl_FragColor = texture2D(s_texture, v_texcoord); \n"
        "}");

    program.link();
    program.bind();
}

void GstPlayer::initializeGst()
{
    gst_init(nullptr, nullptr);

    QNativeInterface::QEGLContext *qeglcontext = context()->nativeInterface<QNativeInterface::QEGLContext>();
    QEGLPlatformContext *qeglpcontext = dynamic_cast<QEGLPlatformContext *>(qeglcontext);
    EGLDisplay egldisplay = qeglpcontext->eglDisplay();
    EGLContext eglcontext = qeglpcontext->eglContext();
    GstGLDisplay *gldisplay = GST_GL_DISPLAY(gst_gl_display_egl_new_with_egl_display(reinterpret_cast<gpointer>(egldisplay)));
    GstGLContext *glcontext = gst_gl_context_new_wrapped(gldisplay, reinterpret_cast<guintptr>(eglcontext), GST_GL_PLATFORM_EGL, GST_GL_API_GLES2);

    // gst_gl_context_activate(glcontext, TRUE);
    // gst_gl_context_fill_info(glcontext, NULL);

    pipeline = GST_PIPELINE(gst_parse_launch(
        "videotestsrc !"
        "glimagesink name=glimagesink0", NULL));

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_signal_watch(bus);
    gst_bus_enable_sync_message_emission(bus);
    g_signal_connect(bus, "sync-message", G_CALLBACK(on_supply_gldisplay), gldisplay);
    g_signal_connect(bus, "sync-message", G_CALLBACK(on_supply_glcontext), glcontext);

    GstElement *glimagesink = gst_bin_get_by_name(GST_BIN(pipeline), "glimagesink0");
    g_signal_connect(G_OBJECT(glimagesink), "client-draw", G_CALLBACK(on_client_draw), reinterpret_cast<gpointer>(this));
    gst_object_unref(glimagesink);
}

void GstPlayer::initializePipeline()
{
    ;
}

gboolean GstPlayer::on_supply_gldisplay(GstBus *bus, GstMessage *msg, gpointer data)
{
    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_NEED_CONTEXT)
    {
        const gchar *context_type;

        gst_message_parse_context_type(msg, &context_type);
        if (g_strcmp0(context_type, GST_GL_DISPLAY_CONTEXT_TYPE) == 0)
        {
            GstGLDisplay *gdisplay = reinterpret_cast<GstGLDisplay *>(data);
            GstContext *display_context = gst_context_new(GST_GL_DISPLAY_CONTEXT_TYPE, TRUE);
            gst_context_set_gl_display(display_context, gdisplay);
            gst_element_set_context(GST_ELEMENT(msg->src), display_context);
            gst_context_unref(display_context);
        }
    }

    return FALSE;
}

gboolean GstPlayer::on_supply_glcontext(GstBus *bus, GstMessage *msg, gpointer data)
{
    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_NEED_CONTEXT)
    {
        const gchar *context_type;

        gst_message_parse_context_type(msg, &context_type);
        if (g_strcmp0(context_type, "gst.gl.app_context") == 0)
        {
            GstGLContext *gcontext = reinterpret_cast<GstGLContext *>(data);
            GstContext *app_context = gst_context_new("gst.gl.app_context", TRUE);
            GstStructure *s = gst_context_writable_structure(app_context);
            gst_structure_set(s, "context", GST_TYPE_GL_CONTEXT, gcontext, NULL);
            gst_element_set_context(GST_ELEMENT(msg->src), app_context);
            gst_context_unref(app_context);
        }
    }

    return FALSE;
}

gboolean GstPlayer::on_client_draw(GstElement *glsink, GstGLContext *context, GstSample *sample, gpointer data)
{
    GstPlayer *player = reinterpret_cast<GstPlayer *>(data);
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstCaps *caps = gst_sample_get_caps(sample);
    GstVideoFrame frame;
    GstVideoInfo info;

    gst_video_info_from_caps(&info, caps);
    gst_video_frame_map(&frame, &info, buffer, (GstMapFlags)(GST_MAP_READ | GST_MAP_GL));
    g_mutex_lock(&app_lock);
    player->texture = *(reinterpret_cast<guint *>(frame.data[0]));
    g_idle_add_full(G_PRIORITY_HIGH, on_redraw_widget, player, NULL); // will exec in GL thread
    g_cond_wait(&app_cond, &app_lock);
    g_mutex_unlock(&app_lock);
    gst_video_frame_unmap(&frame);
    
    return TRUE; // call again
}

gboolean GstPlayer::on_redraw_widget(gpointer data)
{
    GstPlayer *player = reinterpret_cast<GstPlayer *>(data);

    g_mutex_lock(&app_lock);
    player->update();
    g_cond_signal(&app_cond);
    g_mutex_unlock(&app_lock);

    return FALSE;
}
