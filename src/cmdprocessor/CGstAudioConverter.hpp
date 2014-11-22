#ifndef CGSTAUDIOCONVERTER_HPP
#define CGSTAUDIOCONVERTER_HPP

#include <glib.h>
#include <gst/gst.h>
#include <string>

#include "CGstElement.hpp"

class CGstAudioConverter : public CGstElement
{
public:
    CGstAudioConverter();
private:
    static const std::string ELEMENT_NAME;
};

#endif // CGSTAUDIOCONVERTER_HPP
