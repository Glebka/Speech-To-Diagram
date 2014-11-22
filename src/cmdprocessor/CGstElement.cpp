#include "CGstElement.hpp"

const std::string CGstElement::SINK = "sink";
const std::string CGstElement::SOURCE = "src";

const CGstPad CGstElement::INVALID_PAD;

CGstElement::~CGstElement()
{
    gst_object_unref( GST_OBJECT( mElement ) );
}


CGstElement::CGstElement( GstElement *el )
    : mElement( el )
    , mSinkPad( el, ( char * ) SINK.c_str() )
    , mSrcPad( el, ( char * ) SOURCE.c_str() )
{
    gchar *name = gst_element_get_name( mElement );
    mName.append( name );
    g_free( name );
}

CGstPad &CGstElement::getSrcPad()
{
    return mSrcPad;
}

CGstPad &CGstElement::getSinkPad()
{
    return mSinkPad;
}

void CGstElement::setState( GstState state )
{
    gst_element_set_state( mElement, state );
}

bool CGstElement::link(CGstElement &other)
{
    return getSrcPad().link( other.getSinkPad() );
}

bool CGstElement::unlink(CGstElement &other)
{
    return getSrcPad().unlink( other.getSinkPad() );
}

std::string CGstElement::getName() const
{
    return mName;
}

GstElement *CGstElement::ref()
{
    return ( GstElement * )gst_object_ref( mElement );
}
