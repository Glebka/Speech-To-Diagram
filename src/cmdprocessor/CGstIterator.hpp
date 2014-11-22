#ifndef CGSTITERATOR_HPP
#define CGSTITERATOR_HPP

#include <gst/gst.h>

class CGstIterator
{
public:

    CGstIterator( GstIterator *iterator );

    ~CGstIterator();

private:
    GstIterator *mIterator;
};

#endif // CGSTITERATOR_HPP
