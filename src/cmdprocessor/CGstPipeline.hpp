#ifndef CGSTPIPELINE_HPP
#define CGSTPIPELINE_HPP

#include "CGstElement.hpp"
#include "CMainLoop.hpp"

class CGstPipeline : public CGstElement
{
public:
    CGstPipeline( CMainLoop &loop );

    virtual ~CGstPipeline();

    bool addElement( CGstElement *element);

    bool removeElement( CGstElement *element);

    bool linkElements();

    void start();

    void pause();

    void stop();

private:

    void setBusHandler();

    CMainLoop &mLoop;
};

#endif // CGSTPIPELINE_HPP
