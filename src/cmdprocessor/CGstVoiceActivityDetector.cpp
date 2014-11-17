#include "CGstVoiceActivityDetector.hpp"

const std::string CGstVoiceActivityDetector::ELEMENT_NAME = "vader";
const std::string CGstVoiceActivityDetector::AUTO_THRESHOLD_PARAM = "auto-threshold";

CGstVoiceActivityDetector::CGstVoiceActivityDetector()
    : CGstElement( gst_element_factory_make( ELEMENT_NAME.c_str() ) )
{
}

void CGstVoiceActivityDetector::setAutoThreshold(bool value)
{
    g_object_set( G_OBJECT(mElement), AUTO_THRESHOLD_PARAM.c_str(), value, NULL );
}

bool CGstVoiceActivityDetector::isAutoThreshold()
{
    bool result = false;
    g_object_get( G_OBJECT(mElement), AUTO_THRESHOLD_PARAM.c_str(), &result, NULL);
    return result;
}
