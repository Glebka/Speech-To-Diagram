#ifndef CGSTGOOGLESPEECH_HPP
#define CGSTGOOGLESPEECH_HPP

#include <glib.h>
#include <gst/gst.h>
#include "CGstElement.hpp"

class CGstGoogleSpeech : public CGstElement
{
public:
    CGstGoogleSpeech();
    ~CGstGoogleSpeech();

    void setKey( const std::string&  key );
    void setLang( const std::string& lang );
    void setApp( const std::string& app );

    std::string getKey() const;
    std::string getLang() const;
    std::string getApp() const;

private:
    static void resultHandler( GstElement *object, gchararray arg0, gpointer data );
    static void errorHandler( GstElement *object, gint arg0, gchararray arg1, gpointer data );

private:
    static const std::string ELEMENT_NAME;
    static const std::string KEY_PARAM;
    static const std::string LANG_PARAM;
    static const std::string APP_PARAM;
};

#endif // CGSTGOOGLESPEECH_HPP
