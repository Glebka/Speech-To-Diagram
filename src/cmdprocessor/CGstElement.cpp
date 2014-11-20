#include "CGstElement.hpp"

CGstElement::~CGstElement()
{
    gst_object_unref( GST_OBJECT( mElement ) );
}


CGstElement::CGstElement( GstElement *el )
    : mElement( el )
{

}
