#include "CGstQueue.hpp"

const std::string CGstQueue::ELEMENT_NAME = "queue";

CGstQueue::CGstQueue()
    : CGstElement( gst_element_factory_make( ELEMENT_NAME.c_str(), NULL ) )
{
}
