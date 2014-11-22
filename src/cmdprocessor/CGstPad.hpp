#ifndef CGSTPAD_HPP
#define CGSTPAD_HPP

#include <glib.h>
#include <gst/gst.h>
#include <string>

class CGstPad
{
    friend class CGstTee;
public:

    CGstPad();
    explicit CGstPad( GstPad *pad );
    CGstPad( GstElement *element, gchar *name );
    ~CGstPad();

    std::string getName() const;

    bool isValid();

    void lock();
    void unlock();

    void sendEosEvent();

    bool link( CGstPad &other );
    bool unlink( CGstPad &other );

private:
    CGstPad( const CGstPad &p );
    void prepareName();

    GstPad *mPad;
    std::string mName;

private:
    static const std::string SINK;
    static const std::string SOURCE;
};

#endif // CGSTPAD_HPP
