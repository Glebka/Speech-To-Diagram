#include "CGstAudioConverter.hpp"

const std::string CGstAudioConverter::ELEMENT_NAME = "audioconvert";

CGstAudioConverter::CGstAudioConverter()
    :CGstElement( gst_element_factory_make( ELEMENT_NAME.c_str(), NULL ) )
{
}
