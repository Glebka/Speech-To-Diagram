#ifndef CGSTELEMENT_HPP
#define CGSTELEMENT_HPP

#include <glib.h>
#include <gst/gst.h>

class CGstElement
{
public:
    CGstElement( GstElement *el = 0 );
    virtual ~CGstElement() = 0;

protected:
    GstElement *mElement;
};

#endif // CGSTELEMENT_HPP
