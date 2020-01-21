 /////////////////////////////////////////////////////////////////////////////
/////////////// the A.Daly  6502 CPU Emulator (c) jan 2000 ////////////////
/////////////////////////////////////////////////////////////////////////////
typedef unsigned int WORD;
typedef unsigned char BYTE;
extern BYTE *mem;
/////////////////////////////////////////////////////////////////////////////
///////////////////////// 6502 cpu structure ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
struct cpu {    BYTE  a, x, y;
                WORD  s, pc;
                } reg ;
struct flag_bits {
                unsigned c: 1;
                unsigned z: 1;
                unsigned i: 1;
                unsigned d: 1;
                unsigned b: 1;
                unsigned r: 1;
                unsigned v: 1;
                unsigned n: 1;
                } ;
union  {
                BYTE p;
                struct flag_bits flag;
                } status ;
/////////////////////////////////////////////////////////////////////////////
//////////////// cpu instruction and address mode and timing tables /////////
/////////////////////////////////////////////////////////////////////////////
enum {implied=0,immediate,absolute,relative,indirect,absx,absy,zp,zpx,zpy,indx,indy,indabsx,indzp};
enum {
adc=0,land, asl, asla, bcc,  bcs, beq,  bit, bmi, bne, bpl, brk, bvc, bvs, clc, cld,  cli, clv, cmp,
cpx,   cpy, dec, dex,  dey,  eor, inc,  inx, iny, jmp, jsr, lda, ldx, ldy, lsr, lsra, nop, ora, pha,
php,   pla, plp, rol,  rola, ror, rora, rti, rts, sbc, sec, sed, sei, sta, stx, sty,  tax, tay, tsx,
txa,   txs, tya, bra,  dea,  ina, phx,  plx, phy, ply, stz, tsb, trb, badop
};
static unsigned char adrmode[]={
implied,  indx,implied,  implied,zp,     zp, zp, implied,implied,immediate,implied,implied,absolute,absolute,absolute,implied,
relative, indy,indzp,    implied,zp,     zpx,zpx,implied,implied,absy,     implied,implied,absolute,absx    ,absx    ,implied,
absolute, indx,implied,  implied,zp,     zp, zp, implied,implied,immediate,implied,implied,absolute,absolute,absolute,implied,
relative, indy,indzp,    implied,zpx,    zpx,zpx,implied,implied,absy,     implied,implied,absx,    absx    ,absx    ,implied,
implied,  indx,implied,  implied,implied,zp, zp, implied,implied,immediate,implied,implied,absolute,absolute,absolute,implied,
relative, indy,indzp,    implied,implied,zpx,zpx,implied,implied,absy,     implied,implied,implied, absx    ,absx    ,implied,
implied,  indx,implied,  implied,zp,     zp, zp, implied,implied,immediate,implied,implied,indirect,absolute,absolute,implied,
relative, indy,indzp,    implied,zpx,    zpx,zpx,implied,implied,absy,     implied,implied,indabsx, absx    ,absx    ,implied,
relative, indx,implied,  implied,zp,     zp, zp, implied,implied,immediate,implied,implied,absolute,absolute,absolute,implied,
relative, indy,indzp,    implied,zpx,    zpx,zpy,implied,implied,absy,     implied,implied,absolute,absx    ,absx    ,implied,
immediate,indx,immediate,implied,zp,     zp, zp, implied,implied,immediate,implied,implied,absolute,absolute,absolute,implied,
relative, indy,indzp,    implied,zpx,    zpx,zpy,implied,implied,absy,     implied,implied,absx,    absx    ,absy    ,implied,
immediate,indx,implied,  implied,zp,     zp, zp, implied,implied,immediate,implied,implied,absolute,absolute,absolute,implied,
relative, indy,indzp,    implied,implied,zpx,zpx,implied,implied,absy,     implied,implied,implied, absx    ,absx    ,implied,
immediate,indx,implied,  implied,zp,     zp, zp, implied,implied,immediate,implied,implied,absolute,absolute,absolute,implied,
relative, indy,indzp,    implied,implied,zpx,zpx,implied,implied,absy,     implied,implied,implied, absx    ,absx    ,implied
};
static unsigned char instruction[]={
brk, ora,  badop, badop, tsb,   ora,  asl, badop, php, ora,  asla, badop, tsb,   ora,  asl, badop,
bpl, ora,  ora,   badop, trb,   ora,  asl, badop, clc, ora,  ina,  badop, trb,   ora,  asl, badop,
jsr, land, badop, badop, bit,   land, rol, badop, plp, land, rola, badop, bit,   land, rol, badop,
bmi, land, land,  badop, bit,   land, rol, badop, sec, land, dea,  badop, bit,   land, rol, badop,
rti, eor,  badop, badop, badop, eor,  lsr, badop, pha, eor,  lsra, badop, jmp,   eor,  lsr, badop,
bvc, eor,  eor,   badop, badop, eor,  lsr, badop, cli, eor,  phy,  badop, badop, eor,  lsr, badop,
rts, adc,  badop, badop, stz,   adc,  ror, badop, pla, adc,  rora, badop, jmp,   adc,  ror, badop,
bvs, adc,  adc,   badop, stz,   adc,  ror, badop, sei, adc,  ply,  badop, jmp,   adc,  ror, badop,
bra, sta,  badop, badop, sty,   sta,  stx, badop, dey, bit,  txa,  badop, sty,   sta,  stx, badop,
bcc, sta,  sta,   badop, sty,   sta,  stx, badop, tya, sta,  txs,  badop, stz,   sta,  stz, badop,
ldy, lda,  ldx,   badop, ldy,   lda,  ldx, badop, tay, lda,  tax,  badop, ldy,   lda,  ldx, badop,
bcs, lda,  lda,   badop, ldy,   lda,  ldx, badop, clv, lda,  tsx,  badop, ldy,   lda,  ldx, badop,
cpy, cmp,  badop, badop, cpy,   cmp,  dec, badop, iny, cmp,  dex,  badop, cpy,   cmp,  dec, badop,
bne, cmp,  cmp,   badop, badop, cmp,  dec, badop, cld, cmp,  phx,  badop, badop, cmp,  dec, badop,
cpx, sbc,  badop, badop, cpx,   sbc,  inc, badop, inx, sbc,  nop,  badop, cpx,   sbc,  inc, badop,
beq, sbc,  sbc,   badop, badop, sbc,  inc, badop, sed, sbc,  plx,  badop, badop, sbc,  inc, badop
};
static int ticks[]={
7, 6, 2, 2, 3, 3, 5, 2, 3, 3, 2, 2, 4, 4, 6, 2, 2, 5, 3, 2, 3, 4, 6, 2, 2, 4, 2, 2, 4, 4, 7, 2,
6, 6, 2, 2, 3, 3, 5, 2, 4, 3, 2, 2, 4, 4, 6, 2, 2, 5, 3, 2, 4, 4, 6, 2, 2, 4, 2, 2, 4, 4, 7, 2,
6, 6, 2, 2, 2, 3, 5, 2, 3, 3, 2, 2, 3, 4, 6, 2, 2, 5, 3, 2, 2, 4, 6, 2, 2, 4, 3, 2, 2, 4, 7, 2,
6, 6, 2, 2, 3, 3, 5, 2, 4, 3, 2, 2, 5, 4, 6, 2, 2, 5, 3, 2, 4, 4, 6, 2, 2, 4, 4, 2, 6, 4, 7, 2,
2, 6, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 2, 2, 6, 3, 2, 4, 4, 4, 2, 2, 5, 2, 2, 4, 5, 5, 2,
3, 6, 3, 2, 3, 3, 3, 2, 2, 3, 2, 2, 4, 4, 4, 2, 2, 5, 3, 2, 4, 4, 4, 2, 2, 4, 2, 2, 4, 4, 4, 2,
3, 6, 2, 2, 3, 3, 5, 2, 2, 3, 2, 2, 4, 4, 6, 2, 2, 5, 3, 2, 2, 4, 6, 2, 2, 4, 3, 2, 2, 4, 7, 2,
3, 6, 2, 2, 3, 3, 5, 2, 2, 3, 2, 2, 4, 4, 6, 2, 2, 5, 3, 2, 2, 4, 6, 2, 2, 4, 4, 2, 2, 4, 7, 2
};
////////////////////////////////////////////////////////////////////////////
/////////////////////prototypes/////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void vector_draw(void);

