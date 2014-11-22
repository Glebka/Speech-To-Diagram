#ifndef CGSTELEMENT_HPP
#define CGSTELEMENT_HPP

#include <glib.h>
#include <gst/gst.h>
#include <string>

#include "CGstPad.hpp"

class CGstElement
{
    friend class CGstPipeline;

public:
    CGstElement( GstElement *el = 0 );
    virtual ~CGstElement() = 0;    

    CGstPad &getSrcPad();
    CGstPad &getSinkPad();

    void setState( GstState state );

    std::string getName() const;

protected:
    GstElement *mElement;
    std::string mName;

    CGstPad mSrcPad;
    CGstPad mSinkPad;
};

#endif // CGSTELEMENT_HPP
