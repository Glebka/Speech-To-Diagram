#include "CGstElement.hpp"

CGstElement::~CGstElement()
{
    gst_object_unref( GST_OBJECT( mElement ) );
}


CGstElement::CGstElement( GstElement *el )
    : mElement( el )
    , mSinkPad( el, false )
    , mSrcPad( el, true )
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

std::string CGstElement::getName() const
{
    return mName;
}
