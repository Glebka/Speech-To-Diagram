#include "CGstSpeechRecognizer.hpp"

static const std::string CGstSpeechRecognizer::ELEMENT_NAME = "pocketsphinx";
static const std::string CGstSpeechRecognizer::HMM_PARAM = "hmm";
static const std::string CGstSpeechRecognizer::DICT_PARAM = "dict";
static const std::string CGstSpeechRecognizer::LM_PARAM = "lm";

CGstSpeechRecognizer::CGstSpeechRecognizer()
    : CGstElement( gst_element_factory_make( ELEMENT_NAME.c_str() ) )
{
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
