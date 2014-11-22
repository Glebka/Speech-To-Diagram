#ifndef CGSTVALVE_HPP
#define CGSTVALVE_HPP

#include "CGstElement.hpp"

class CGstValve : public CGstElement
{

public:
    CGstValve();

    void open();
    void close();

private:
    static const std::string ELEMENT_NAME;
    static const std::string DROP_PARAM;
};

#endif // CGSTVALVE_HPP
