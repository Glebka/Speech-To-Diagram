extern "C" {
#include "cphonetisaurus.h"
}
#include "Phonetisaurus.hpp"
#include "util.hpp"

HPhonetisaurus* phonetisaurus_create( const char* _g2pmodel_file, int _mbrdecode, float _alpha, float _precision, float _ratio, int _order )
{
   return reinterpret_cast<HPhonetisaurus*>( new Phonetisaurus( _g2pmodel_file, _mbrdecode, _alpha, _precision, _ratio, _order ) );
}


void phonetisaurus_destroy( HPhonetisaurus* v )
{
   delete reinterpret_cast<Phonetisaurus*>(v);
}


char* phonetisaurus_phonetise( HPhonetisaurus* v, const char* word )
{
   Phonetisaurus* p = reinterpret_cast<Phonetisaurus*>( v );
   std::string sWord( word );
   std::string sep( "" );
   vector<string>   entry = tokenize_entry( &sWord, &sep, p->isyms );

   vector<PathData> result =  p->phoneticize( entry, 1 );
   bool res = p->printPaths( result, 1, "", "" );
   return 0;
}
