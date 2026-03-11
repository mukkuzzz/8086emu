#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;


//struct definition for registers
typedef union {
	u16 x;
	struct 
	{
  		#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        		u8 l;
        		u8 h;
		#else
        		u8 h;
        		u8 l;
		#endif	
	};
}reg16;


//CPU struct
typedef struct{
	reg16 ax,bx,cx,dx;
	reg16 sp,bp,si,di;

	u16 ip;

	u16 cs,ds,es,ss;

	u16 flags;
} CPU8086;

#define MEM_SIZE (1024*1024)
u8 memory[1024*1024];





u32 phy(u16 segment, u16 offset) 
{

	return ((u32)segment << 4) + offset;
}



//Instruction pointer fetch
u8
fetch8(CPU8086 *cpu)
{
	u32 addr = phy(cpu->cs,cpu->ip);
	if (addr >= MEM_SIZE){
		printf("Segmentation fault at %04X:%04X\n",cpu->cs,cpu->ip);
		exit(1);
	}
	cpu->ip++;
	return memory[addr];
}

u16
fetch16(CPU8086 *cpu)
{
	u8 lo = fetch8(cpu);
	u8 hi = fetch8(cpu);
	return (hi << 8) | lo;
}


static reg16* reg16_table(CPU8086 *cpu, int index) 
{ 
	switch(index)
	{ 
		case 0: return &cpu->ax; 
		case 1: return &cpu->cx;
		case 2: return &cpu->dx;
		case 3: return &cpu->bx;
		case 4: return &cpu->sp;
		case 5: return &cpu->bp;
		case 6: return &cpu->si;
		case 7: return &cpu->di;
		default: fprintf(stderr, "Invalid register index %d\n", index); exit(1); 
	}
}

static 
u8* reg8_table(CPU8086 *cpu, int index)
{
	switch(index)
	{	
		case 0: return &cpu->ax.l;
		case 1: return &cpu->cx.l;
		case 2: return &cpu->dx.l;
		case 3: return &cpu->bx.l;
		case 4: return &cpu->ax.h;
		case 5: return &cpu->cx.h;
		case 6: return &cpu->dx.h;
		case 7: return &cpu->bx.h;
		default: fprintf(stderr, "Invalid register h/l index %d\n", index);exit(1);
	}
}


static
const char *reg16_names[8] = {
    "AX","CX","DX","BX","SP","BP","SI","DI"
};
static
const char *reg8_names[8] = {
	"AL","CL","DL","BL","AH","CH","DH","BH"
};

typedef struct {
    u8 modrm; 
    u8 mod;
    u8 reg;
    u8 rm;
    void *src;   
    void *dst;
    const char *src_reg;
    const char *dst_reg;
   } ModRM;

ModRM modResolve(CPU8086* cpu,bool ops){
	ModRM op = {0};
	op.modrm = fetch8(cpu);
	op.mod = (op.modrm>>6) & 0x03;
	op.reg = (op.modrm>>3) & 0x07;
	op.rm = op.modrm & 0x07;
	return op;
}

//load assembly language into memory 
void 
load_binary(const char *filename,u16 segment, u16 offset)
{
	FILE *file = fopen(filename,"rb");
	if (!file)
	{
		perror("fopen");
		exit(1);
	}

	u32 addr = phy(segment,offset);

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
dump_memory(CPU8086 *cpu)
{
	printf("AX=%04X BX=%04X CX=%04X DX=%04X\n",
	cpu->ax.x,cpu->bx.x,cpu->cx.x,cpu->dx.x);
	printf("SP=%04X BP=%04X SI=%04X DI=%04X\n",
	cpu->sp.x,cpu->bp.x,cpu->si.x,cpu->di.x);
	printf("DS=%04X ES=%04X SS=%04X\n",
	cpu->ds,cpu->es,cpu->ss);
	printf("CS:IP=%04X:%04X\n",cpu->cs,cpu->ip);
}
void 
reset_cpu(CPU8086 *cpu)
{
	memset(cpu,0,sizeof(*cpu));
}

int
execute_instruction(CPU8086 *cpu)
{
	u8 inst = fetch8(cpu);
	printf("CS:IP %04X:%04X  Opcode: %02X\n",
           cpu->cs, cpu->ip - 1, inst);
	switch(inst)
	{
		case 0xB8 ... 0xBF:
		{
			u16 imm = fetch16(cpu);
			int index = inst - 0xB8;
			reg16_table(cpu,index)->x = imm;
			printf("MOV %s , %04X\n",reg16_names[index],imm);
			break;
		}			
		
		case 0xB0 ... 0xB7:
		{
    			u8 imm = fetch8(cpu);
			int index = inst - 0xB0;
			u8 *ptr = reg8_table(cpu,index);
			*ptr = imm;
			break;
		}

		case 0x88:
		{
			ModRM  res = modResolve(cpu,0);
			if(res.mod == 3){
				*(reg8_table(cpu,res.rm)) = *(reg8_table(cpu,res.reg));
				printf("MOV %s , %s\n",reg8_names[res.rm],reg8_names[res.reg]);
			}else{
				u8 offset = fetch16(cpu);
			
				u32 addr = phy(cpu->ds,offset);
				memory[addr] = *(reg8_table(cpu,res.reg));
				printf("MOV [%04X] %s\n",offset,reg8_names[res.reg]);
			}

		}

		case 0x89:
		{
			ModRM res = modResolve(cpu,0);
			if(res.mod == 3){
				*(reg16_table(cpu,res.rm)) = *(reg16_table(cpu,res.reg));
				printf("MOV %s , %s \n", reg16_names[res.rm],reg16_names[res.reg]);
			}
		}
		
			
		case 0xF4:
			printf("HLT\n");
			dump_memory(cpu);
			return 0;
		default:
			printf("Unknown Instruction!!\n");
			break;
	}
	return 1;
}

int
main()
{	CPU8086 cpu = {0};
	load_binary("test.bin",0x1000,0x0000);
	cpu.cs=0x1000;
	cpu.ip=0x0000;
	while(execute_instruction(&cpu));
}
