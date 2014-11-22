#ifndef CGSTQUEUE_HPP
#define CGSTQUEUE_HPP

#include "CGstElement.hpp"

class CGstQueue : public CGstElement
{
public:
    CGstQueue();

private:
    static const std::string ELEMENT_NAME;
};

#endif // CGSTQUEUE_HPP
