#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>






typedef union {
	uint16_t x;
	struct {
		uint8_t l;
		uint8_t h;
	};
}reg16;





typedef struct{
	reg16 ax,bx,cx,dx;
	reg16 sp,bp,si,di;

	uint16_t ip;

	uint16_t cs,ds,es,ss;

	uint16_t flags;
} CPU8086;

uint8_t memory[1024*1024];


/**
 * This function only calculates the corresponding physical address
 * for the 8086. 32 bit is used because the address will always be
 * 20 bit and C does not have a native 20 bit type.
 **/
uint32_t 
phy(uint16_t segment ,uint16_t offset) 
{
	return ((uint32_t)segment << 4 ) + offset;
}

uint8_t
fetch8(CPU8086 *cpu)
{
	uint32_t addr = phy(cpu->cs,cpu->ip);
	cpu->ip++;
	return memory[addr];
}


uint16_t
fetch16(CPU8086 *cpu){
	uint8_t lo =fetch8(cpu);
	uint8_t hi =fetch8(cpu);
	return (hi << 8) | lo;
}


void 
execute_instruction(CPU8086 *cpu)
{
	printf("PLACEHOLDER");	
}



