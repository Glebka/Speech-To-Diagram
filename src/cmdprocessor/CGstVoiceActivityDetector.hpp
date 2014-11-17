#ifndef CGSTVOICEACTIVITYDETECTOR_HPP
#define CGSTVOICEACTIVITYDETECTOR_HPP

#include <glib.h>
#include <gst/gst.h>
#include <string>

#include "CGstElement.hpp"

class CGstVoiceActivityDetector : public CGstElement
{
public:
    CGstVoiceActivityDetector();

    void setAutoThreshold( bool value );

    bool isAutoThreshold();

private:
    static const std::string ELEMENT_NAME;
    static const std::string AUTO_THRESHOLD_PARAM;
};

#endif // CGSTVOICEACTIVITYDETECTOR_HPP
