#include "CGstAudioResampler.hpp"

const std::string CGstAudioResampler::ELEMENT_NAME = "audioresample";

CGstAudioResampler::CGstAudioResampler()
    : CGstElement( gst_element_factory_make( ELEMENT_NAME.c_str(), NULL ) )
{
}
