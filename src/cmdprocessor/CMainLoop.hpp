#ifndef CMAINLOOP_HPP
#define CMAINLOOP_HPP

#include <glib.h>
#include <gst/gst.h>

class CMainLoop
{
public:
    CMainLoop( int argc, char *argv[] );
    ~CMainLoop();

    void run();

    void quit();

    bool isRunning();

    void startTimer( int seconds, GSourceFunc callback );

private:
    GMainLoop *mLoop;
};

#endif // CMAINLOOP_HPP
