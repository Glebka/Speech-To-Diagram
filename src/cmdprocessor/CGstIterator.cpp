#include "CGstIterator.hpp"

CGstIterator::CGstIterator(GstIterator *iterator)
    : mIterator( iterator )
{
}

CGstIterator::~CGstIterator()
{
    gst_iterator_free( mIterator );
}
