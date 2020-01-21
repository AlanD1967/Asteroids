/////////////////////////////////////////////////////////////////////////////
/////////////// the A.Daly  6502 CPU Emulator (c) jan 2000 ////////////////
/////////////////////////////////////////////////////////////////////////////
#include "6502.h"
#include "allegro.h"
/////////////////////////////////////////////////////////////////////////////
//////////////// registers flags working variables //////////////////////////
/////////////////////////////////////////////////////////////////////////////
BYTE opcode,value;
WORD help,pc_temp;
int saveflags,cpu_ticks,cpu_clocks=0;

/////////////////////////////////////////////////////////////////////////////
///////////////////// set current address mode //////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void address_mode(unsigned char operand)
{
switch(adrmode[operand]){
case implied:                  break;       // Adressing modes Implied
case immediate: IMMEDIATE;     break;       // #Immediate
case absolute:  ABSOLUTE;      break;       // Absolute
case relative:  RELATIVE;      break;       // relative
case indirect:  INDIRECT;      break;       // indirect
case absx:      ABSX;          break;
case absy:      ABSY;          break;
case zp:        ZPAGE(0);      break;       // ZP
case zpx:       ZPAGE(reg.x);  break;       // ZP,X
case zpy:       ZPAGE(reg.y);  break;       // ZP,Y
case indx:      INDX(reg.x);   break;
case indy:      INDY;          break;
case indabsx:   INDABSX;       break;
case indzp:     INDX(0);       break;
default:                       break;
     }
}

