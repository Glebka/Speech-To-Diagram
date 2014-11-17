#include "CGstElement.hpp"

CGstElement::~CGstElement()
{
    gst_object_unref( mElement );
}


CGstElement::CGstElement( GstElement *el )
    : mElement( el )
{

}
