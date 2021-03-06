#ifndef CGSTVOICEACTIVITYDETECTOR_HPP
#define CGSTVOICEACTIVITYDETECTOR_HPP

#include <glib.h>
#include <gst/gst.h>
#include <string>

#include "CGstElement.hpp"
#include "CGstPad.hpp"

class CGstVoiceActivityDetector : public CGstElement
{
public:
    CGstVoiceActivityDetector();

    void setAutoThreshold( bool value );

    bool isAutoThreshold();

    CGstPad & getSrcCopyPad();

private:

    static void vaderStartHandler( GstElement* object, guint64 arg0, gpointer data );
    static void vaderStopHandler( GstElement* object, guint64 arg0, gpointer data );

    CGstPad mSrcCopyPad;

private:
    static const std::string ELEMENT_NAME;
    static const std::string AUTO_THRESHOLD_PARAM;

    static const std::string VADER_START_EVENT;
    static const std::string VADER_STOP_EVENT;
    static const std::string VADER_SRC_COPY_PAD;
};

#endif // CGSTVOICEACTIVITYDETECTOR_HPP
