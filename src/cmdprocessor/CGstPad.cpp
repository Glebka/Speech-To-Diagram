#include "CGstPad.hpp"

CGstPad::CGstPad()
    : mPad( 0 )
{

}

CGstPad::CGstPad( GstPad *pad )
    : mPad( pad )
{
    prepareName();
}

CGstPad::CGstPad(GstElement *element, gchar *name)
{
    mPad = gst_element_get_static_pad( element, name );
    prepareName();
}

CGstPad::~CGstPad()
{
    if ( mPad )
        gst_object_unref( mPad );
}

std::string CGstPad::getName() const
{
    return mName;
}

bool CGstPad::isValid()
{
    return ( mPad != 0 );
}

void CGstPad::lock()
{
    /*if ( mPad )
        gst_pad_set_blocked( mPad, true );*/
}

void CGstPad::unlock()
{
    /*if ( mPad )
        gst_pad_set_blocked( mPad, false );*/
}

void CGstPad::sendEosEvent()
{
    if ( mPad )
        gst_pad_send_event( mPad, gst_event_new_eos() );
}

bool CGstPad::link(CGstPad &other)
{
    bool result = false;

    if ( mPad )
        result = ( gst_pad_link( mPad, other.mPad ) == GST_PAD_LINK_OK );

    return result;
}

bool CGstPad::unlink(CGstPad &other)
{
    return gst_pad_unlink( mPad, other.mPad );
}

CGstPad::CGstPad(const CGstPad &p)
{
}

void CGstPad::prepareName()
{
    if ( mPad )
    {
        gchar *name = gst_pad_get_name( mPad );
        mName.append( name );
        g_free( name );
    }
}
