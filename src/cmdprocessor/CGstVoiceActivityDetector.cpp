#include "CGstVoiceActivityDetector.hpp"

const std::string CGstVoiceActivityDetector::ELEMENT_NAME = "vader";
const std::string CGstVoiceActivityDetector::AUTO_THRESHOLD_PARAM = "auto-threshold";

const std::string CGstVoiceActivityDetector::VADER_START_EVENT = "vader-start";
const std::string CGstVoiceActivityDetector::VADER_STOP_EVENT = "vader-stop";

CGstVoiceActivityDetector::CGstVoiceActivityDetector()
    : CGstElement( gst_element_factory_make( ELEMENT_NAME.c_str(), NULL ) )
{
    g_signal_connect ( mElement , VADER_START_EVENT.c_str(), G_CALLBACK ( vaderStartHandler ), this );
    g_signal_connect ( mElement, VADER_STOP_EVENT.c_str(), G_CALLBACK ( vaderStopHandler ), this );
}

void CGstVoiceActivityDetector::setAutoThreshold(bool value)
{
    g_object_set( G_OBJECT( mElement ), AUTO_THRESHOLD_PARAM.c_str(), value, NULL );
}

bool CGstVoiceActivityDetector::isAutoThreshold()
{
    bool result = false;
    g_object_get( G_OBJECT( mElement ), AUTO_THRESHOLD_PARAM.c_str(), &result, NULL);
    return result;
}

void CGstVoiceActivityDetector::vaderStartHandler( GstElement *object, guint64 arg0, gpointer data )
{
    g_print( VADER_START_EVENT.c_str() );
}

void CGstVoiceActivityDetector::vaderStopHandler( GstElement *object, guint64 arg0, gpointer data )
{
    g_print( VADER_STOP_EVENT.c_str() );
}
