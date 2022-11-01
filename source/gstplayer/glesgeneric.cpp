#include <gst/gst.h>

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *)data;

    switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_EOS:
        g_print("End-of-stream\n");
        g_main_loop_quit(loop);
        break;
    case GST_MESSAGE_ERROR:
    {
        gchar *debug = NULL;
        GError *err = NULL;

        gst_message_parse_error(msg, &err, &debug);

        g_print("Error: %s\n", err->message);
        g_error_free(err);

        if (debug)
        {
            g_print("Debug deails: %s\n", debug);
            g_free(debug);
        }

        g_main_loop_quit(loop);
        break;
    }
    default:
        break;
    }

    return TRUE;
}

// client reshape callback
static gboolean reshapeCallback(void *gl_sink, void *gl_ctx, GLuint width, GLuint height, gpointer data)
{
    // std::cout << "Reshape: width=" << width << " height=" << height << "\n";
    glViewport(0, 0, width, height);
    return TRUE;
}

// client draw callback
static gboolean drawCallback(void *gl_sink, void *gl_ctx, GLuint texture, GLuint width, GLuint height, gpointer data)
{
    // std::cout << "draw:" << vertexShader << ":" << fragmentShader << ":" << programObject << ":" << linked << "\n";
    if (!linked)
    {
        initGL();
    }

    GLfloat vVertices[] = {
        -0.5f, 0.5f, 0.0f,  // Position 0
        0.0f, 0.0f,         // TexCoord 0
        -0.5f, -0.5f, 0.0f, // Position 1
        0.0f, 1.0f,         // TexCoord 1
        0.5f, -0.5f, 0.0f,  // Position 2
        1.0f, 1.0f,         // TexCoord 2
        0.5f, 0.5f, 0.0f,   // Position 3, skewed a bit
        1.0f, 0.0f          // TexCoord 3
    };
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(programObject);
    // Load the vertex position
    GLint positionLoc = glGetAttribLocation(programObject, "a_position");
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vVertices);
    // Load the texture coordinate
    GLint texCoordLoc = glGetAttribLocation(programObject, "a_texCoord");
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3]);
    glEnableVertexAttribArray(positionLoc);
    glEnableVertexAttribArray(texCoordLoc);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set the texture sampler to texture unit 0
    GLint tex = glGetUniformLocation(programObject, "tex");
    glUniform1i(tex, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    return false;
}

gint main(gint argc, gchar *argv[])
{
    GstStateChangeReturn ret;
    GstElement *pipeline, *videosrc, *glimagesink;

    GMainLoop *loop;
    GstBus *bus;

    /* initialization */
    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);

    /* create elements */
    pipeline = gst_pipeline_new("pipeline");

    /* watch for messages on the pipeline's bus (note that this will only
     * work like this when a GLib main loop is running) */
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    /* create elements */
    videosrc = gst_element_factory_make("videotestsrc", "videotestsrc0");
    glimagesink = gst_element_factory_make("glimagesink", "glimagesink0");

    if (!videosrc || !glimagesink)
    {
        g_print("one element could not be found \n");
        return -1;
    }

    /* change video source caps */
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                        "format", G_TYPE_STRING, "RGB",
                                        "width", G_TYPE_INT, 320,
                                        "height", G_TYPE_INT, 240,
                                        "framerate", GST_TYPE_FRACTION, 25, 1,
                                        NULL);

    /* configure elements */
    g_object_set(G_OBJECT(videosrc), "num-buffers", 400, NULL);
    g_signal_connect(G_OBJECT(glimagesink), "client-reshape", G_CALLBACK(reshapeCallback), NULL);
    g_signal_connect(G_OBJECT(glimagesink), "client-draw", G_CALLBACK(drawCallback), NULL);(G_OBJECT(glimagesink), "client-reshape", G_CALLBACK(reshapeCallback), NULL);
    g_signal_connect(G_OBJECT(glimagesink), "client-draw", G_CALLBACK(drawCallback), NULL);

    /* add elements */
    gst_bin_add_many(GST_BIN(pipeline), videosrc, glimagesink, NULL);

    /* link elements */
    gboolean link_ok = gst_element_link_filtered(videosrc, glimagesink, caps);
    gst_caps_unref(caps);
    if (!link_ok)
    {
        g_warning("Failed to link videosrc to glimagesink!\n");
        return -1;
    }

    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_print("Failed to start up pipeline!\n");

        /* check if there is an error message with details on the bus */
        GstMessage *msg = gst_bus_poll(bus, GST_MESSAGE_ERROR, 0);
        if (msg)
        {
            GError *err = NULL;

            gst_message_parse_error(msg, &err, NULL);
            g_print("ERROR: %s\n", err->message);
            g_error_free(err);
            gst_message_unref(msg);
        }
        return -1;
    }

    // run loop
    g_main_loop_run(loop);

    /* clean up */
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}
