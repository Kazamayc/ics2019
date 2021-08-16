#include "nemu.h"
#include <stdlib.h>
#include <time.h>

const char *regsl[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

void reg_test() {
  srand(time(0));
  uint32_t sample[8];
  uint32_t pc_sample = rand();
  cpu.pc = pc_sample;

  int i;
  for (i = R_EAX; i <= R_EDI; i ++) {
    sample[i] = rand();
    reg_l(i) = sample[i];
    assert(reg_w(i) == (sample[i] & 0xffff));
  }

  assert(reg_b(R_AL) == (sample[R_EAX] & 0xff));
  assert(reg_b(R_AH) == ((sample[R_EAX] >> 8) & 0xff));
  assert(reg_b(R_BL) == (sample[R_EBX] & 0xff));
  assert(reg_b(R_BH) == ((sample[R_EBX] >> 8) & 0xff));
  assert(reg_b(R_CL) == (sample[R_ECX] & 0xff));
  assert(reg_b(R_CH) == ((sample[R_ECX] >> 8) & 0xff));
  assert(reg_b(R_DL) == (sample[R_EDX] & 0xff));
  assert(reg_b(R_DH) == ((sample[R_EDX] >> 8) & 0xff));

  assert(sample[R_EAX] == cpu.eax);
  assert(sample[R_ECX] == cpu.ecx);
  assert(sample[R_EDX] == cpu.edx);
  assert(sample[R_EBX] == cpu.ebx);
  assert(sample[R_ESP] == cpu.esp);
  assert(sample[R_EBP] == cpu.ebp);
  assert(sample[R_ESI] == cpu.esi);
  assert(sample[R_EDI] == cpu.edi);

  assert(pc_sample == cpu.pc);
}

void isa_reg_display() {
  printf("EAX:0x%x\n",cpu.eax);
  printf("ECX:0x%x\n",cpu.ecx);
  printf("EDX:0x%x\n",cpu.edx);
  printf("EBX:0x%x\n",cpu.ebx);
  printf("ESP:0x%x\n",cpu.esp);
  printf("EBP:0x%x\n",cpu.ebp);
  printf("ESI:0x%x\n",cpu.esi);
  printf("EDI:0x%x\n",cpu.edi);
  return;
}

uint32_t isa_reg_str2val(const char *s) {
  if(strcmp("$eax", s)==0) return cpu.eax;
  if(strcmp("$ecx", s)==0) return cpu.ecx;
  if(strcmp("$edx", s)==0) return cpu.edx;
  if(strcmp("$ebx", s)==0) return cpu.ebx;
  if(strcmp("$esp", s)==0) return cpu.esp;
  if(strcmp("$ebp", s)==0) return cpu.ebp;
  if(strcmp("$esi", s)==0) return cpu.esi;
  if(strcmp("$edi", s)==0) return cpu.edi;
  panic("\nError register");
  return 0;
}
