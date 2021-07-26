#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[60000];
int count;
static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";


static int choose(unsigned int i) {
  return rand()%i;
}
static void gen_num() {
  int i=choose(65536);
  sprintf(buf+count, "%d", i);
  while(buf[count]) {
    count++;
  }
}
static void gen_rand_op() {
  switch(choose(4)) {
    case 0:
      sprintf(buf+count, "%c", '+');
      break;
    case 1: 
      sprintf(buf+count, "%c", '-');
      break;
    case 2:
      sprintf(buf+count, "%c", '*');
      break;
    case 3:
      sprintf(buf+count, "%c", '/');
      break;
  }
  count++;
}

static void gen(char c) {
  sprintf(buf+count, "%c", c);
  count++;
}


static inline void gen_rand_expr() {
  int i = choose(3);
  if(count>20) { i = 0; }
  switch (i) {
    case 0:
      gen_num();
      break;
    case 1:
      gen('(');
      gen_rand_expr();
      gen(')');
      buf[count] = '\0';
      break;
    default:
      gen_rand_expr();
      gen_rand_op();
      gen_rand_expr();
      break;
  }
}


int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    count=0;
    gen_rand_expr();
    sprintf(code_buf, code_format, buf);
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
