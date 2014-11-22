#include "CGstFakeSink.hpp"

CGstFakeSink::CGstFakeSink()
    : CGstElement( gst_element_factory_make ("fakesink", NULL) )
{
}

CGstFakeSink::~CGstFakeSink()
{
}
