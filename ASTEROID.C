//          Atari Asteroid emulator ver 1 by alan daly
//              by A.Daly (c) jan 2000

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <malloc.h>
#include <dos.h>

#include "ASTEROID.H"
#include "6502.h"
#include "allegro.h"

unsigned char *mem = NULL;
BITMAP *memory_bitmap;

volatile int sup = 0;           // nasty timer interupt for bmp flips
                                // and audio updates
void sup_inc()                  // timer interupt handler
{                               // only put flag here as more code to scary
sup ++;                         // increment this flag every 1 millisecond
}
END_OF_FUNCTION(sup_inc);





int main(void)
{


unsigned int a ;
unsigned char ch = 0xff;
FILE *fp;

allegro_init();
install_keyboard();
install_timer();                  // start timer
LOCK_VARIABLE(sup);               // lock interupt variable memory
LOCK_FUNCTION(sup_inc);           // lock routine memory
install_int(sup_inc,1);           // start interupt 1 milliseconds

if(set_gfx_mode(GFX_DIRECTX_ACCEL,640,480,0,0) != 0)set_gfx_mode(GFX_SAFE,640,480,0,0);// fullscreen

set_pallete(desktop_pallete);
memory_bitmap = create_bitmap(640,480);

/////////////////////////////////////////////////////////////////////////////////
//           initialise memory for asteroids machine
/////////////////////////////////////////////////////////////////////////////////

mem = malloc(0x8000);
memset(mem,0xff,0x8000);
         // these are important to set memory location clr keys etc
memset(&mem[0x2002],0x00,3);    // 2002 vector done list 2003-2004 keys
memset(&mem[0x2007],0x00,1);    // something to do with self test and tilt
memset(&mem[0x2400],0x00,1);    // clear coin mechanism
memset(&mem[0x2403],0x00,5);    // clear keys
memset(&mem[0x2800],0x00,4);    // dip switches


////////////////////////////////////////////////////////////////////////////////
// load arcade machine roms
/////////////////////////////////////////////////////////////////////////////////

for (a = 0; a < 4; a++){      // this loads roms into memmap

     if((fp = fopen(romlist[a],"rb"))==NULL){
      			printf("cannot a open file %s",romlist[a] );
      			exit(1);
		   				}
                fread(&mem[romaddr[a]], 0x800, 1 ,fp);
      		 fclose(fp);

      }

////////////////////////////////////////////////////////////////////////////////
//  main loop
////////////////////////////////////////////////////////////////////////////////

reset6502();        // initialise mpu

do {

    //for (a = 0; a < 5; a++){
    if (key[KEY_SPACE])    mem[0x2003] = 0x80; else mem[0x2003] = 0x00  ;  // set hyperspace
    if (key[KEY_LCONTROL]) mem[0x2004] = 0x80; else mem[0x2004] = 0x00  ;  // set fire
    if (key[KEY_3])        mem[0x2400] = 0x80; else mem[0x2400] = 0x00  ;  // set coin in
    if (key[KEY_1])        mem[0x2403] = 0x80; else mem[0x2403] = 0x00  ;  // set start 1p
	if (key[KEY_2])        mem[0x2404] = 0x80; else mem[0x2404] = 0x00  ;  // set start 2p
    if (key[KEY_UP])       mem[0x2405] = 0x80; else mem[0x2405] = 0x00  ;  // set thrust
    if (key[KEY_RIGHT])    mem[0x2406] = 0x80; else mem[0x2406] = 0x00  ;  // set rotate r
    if (key[KEY_LEFT])     mem[0x2407] = 0x80; else mem[0x2407] = 0x00  ;  // set rotate l


                exec6502(25000);           // timing cludge as windows clock is so useless
                nmi6502();
                exec6502(25000);
                nmi6502();
                							//}

do{
              		 }while(sup < 3);
               sup = 0;

	}while(!key[KEY_ESC]);


          free(mem);    //clean up for exit
          destroy_bitmap(memory_bitmap);  //free up bmp memory
}
END_OF_MAIN();