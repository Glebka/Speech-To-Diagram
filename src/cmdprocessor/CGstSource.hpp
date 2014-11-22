#ifndef CGSTSOURCE_HPP
#define CGSTSOURCE_HPP

#include <glib.h>
#include <gst/gst.h>
#include <config.h>

#include "CGstElement.hpp"

class CGstSource : public CGstElement
{
public:
    CGstSource();
    virtual ~CGstSource();
};

#endif // CGSTSOURCE_HPP
