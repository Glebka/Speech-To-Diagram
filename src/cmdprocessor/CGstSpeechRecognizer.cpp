#include "CGstSpeechRecognizer.hpp"

const std::string CGstSpeechRecognizer::ELEMENT_NAME = "pocketsphinx";
const std::string CGstSpeechRecognizer::HMM_PARAM = "hmm";
const std::string CGstSpeechRecognizer::DICT_PARAM = "dict";
const std::string CGstSpeechRecognizer::LM_PARAM = "lm";

const std::string CGstSpeechRecognizer::PARTIAL_RESULT_EVENT = "partial_result";
const std::string CGstSpeechRecognizer::RESULT_EVENT = "result";

CGstSpeechRecognizer::CGstSpeechRecognizer()
    : CGstElement( gst_element_factory_make( ELEMENT_NAME.c_str(), NULL ) )
{
    g_signal_connect ( mElement, PARTIAL_RESULT_EVENT.c_str(), G_CALLBACK ( partialResultHandler ), this );
    g_signal_connect ( mElement, RESULT_EVENT.c_str(), G_CALLBACK ( resultHandler ), this );
}

void CGstSpeechRecognizer::setAcousticModelDirectoryPath(const std::string &path)
{
    g_object_set( G_OBJECT( mElement ), HMM_PARAM.c_str(), path.c_str(), NULL );
}

void CGstSpeechRecognizer::setDictionaryFilePath(const std::string &path)
{
    g_object_set( G_OBJECT( mElement ), DICT_PARAM.c_str(), path.c_str(), NULL );
}

void CGstSpeechRecognizer::setLanguageModelFilePath(const std::string &path)
{
    g_object_set( G_OBJECT( mElement ), LM_PARAM.c_str(), path.c_str(), NULL );
}

std::string CGstSpeechRecognizer::getAcousticModelDirectoryPath() const
{
    gchar *paramValue;

    g_object_get( G_OBJECT( mElement ), HMM_PARAM.c_str(), &paramValue, NULL );

    std::string result( paramValue );

    g_free( paramValue );

    return result;
}

std::string CGstSpeechRecognizer::getDictionaryFilePath() const
{
    gchar *paramValue;

    g_object_get( G_OBJECT( mElement ), DICT_PARAM.c_str(), &paramValue, NULL );

    std::string result( paramValue );

    g_free( paramValue );
}

std::string CGstSpeechRecognizer::getLanguageModelFilePath() const
{
    gchar *paramValue;

    g_object_get( G_OBJECT( mElement ), LM_PARAM.c_str(), &paramValue, NULL );

    std::string result( paramValue );

    g_free( paramValue );
}

void CGstSpeechRecognizer::partialResultHandler(GstElement *object, gchararray arg0, gchararray arg1, gpointer data)
{
    g_print ( "\nPartial: %s\n", arg0 );
}

void CGstSpeechRecognizer::resultHandler(GstElement *object, gchararray arg0, gchararray arg1, gpointer data)
{
    g_print ( "\nResult: %s\n", arg0 );
}
