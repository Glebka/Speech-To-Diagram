#ifndef CGSTPAD_HPP
#define CGSTPAD_HPP

#include <glib.h>
#include <gst/gst.h>
#include <string>

class CGstPad
{
public:
    CGstPad( GstElement *element, bool isSrcPad );
    ~CGstPad();

    void block();
    void release();

private:
    CGstPad( const CGstPad &p );
    GstPad *mPad;

private:
    static const std::string SINK;
    static const std::string SOURCE;
};

#endif // CGSTPAD_HPP
