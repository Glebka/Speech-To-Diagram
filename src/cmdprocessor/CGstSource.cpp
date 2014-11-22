#include "CGstSource.hpp"

CGstSource::CGstSource()
    : CGstElement( gst_element_factory_make( AUDIO_SOURCE_PLUGIN, NULL ) )
{
}

CGstSource::~CGstSource()
{

}
