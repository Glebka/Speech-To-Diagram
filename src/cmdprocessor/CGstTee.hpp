#ifndef CGSTTEE_HPP
#define CGSTTEE_HPP

#include <map>
#include "CGstElement.hpp"

class CGstTee : public CGstElement
{
public:
    CGstTee();
    virtual ~CGstTee();

    void releaseAllRequestedPads();

    // CGstElement interface
public:
    virtual CGstPad &getSrcPad();

    virtual bool link( CGstElement &other );
    virtual bool unlink( CGstElement &other );

    void lockSrcPadLinkedWith( CGstElement &other );
    void unlockSrcPadLinkedWith( CGstElement &other );

private:
    CGstPad *requestSrcPad();
    void releaseRequestedPad( CGstPad *pad );

    std::map<std::string, CGstPad *> mRequestedPadsMap;
    GstPadTemplate * mPadTemplate;

private:
    static const std::string ELEMENT_NAME;
    static const std::string PAD_TEMPLATE;
};

#endif // CGSTTEE_HPP
