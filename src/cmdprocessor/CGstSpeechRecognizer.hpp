#ifndef CGSTSPEECHRECOGNIZER_HPP
#define CGSTSPEECHRECOGNIZER_HPP

#include <glib.h>
#include <gst/gst.h>
#include <string>

#include "CGstElement.hpp"

class CGstSpeechRecognizer : public CGstElement
{
public:
    CGstSpeechRecognizer();

    void setAcousticModelDirectoryPath( const std::string &path );

    void setDictionaryFilePath( const std::string &path );

    void setLanguageModelFilePath( const std::string &path );

    std::string getAcousticModelDirectoryPath() const;

    std::string getDictionaryFilePath() const;

    std::string getLanguageModelFilePath() const;

private:

    static void partialResultHandler( GstElement *object, gchararray arg0, gchararray arg1, gpointer data );
    static void resultHandler( GstElement *object, gchararray arg0, gchararray arg1, gpointer data );

private:
    static const std::string ELEMENT_NAME;
    static const std::string HMM_PARAM;
    static const std::string DICT_PARAM;
    static const std::string LM_PARAM;

    static const std::string PARTIAL_RESULT_EVENT;
    static const std::string RESULT_EVENT;
};

#endif // CGSTSPEECHRECOGNIZER_HPP
