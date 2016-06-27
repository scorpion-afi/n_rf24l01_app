/*
 ============================================================================
 Name        : test_app.c
 Author      : Ila
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "init_n_rf24l01.h"

//
//======================================================================================================
int main ( void )
{
  char str[] = "Lena I want to make your life more happy.\n";
  int ret;

  ret = init_n_rf24l01();
  if( ret < 0 )
    return 1;

  while( 1 )
  {
    printf( "transmit: %s\n", str );
    transmit_str( str );

    //sleep( 2 );
  }

  return 0;
}
