#include "CGstPad.hpp"

const std::string CGstPad::SINK = "sink";
const std::string CGstPad::SOURCE = "src";

CGstPad::CGstPad(GstElement *element, bool isSrcPad)
{
    if ( isSrcPad )
    {
        mPad = gst_element_get_static_pad( element, SOURCE.c_str() );
    }
    else
    {
        mPad = gst_element_get_static_pad( element, SINK.c_str() );
    }
}

CGstPad::~CGstPad()
{
    gst_object_unref( mPad );
}

void CGstPad::block()
{
    gst_pad_set_blocked( mPad, true );
}

void CGstPad::release()
{
    gst_pad_set_blocked( mPad, false );
}

CGstPad::CGstPad(const CGstPad &p)
{

}
