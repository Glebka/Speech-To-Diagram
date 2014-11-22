#ifndef CGSTPIPELINE_HPP
#define CGSTPIPELINE_HPP

#include <list>

#include "CGstElement.hpp"
#include "CMainLoop.hpp"

class CGstPipeline : public CGstElement
{
public:
    CGstPipeline( CMainLoop &loop );

    virtual ~CGstPipeline();

    bool addElement( CGstElement *element);

    bool removeElement( CGstElement *element);

    bool linkAllElements();

    /*bool linkPair( CGstElement &first, CGstElement &second );
    void unlinkPair( CGstElement &first, CGstElement &second );*/

    void start();

    void pause();

    void stop();

    // CGstElement interface
public:
    virtual CGstPad &getSrcPad();
    virtual CGstPad &getSinkPad();
    virtual bool link(CGstElement &other);
    virtual bool unlink(CGstElement &other);

private:

    typedef std::list<CGstElement *>::iterator GstElemPtrIter;

    void setBusHandler();

    CMainLoop &mLoop;
    std::list<CGstElement *> mElements;

};

#endif // CGSTPIPELINE_HPP
