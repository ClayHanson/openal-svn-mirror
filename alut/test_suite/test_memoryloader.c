
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <AL/alut.h>

/*
  This program loads and plays a variety of files.
*/

int main ( int argc, char **argv )
{
  ALuint memBuffer, memSource ;
  FILE *fd ;
  char buffer [ 100000 ] ;
  int  length ;

  alutInit ( & argc, argv ) ;

  fd = fopen ( "file1.wav", "rb" ) ;
  length = fread ( buffer, 1, 10000, fd ) ;
  fclose ( fd ) ;
  
  memBuffer = alutCreateBufferFromFileImage ( buffer, length ) ;

  if ( memBuffer == 0 )
    fprintf(stderr, "Error loading wav file: '%s'\n",
                    alutGetErrorString ( alutGetError () ) ) ;

  alGenSources ( 1, &memSource ) ;
  alSourcei ( memSource, AL_BUFFER, memBuffer ) ;

  alSourcePlay ( memSource ) ; sleep ( 2 ) ;

  alutExit () ;
  exit ( -1 ) ;
}


