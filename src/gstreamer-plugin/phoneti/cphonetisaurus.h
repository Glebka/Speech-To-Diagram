#ifndef CPHONETISAURUS
#define CPHONETISAURUS

#include "phonetisaurus_export.h"

struct PHONETISAURUS_EXPORT HPhonetisaurus;
typedef struct HPhonetisaurus HPhonetisaurus;

HPhonetisaurus* PHONETISAURUS_EXPORT phonetisaurus_create( const char* _g2pmodel_file, int _mbrdecode, float _alpha, float _precision, float _ratio, int _order );
void PHONETISAURUS_EXPORT phonetisaurus_destroy( HPhonetisaurus* v );

char* PHONETISAURUS_EXPORT phonetisaurus_phonetise( HPhonetisaurus* v, const char* word );

#endif // CPHONETISAURUS

