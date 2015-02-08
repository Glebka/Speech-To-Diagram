#include "CGstWavenc.hpp"

CGstWavenc::CGstWavenc()
    : CGstElement( gst_element_factory_make ("wavenc", NULL) )
{

}

CGstWavenc::~CGstWavenc()
{

}

