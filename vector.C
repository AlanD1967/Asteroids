//    Atari XY Vector Graphics Drawing engine emulation
//    by A.daly (c) jan 2000
//    Bit Key:
//    X - X coordinate bits
//    Y - Y coordinate bits
//    I - Vector intensity(brightness) bits
//    A - Vector address bits
//    R - Red color bit
//    G - Green color bit
//    B - Blue color bit
//    S - Scale (size)
//    U - Unused bit
//    VECTOR ENGINE commands
//    test pattern seems to end in 00 00 CHAOS!!!!
//    screen position        1010YYYY YYYYYYYY UUUUXXXX XXXXXXXX
//    Halt                   1011UUUU UUUUUUUU
//    Jump to subroutine     1100AAAA AAAAAAAA
//    Return from subroutine 1101UUUU UUUUUUUU
//    Jump to new address    1110AAAA AAAAAAAA
//    Short vector draw      1111YYYY IIIIXXXX

#include "6502.H"
#include "allegro.h"

#define CHAOS  0x00
#define LABS   0xa0
#define HALT   0xb0
#define JSRL   0xc0
#define RTSL   0xd0
#define JMPL   0xe0
#define SVEC   0xf0
#define OPMASK 0xf0
#define COMASK 0x0f

extern BITMAP *memory_bitmap;


void vector_draw()
{
int vscale = 1;int dscale =1 ;int vectx = 2048 ; int vecty = 2048;
int sa ,sb ,sc ,sd ,se ,sf ;
int ym = 500; int xm =50; int vpc = 0x4000;
int dcolor = 0;int color = 7;int fcolor = 8;int bcolor = 15;int vstack[0x100];
unsigned char vpointer = 0;

blit(memory_bitmap,screen,0,0,0,0,640,480);
clear_to_color(memory_bitmap, bcolor);

do{      // read next four bytes potential max instruction length

sa =  mem[vpc]    ; sb =  mem[vpc+1]  ;
sc =  mem[vpc+2]  ; sd =  mem[vpc+3]  ;

switch (sb & OPMASK) {

case CHAOS: sb = HALT; break;

case HALT:break;

case LABS:vscale = (sd & 0x00f0) >> 4 ;
          vecty = ((sb & 0x0f) << 8) + sa ;
          vectx = ((sd & 0x0f) << 8) + sc ;
	      vpc +=4 ;
	      break ;

case JSRL:vstack[vpointer] = vpc + 2 ;                          // push address on to stack
	      vpointer++ ;                                          // add one to stack pointer
	      vpc = (((((sb & COMASK) << 8) + sa)+0x2000) << 1);    // ld subroutine address
	      break ;

case RTSL:vpointer-- ;                                          // dec stack pointer
	      vpc = vstack[vpointer];                               // retrieve return address
	      break ;

case JMPL:vpc = (((((sb & COMASK) << 8) + sa) + 0x2000) << 1);
	      break ;
////////////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////////////
case SVEC:color = bcolor;
	      if(sa & OPMASK) color = fcolor;                       //grey like the origanal

	      sc  = (sa & 0x03)<< 1; sd = (sb & 0x03) << 1;

          if (sb & 0x08){                                       // scaling
	      sc <<= 1; sd <<= 1;
              }

	      if (sa & 0x08){                                       // scaling
	      sc <<= 2; sd <<= 2;
	      }

	      if (sa & 0x04)sc = -sc;                               // sign
	      if (sb & 0x04)sd = -sd;                               // sign


			switch (vscale){
							case 8 :  break ;
							case 9 :  sc >>= 7 ; sd >>= 7 ; break ;
							case 10:  sc >>= 6 ; sd >>= 6 ; break ;
							case 11:  sc >>= 5 ; sd >>= 5 ; break ;
							case 12:  sc >>= 4 ; sd >>= 4 ; break ;
							case 13:  sc >>= 3 ; sd >>= 3 ; break ;
							case 14:  sc >>= 2 ; sd >>= 2 ; break ;
							case 15:  sc >>= 1 ; sd >>= 1 ; break ;
            				default:sc <<= vscale ; sd <<= vscale ; break ;
              				}

           if (color == fcolor){
				line( memory_bitmap, (vectx>>1) + xm , ym - (vecty>>1) , ((vectx+sc)>>1) + xm , ym - ((vecty+ sd)>>1),  color);
               					}
				vectx += sc ; vecty += sd ; vpc +=2;
	      break;
////////////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////////////
default:      color = bcolor;                                       //   setcolor
              dscale = sb & 0x0f0;
              if(sd & OPMASK)color = fcolor;                        //grey like the origanal
	    	  se = ((sb & 0x03) << 8)+sa ; sf = ((sd & 0x03) << 8)+sc ;


				switch (dscale){                                                  //long vector draw scale
                	    	case 0x10:  se >>=8 ; sf >>=8 ; break ;
	      					case 0x20:  se >>=7 ; sf >>=7 ; break ;
	      					case 0x30:  se >>=6 ; sf >>=6 ; break ;
	      					case 0x40:  se >>=5 ; sf >>=5 ; break ;
	      					case 0x50:  se >>=4 ; sf >>=4 ; break ;
	      					case 0x60:  se >>=3 ; sf >>=3 ; break ;
	      					case 0x70:  se >>=2 ; sf >>=2 ; break ;
	      					case 0x80:  se >>=1 ; sf >>=1 ; break ;
          					case 0x90:  ; break;
              					}

				switch (vscale){                                                  //LABS global vector scale

	      					case 8:   break   ;
	      					case 9:   se >>=7 ; sf >>=7 ; break ;
	      					case 10:  se >>=6 ; sf >>=6 ; break ;
	      					case 11:  se >>=5 ; sf >>=5 ; break ;
	      					case 12:  se >>=4 ; sf >>=4 ; break ;
	      					case 13:  se >>=3 ; sf >>=3 ; break ;
	      					case 14:  se >>=2 ; sf >>=2 ; break ;
	      					case 15:  se >>=1 ; sf >>=1 ; break ;
              				default:  se <<= vscale ; sf <<= vscale ; break ;
	      						}

     	      if (sb & 0x04)se = -se;                               // sign make minus
	    	  if (sd & 0x04)sf = -sf;                               // sign make minus
              if (color == fcolor){
					line(memory_bitmap,(vectx>>1)+xm,ym-(vecty>>1),((vectx+sf)>>1)+xm,ym-((vecty+se)>>1),color);
                	                			   }




              vectx += sf ; vecty += se ;  vpc +=4;
			  break;
////////////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////////////

              }

	  }while((sb & OPMASK) != HALT);

}