/////////////////////////////////////////////////////////////////////////////
////////////////// execute current instruction //////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void do6502op(unsigned char operand)
{
switch(instruction[operand])
  {
case adc:  address_mode(operand);
           value = rd_mem(pc_temp);
           pc_temp = value + reg.a + status.flag.c;
           CHECK_NZ_FLAGS((char)pc_temp);
           if (!((reg.a^value)&0x80) && ((reg.a^pc_temp)&0x80))status.flag.v=SET; else status.flag.v=CLR;

           if(status.flag.d) {    // decimmal
                              if ((reg.a&0x0f)+(value&0x0f)+(status.flag.c)>9) pc_temp+=6;
                              if (pc_temp&N) status.flag.n=SET; else status.flag.n=CLR;
                              if (pc_temp>0x99){ pc_temp+=0x60; status.flag.c=SET;}
                                                else status.flag.c=CLR;
                             }
                         else
                             {    // binary
                              if (pc_temp>0xff) status.flag.c=SET;  else status.flag.c=CLR;
                             }
           reg.a = (char)pc_temp;
           break;

case land: address_mode(operand);
           value = rd_mem(pc_temp);
           reg.a &= value;
           CHECK_NZ_FLAGS(reg.a);                  break;

case asl:  ASL(mem[pc_temp]);                      break;
case asla: ASL(reg.a);                             break;

case bcs:  BRANCH_TRUE(status.flag.c);             break;
case beq:  BRANCH_TRUE(status.flag.z);             break;
case bmi:  BRANCH_TRUE(status.flag.n);             break;
case bvs:  BRANCH_TRUE(status.flag.v);             break;
case bcc:  BRANCH_FALSE(status.flag.c);            break;
case bne:  BRANCH_FALSE(status.flag.z);            break;
case bpl:  BRANCH_FALSE(status.flag.n);            break;
case bvc:  BRANCH_FALSE(status.flag.v);            break;

case bit:  address_mode(operand);
           value=rd_mem(pc_temp);
           CHECK_Z_FLAG(value & reg.a);
           if(operand != immediate) CHECK_NV_BIT(value);break; //0x89

case brk:  wr_mem(reg.s--,(BYTE)(++reg.pc>>8));
           wr_mem(reg.s--,(BYTE)(reg.pc));
           status.flag.b = SET;
           status.flag.i = SET;
           wr_mem(reg.s--,status.p);
           reg.pc = mem[0x7ffe] | (mem[0x7fff] << 8);   break;


case clc:  status.flag.c = CLR;                    break;
case cld:  status.flag.d = CLR;                    break;
case cli:  status.flag.i = CLR;                    break;
case clv:  status.flag.v = CLR;                    break;

case cmp:  COMPARE(reg.a);                         break;
case cpx:  COMPARE(reg.x);                         break;
case cpy:  COMPARE(reg.y);                         break;

case dec:  DEC_REG(mem[pc_temp]);                  break;
case dea:  DEC_REG(reg.a);						   break;
case dex:  DEC_REG(reg.x);   				       break;
case dey:  DEC_REG(reg.y);   					   break;

case eor:  address_mode(operand);
           reg.a ^= rd_mem(pc_temp);
           CHECK_NZ_FLAGS(reg.a);                  break;

case inc:  INC_REG(mem[pc_temp]);                  break;
case ina:  INC_REG(reg.a);						   break;
case inx:  INC_REG(reg.x);						   break;
case iny:  INC_REG(reg.y);						   break;

case jmp:  address_mode(operand); reg.pc = pc_temp;break;

case jsr:  wr_mem(reg.s--,(BYTE)(++reg.pc >> 8));
           wr_mem(reg.s--,(BYTE)(reg.pc-- ));
           address_mode(operand);
           reg.pc=pc_temp;
           break;

case lda:  LOAD_REG(reg.a); break;
case ldx:  LOAD_REG(reg.x); break;
case ldy:  LOAD_REG(reg.y); break;

case lsr:  LSR(mem[pc_temp]);            break;
case lsra: LSR(reg.a);                   break;
case nop:                                break;
case ora:  address_mode(operand);
           reg.a |= mem[pc_temp];
           CHECK_NZ_FLAGS(reg.a);        break;

case pha:  PUSH(reg.a);			         break;
case phx:  PUSH(reg.x);			         break;
case phy:  PUSH(reg.y);			         break;
case php:  PUSH(status.p);			     break;

case pla:  PULL(reg.a);					 break;
case plx:  PULL(reg.x);					 break;
case ply:  PULL(reg.y);					 break;

case plp:  status.p=mem[++reg.s];  status.flag.r = SET;  break;

case rol:  ROL(mem[pc_temp]);            break;
case rola: ROL(reg.a);                   break;

case ror:  ROR(mem[pc_temp]);            break;
case rora: ROR(reg.a);                   break;

case rti:  status.p = mem[++reg.s]; status.flag.r = SET;
           reg.pc = (mem[++reg.s]) | (mem[++reg.s] << 8);
           break;

case rts:  reg.pc = (mem[++reg.s]) | (mem[++reg.s] << 8);
           reg.pc++;
           break;

case sbc:  address_mode(operand);
           value = rd_mem(pc_temp) ;
           pc_temp = reg.a-value-(~(status.flag.c)&C);
           CHECK_NZ_FLAGS((BYTE)pc_temp);
           if (((reg.a^pc_temp)&0x80) && ((reg.a^value)&0x80))status.flag.v=SET; else status.flag.v=CLR;

           if(status.flag.d){  // decimal
                             if (((reg.a&0x0f)-(~(status.flag.c)&C))<(value&0x0f))pc_temp-=6;
                             if (pc_temp>0x99){pc_temp-=0x60; status.flag.c=CLR;}
                                               else status.flag.c=SET;
                            }
                         else
                            {  //binary
                             if (pc_temp>0xff) status.flag.c=CLR; else status.flag.c=SET;
                            }
           reg.a = (BYTE)pc_temp;
           break;

case sec:  status.flag.c = SET;                             break;
case sed:  status.flag.d = SET;                             break;
case sei:  status.flag.i = SET;                             break;

case sta:  STOREZP(reg.a);   								break;
case stx:  STOREZP(reg.x);   								break;
case sty:  STOREZP(reg.y);   								break;
case stz:  STOREZP(0x00);       							break;

case tax:  reg.x = reg.a;          CHECK_NZ_FLAGS(reg.x);   break;
case tay:  reg.y = reg.a;          CHECK_NZ_FLAGS(reg.y);   break;
case tsx:  reg.x = (char)reg.s;    CHECK_NZ_FLAGS(reg.x);   break;
case txa:  reg.a = reg.x;          CHECK_NZ_FLAGS(reg.a);   break;
case txs:  reg.s = reg.x;          reg.s |= 0x100;          break;
case tya:  reg.a = reg.y;          CHECK_NZ_FLAGS(reg.a);   break;

case bra:  address_mode(operand);  reg.pc += pc_temp; cpu_clocks++;   break;



case tsb:  address_mode(operand);
           CHECK_NV_BIT(mem[pc_temp]);
           mem[pc_temp] |= reg.a;
           CHECK_Z_FLAG(mem[pc_temp]);                      break;

case trb:  address_mode(operand);
           CHECK_NV_BIT(mem[pc_temp]);
           mem[pc_temp] &= (reg.a^0xff);
           CHECK_Z_FLAG(mem[pc_temp]);                      break;

case badop:bad_operand();                                   break;
// case 1000 ... 2000: break;
default:   bad_operand();                                   break;
                }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////catch unexpected machine operation dump some////////
/////////////////////////machine state information to a file ////////////////
/////////////////////////////////////////////////////////////////////////////
void bad_operand()
{    FILE *p; unsigned int a ; unsigned char ch ;
     if((p = fopen("badop.bin","wb"))!=NULL){
     for (a=0x0000;a<0x8000;a++){ ch=mem[a];  fputc(ch,p);  }
     fprintf (p ," pc=%4x a=%2x x=%2x y=%2x",reg.pc,reg.a,reg.x,reg.y);
     fclose(p);    exit(1);}
               }
////////////////////////////////////////////////////////////////////////////
/////////////////////// reset cpu //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void reset6502()
{
      reg.a = reg.x = reg.y = status.p = CLR;
      status.flag.r=SET;
      status.flag.b=SET;
      reg.s=0x1ff;
      reg.pc = rd_mem(0x7ffc) | (rd_mem(0x7ffd) << 8);  // asteroids puts roms in wrong place
              }

////////////////////////////////////////////////////////////////////////////
///////////////////// Non maskerable interrupt /////////////////////////////
////////////////////////////////////////////////////////////////////////////
void nmi6502()
{
      wr_mem(reg.s--,(BYTE)(reg.pc >> 8));
      wr_mem(reg.s--,(BYTE)(reg.pc));
      status.flag.b = CLR;
      wr_mem(reg.s--,status.p);
      status.flag.b = SET;
      status.flag.i = SET;
      reg.pc = rd_mem(0x7ffa) | (rd_mem(0x7ffb) << 8);
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////// Maskerable Interrupt //////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void irq6502()
{
     wr_mem(reg.s--,(BYTE)(reg.pc >> 8));
     wr_mem(reg.s--,(BYTE)(reg.pc));
     status.flag.b = CLR;
     wr_mem(reg.s--,status.p);
     status.flag.i = SET;
     status.flag.b = SET;
     reg.pc = rd_mem(0x7ffe) | (rd_mem(0x7fff) << 8);
    }

///////////////////////////////////////////////////////////////////////////
///////////////////////// execute n clock pulses ////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void exec6502(int cpu_ticks){

   do{
        opcode = rd_mem(reg.pc++);
        do6502op(opcode);
        cpu_clocks += ticks[opcode];
        cpu_ticks -= cpu_clocks;
        cpu_clocks = 0;          } while (cpu_ticks > 0);
    }

//////////////////////////////////////////////////////////////////////////////////////
//   read memory function
//////////////////////////////////////////////////////////////////////////////////////

int rd_mem(WORD addr) {
	return mem[addr];
	}

////////////////////////////////////////////////////////////////////////////////////////
//    write memory funtion
////////////////////////////////////////////////////////////////////////////////////////
void wr_mem(WORD addr, BYTE byte){
	mem[addr] = byte;
    if (addr == 0x3000)  vector_draw();   // catch to initate vector engine
	}

