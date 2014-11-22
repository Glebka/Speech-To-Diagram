#ifndef CGSTAUDIORESAMPLER_HPP
#define CGSTAUDIORESAMPLER_HPP

#include <glib.h>
#include <gst/gst.h>
#include <string>

#include "CGstElement.hpp"

class CGstAudioResampler : public CGstElement
{
public:
    CGstAudioResampler();
private:
    static const std::string ELEMENT_NAME;
};

#endif // CGSTAUDIORESAMPLER_HPP
