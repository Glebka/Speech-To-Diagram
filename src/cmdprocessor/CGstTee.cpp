#include "CGstTee.hpp"

const std::string CGstTee::ELEMENT_NAME = "tee";
const std::string CGstTee::PAD_TEMPLATE = "src%d";

CGstTee::CGstTee()
    : CGstElement( gst_element_factory_make( ELEMENT_NAME.c_str(), NULL ) )
    , mPadTemplate( gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS ( mElement ), PAD_TEMPLATE.c_str() ) )
{
}

CGstTee::~CGstTee()
{
    releaseAllRequestedPads();
}

CGstPad *CGstTee::requestSrcPad()
{
    GstPad *pad = gst_element_request_pad( mElement, mPadTemplate, NULL, NULL);
    CGstPad *cpad = new CGstPad( pad );
    return cpad;
}

void CGstTee::releaseRequestedPad( CGstPad *pad )
{
    gst_element_release_request_pad ( mElement, pad->mPad );
    delete pad;
}

void CGstTee::releaseAllRequestedPads()
{
    std::map<std::string, CGstPad *>::iterator it = mRequestedPadsMap.begin();
    CGstPad *pad = 0;

    while ( it != mRequestedPadsMap.end() )
    {
        pad = it->second;

        releaseRequestedPad( pad );

        it++;
    }

    mRequestedPadsMap.clear();
}

CGstPad &CGstTee::getSrcPad()
{
    return const_cast<CGstPad &>( CGstElement::INVALID_PAD );
}

bool CGstTee::link( CGstElement &other )
{
    CGstPad *srcPad = requestSrcPad();
    CGstPad & otherSinkPad = other.getSinkPad();

    bool result = srcPad->link( otherSinkPad );

    if ( !result )
    {
        releaseRequestedPad( srcPad );
    }

    if ( result )
    {
        mRequestedPadsMap[ other.getName() ] = srcPad;
    }

    return result;
}

bool CGstTee::unlink(CGstElement &other)
{
    std::map<std::string, CGstPad *>::iterator it = mRequestedPadsMap.find( other.getName() );
    bool result = ( it != mRequestedPadsMap.end() );

    if ( result )
    {
        CGstPad *pad = it->second;
        pad->unlink( other.getSinkPad() );
        releaseRequestedPad( pad );
        mRequestedPadsMap.erase( it->first );
    }

    return result;
}

void CGstTee::lockSrcPadLinkedWith( CGstElement &other )
{
    std::map<std::string, CGstPad *>::iterator it = mRequestedPadsMap.find( other.getName() );
    bool result = ( it != mRequestedPadsMap.end() );

    if ( result )
    {
        CGstPad *pad = it->second;
        pad->lock();
        other.getSinkPad().sendEosEvent();
    }
}

void CGstTee::unlockSrcPadLinkedWith( CGstElement &other )
{
    std::map<std::string, CGstPad *>::iterator it = mRequestedPadsMap.find( other.getName() );
    bool result = ( it != mRequestedPadsMap.end() );

    if ( result )
    {
        CGstPad *pad = it->second;
        pad->unlock();
    }
}
