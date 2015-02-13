#include "CGstGoogleSpeech.hpp"

const std::string CGstGoogleSpeech::ELEMENT_NAME = "googlespeech";
const std::string CGstGoogleSpeech::KEY_PARAM = "key";
const std::string CGstGoogleSpeech::LANG_PARAM = "lang";
const std::string CGstGoogleSpeech::APP_PARAM = "application";

CGstGoogleSpeech::CGstGoogleSpeech()
    : CGstElement( gst_element_factory_make( ELEMENT_NAME.c_str(), NULL ) )
{
    g_signal_connect ( mElement, "result", G_CALLBACK ( resultHandler ), this );
    g_signal_connect ( mElement, "error", G_CALLBACK ( errorHandler ), this );
}

CGstGoogleSpeech::~CGstGoogleSpeech()
{

}

void CGstGoogleSpeech::setKey(const std::string &key)
{
    g_object_set( G_OBJECT( mElement ), KEY_PARAM.c_str(), key.c_str(), NULL );
}

void CGstGoogleSpeech::setLang(const std::string &lang)
{
    g_object_set( G_OBJECT( mElement ), LANG_PARAM.c_str(), lang.c_str(), NULL );
}

void CGstGoogleSpeech::setApp(const std::string &app)
{
    g_object_set( G_OBJECT( mElement ), APP_PARAM.c_str(), app.c_str(), NULL );
}

std::string CGstGoogleSpeech::getKey() const
{
    return "";
}

std::string CGstGoogleSpeech::getLang() const
{
    return "";
}

std::string CGstGoogleSpeech::getApp() const
{
    return "";
}

void CGstGoogleSpeech::resultHandler(GstElement *object, gchararray arg0, gpointer data)
{
    CGstGoogleSpeech *self = ( CGstGoogleSpeech * ) data;
    g_print ( "\n%s->Result: %s\n", self->getName().c_str(), arg0 );
    //g_free( arg0 );
}

void CGstGoogleSpeech::errorHandler(GstElement *object, gint arg0, gchararray arg1, gpointer data)
{

}

