#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


//struct definition for registers
typedef union {
	uint16_t x;
	struct {
		uint8_t l;
		uint8_t h;
	};
}reg16;



//CPU struct
typedef struct{
	reg16 ax,bx,cx,dx;
	reg16 sp,bp,si,di;

	uint16_t ip;

	uint16_t cs,ds,es,ss;

	uint16_t flags;
} CPU8086;

#define MEM_SIZE (1024*1024)
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

//Instruction pointer fetch
uint8_t
fetch8(CPU8086 *cpu)
{
	uint32_t addr = phy(cpu->cs,cpu->ip);
	if (addr >= MEM_SIZE){
		printf("Segmentation fault at %04X:%04X\n",cpu->cs,cpu->ip);
		exit(1);
	}
	cpu->ip++;
	return memory[addr];
}


uint16_t
fetch16(CPU8086 *cpu){
	uint8_t lo =fetch8(cpu);
	uint8_t hi =fetch8(cpu);
	return (hi << 8) | lo;
}

//load assembly language into memory 
void 
load_binary(const char *filename,uint16_t segment, uint16_t offset)
{
	FILE *file = fopen(filename,"rb");
	if (!file)
	{
		perror("fopen");
		exit(1);
	}

	uint32_t addr = phy(segment,offset);

	size_t n;
	
	while ((n = fread(&memory[addr],1,4096,file)) > 0){
		addr+=n;
		if(addr >= MEM_SIZE) { 
			fprintf(stderr, "Program too large\n"); 
			exit(1);
		}
	}
	fclose(file);
}


void 
execute_instruction(CPU8086 *cpu)
{
	uint8_t inst = fetch8(cpu);

	printf("%d",inst);	
	
}



int
main()
{
	CPU8086 cpu = {0};

	execute_instruction(&cpu);

}