void reset6502(void);
void nmi6502(void);
void exec6502(signed int timerTicks);
void wr_mem(WORD addr, BYTE byte);
int  rd_mem(WORD addr);
void bad_operand(void);
//////////////////////////some useful defines ///////////////////////////////
#define N  0x80
#define C  0x01
#define SET 0x01
#define CLR 0x00
//////////////////////////////// common flag checks//////////////////////////
#define CHECK_NZ_FLAGS(reg)\
{        if (reg)  status.flag.z = CLR; else status.flag.z = SET;\
         if (reg&N)status.flag.n = SET; else status.flag.n = CLR;\
             }
#define CHECK_Z_FLAG(reg)\
{        if (reg) status.flag.z = CLR; else status.flag.z = SET;\
             }
#define CHECK_NV_BIT(reg)\
{        if(reg & 0x80) status.flag.n = SET;  else status.flag.n = CLR;\
         if(reg & 0x40) status.flag.v = SET;  else status.flag.v = CLR;\
             }
//////////////////////////////// common instructions ////////////////////////
////////// arithmetic shift left thru carry /////////////////////////////////
#define ASL(reg)\
{          address_mode(operand);\
	       status.flag.c = ((reg >> 7) & C);\
           reg <<= 1;\
           CHECK_NZ_FLAGS(reg); \
       }
