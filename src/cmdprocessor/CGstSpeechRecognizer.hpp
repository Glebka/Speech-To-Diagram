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
    static const std::string ELEMENT_NAME;
    static const std::string HMM_PARAM;
    static const std::string DICT_PARAM;
    static const std::string LM_PARAM;
};

#endif // CGSTSPEECHRECOGNIZER_HPP
