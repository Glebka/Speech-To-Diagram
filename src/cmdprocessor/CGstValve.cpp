#include "CGstValve.hpp"

const std::string CGstValve::ELEMENT_NAME = "valve";
const std::string CGstValve::DROP_PARAM = "drop";

CGstValve::CGstValve()
    : CGstElement( gst_element_factory_make( ELEMENT_NAME.c_str(), NULL ) )
{
}

void CGstValve::open()
{
    gboolean isDrop = TRUE;
    g_object_set( G_OBJECT( mElement ), DROP_PARAM.c_str(), isDrop, NULL );
}

void CGstValve::close()
{
    gboolean isDrop = FALSE;
    g_object_set( G_OBJECT( mElement ), DROP_PARAM.c_str(), isDrop, NULL );
}