////////// branch if flag set ///////////////////////////////////////////////
#define BRANCH_TRUE(testflag)\
{          if(testflag){      address_mode(operand);\
	                          reg.pc += pc_temp;\
                              cpu_clocks++;\
                           }  else  reg.pc++;\
}
/////////// branch if flag clear ////////////////////////////////////////////
#define BRANCH_FALSE(testflag)\
{           if(!(testflag)){  address_mode(operand);\
                              reg.pc += pc_temp;\
                              cpu_clocks++;\
                           }  else  reg.pc++;\
 }
/////////// compare to register//////////////////////////////////////////////
#define COMPARE(reg)\
{          address_mode(operand);\
           value=mem[pc_temp];\
           if(reg-value+0x100>0xff) status.flag.c = 1; else status.flag.c = 0;\
           value=reg-value;\
           CHECK_NZ_FLAGS(value);\
           }
//////////// decrement //////////////////////////////////////////////////////
#define DEC_REG(reg)\
{		   address_mode(operand);\
           reg--;\
           CHECK_NZ_FLAGS(reg);\
}
//////////// increment //////////////////////////////////////////////////////
#define INC_REG(reg)\
{		   address_mode(operand);\
           reg++;\
           CHECK_NZ_FLAGS(reg);\
}
//////////// load register //////////////////////////////////////////////////
#define LOAD_REG(reg)\
{          address_mode(operand);\
           reg=mem[pc_temp];\
           CHECK_NZ_FLAGS(reg);\
           }
/////////// logical shift right /////////////////////////////////////////////
#define LSR(reg)\
{          address_mode(operand);\
	       status.flag.c = (reg & C);\
           reg>>=1;\
           CHECK_NZ_FLAGS(reg);\
           }
////////// rotate left thru carry ///////////////////////////////////////////
#define ROL(reg)\
 {          address_mode(operand);\
	        saveflags=status.flag.c;\
            status.flag.c =((reg >> 7) & C);\
            reg <<= 1;\
            reg |= saveflags;\
            CHECK_NZ_FLAGS(reg);\
           }
/////////// rotate right thru carry /////////////////////////////////////////
#define ROR(reg)\
{           address_mode(operand);\
	        saveflags= status.flag.c;\
            status.flag.c = (reg & C);\
            reg >>= 1;\
            if(saveflags)reg |= N;\
            CHECK_NZ_FLAGS(reg);\
            }

////////// push to stack /////////////////////////////////////////////////////
#define PUSH(rg)\
{		   mem[reg.s--] = rg;\
}
/////// pull from stack /////////////////////////////////////////////////
#define PULL(rg)\
{			rg = mem[++reg.s];\
	        CHECK_NZ_FLAGS(rg);\
}
/////////////// store zero page//////////////////////////////////////////////

/////////////// store zero page//////////////////////////////////////////////
#define STOREZP(rg)\
{			address_mode(operand);\
			wr_mem(pc_temp,rg);\
		}

			//mem[pc_temp] = rg;\
		//}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////address mode displacement calculations ////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#define IMMEDIATE  pc_temp = reg.pc++

#define ABSOLUTE   pc_temp=mem[reg.pc]+(mem[reg.pc+1]<<8);\
                   reg.pc += 2

#define RELATIVE   pc_temp=mem[reg.pc++];\
                   if(pc_temp & N)pc_temp-=0x100;\
                   if((pc_temp >> 8)!=(reg.pc >> 8))cpu_clocks++

#define INDIRECT   help=mem[reg.pc]+(mem[reg.pc+1]<<8);\
                   pc_temp=mem[help]+(mem[help+1] << 8);\
                   reg.pc += 2

#define ABSX       pc_temp=mem[reg.pc]+(mem[reg.pc+1]<<8);  \
                   reg.pc += 2; \
                   if(ticks[opcode]==4) \
                   if((pc_temp>>8)!=((pc_temp+reg.x)>>8)) cpu_clocks++; \
                   pc_temp += reg.x

#define ABSY       pc_temp=mem[reg.pc]+(mem[reg.pc+1]<<8);  \
                   reg.pc += 2;\
                   if(ticks[opcode]==4)\
                   if((pc_temp >> 8)!=((pc_temp+reg.y) >> 8)) cpu_clocks++; \
                   pc_temp += reg.y

#define ZPAGE(zp)  pc_temp = mem[reg.pc++]+zp

#define INDX(ad)   value = mem[reg.pc++]+ad; \
                   pc_temp = mem[value]|(mem[value+1]<<8)

#define INDY       value = mem[reg.pc++]; \
                   pc_temp = mem[value] | (mem[value+1] << 8); \
                   if(ticks[opcode]==5)\
                   if((pc_temp >> 8)!=((pc_temp+reg.y) >> 8)) cpu_clocks++; \
                   pc_temp += reg.y

#define INDABSX    help=mem[reg.pc]+(mem[reg.pc+1]<<8)+reg.x; \
                   pc_temp=mem[help]|(mem[help+1]<<8)

