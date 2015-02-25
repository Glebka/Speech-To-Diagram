#include <stdio.h>
#include <cphonetisaurus.h>

int main( void )
{
   HPhonetisaurus* ph = phonetisaurus_create( "cmu.fst", 0, 0.6, 0.85, 0.72, 6 );
   phonetisaurus_phonetise( ph, "HELLO WORLD" );
   phonetisaurus_destroy( ph );
   return 0;
}
