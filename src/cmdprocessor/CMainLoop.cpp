#include "CMainLoop.hpp"

CMainLoop::CMainLoop(int argc, char *argv[])
    : mLoop( g_main_loop_new (NULL, FALSE) )
{
    gst_init ( &argc, &argv );
}

CMainLoop::~CMainLoop()
{
    g_main_loop_unref( mLoop );
}

void CMainLoop::run()
{
    g_main_loop_run( mLoop );
}

void CMainLoop::quit()
{
    g_main_loop_quit( mLoop );
}

bool CMainLoop::isRunning()
{
    return g_main_loop_is_running( mLoop );
}

void CMainLoop::startTimer(  int seconds, GSourceFunc callback )
{
     g_timeout_add_seconds( seconds, callback, mLoop );
}
