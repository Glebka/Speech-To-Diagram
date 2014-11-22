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

    virtual CGstPad &getSrcPad();
    virtual CGstPad &getSinkPad();

    void setState( GstState state );

    virtual bool link( CGstElement &other );
    virtual bool unlink( CGstElement &other );

    std::string getName() const;

protected:

    GstElement *ref();

    GstElement *mElement;
    std::string mName;

    CGstPad mSrcPad;
    CGstPad mSinkPad;

protected:
    static const CGstPad INVALID_PAD;

private:
    static const std::string SINK;
    static const std::string SOURCE;

};

#endif // CGSTELEMENT_HPP
