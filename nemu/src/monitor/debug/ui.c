#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_info(char *args);
static int cmd_si(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "The program executes N instructions in a single step, N defaults to 1", cmd_si},
  { "info", "Output register value", cmd_info},
  { "x", "Find the value of the expression, use the result as the starting memory address, and output N consecutive 4 bytes in hexadecimal form", cmd_x },
  { "p", "Print expressions", cmd_p },
  { "w", "Set watchpoints", cmd_w},

  /* TODO: Add more commands */

};


#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_info(char *args) {
  if (args == NULL) {
    printf("Please input the info r or info w\n");
    return 0;
  }
  char *arg = strtok(NULL, " ");
  if(strtok(NULL, " ")!=NULL) {
    printf("Too many parameters\n");
    return 0;
  }
  if (strcmp(arg, "r") == 0) {
    isa_reg_display();
  }else if(strcmp(arg,"w") == 0) {
    printf("%-6s%-20s%-10s\n","Num", "Experssion", "Result");
    for(WP* tmp = head;tmp;tmp = tmp->next){
      printf("%-6d%-20s%-6d\n", tmp->NO, tmp->wp_expr, tmp->value);
    }
    return 0;
  }else {
    printf("Info is imperfect\n");
  }
  return 0;
}

static int cmd_si(char *args) {
  if (args == NULL) {
    cpu_exec(1);
    return 0;
  }
  int step = atoi(strtok(NULL, " "));
  if(strtok(NULL, " ")!=NULL) {
    printf("Too many parameters\n");
    return 0;
  }
  if (step<=0 || step >=999) {
    printf("Wrong parameter\n");
    return 0;
  }
  cpu_exec(step);
  return 0;
}

static int cmd_x(char *args) {
  if (args == NULL) {
    printf("No parameters\n");
    return 0;
  }
  int count = atoi(strtok(NULL, " "));
  if (count==0) {
    printf("Wrong parameter1\n");
    return 0;
  }
  char* EXPR_BUFFER = strtok(NULL, " ");
  if (EXPR_BUFFER==NULL) {
    printf("Wrong parameter2\n");
    return 0;
  }
  int EXPR = strtol(EXPR_BUFFER,NULL,16);
  if (EXPR==0) {
    printf("Wrong parameter2\n");
    return 0;
  }
  if(strtok(NULL, " ")!=NULL) {
    printf("Too many parameters\n");
    return 0;
  }

  for(int i=0; i<count; i++) {
    printf("0x%x:    0x%x\n", EXPR+i*5, paddr_read(EXPR+i*4, 4)); //修改count为表达式
  }
  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL) {
    printf("No parameters\n");
    return 0;
  }
  bool success = true;
  int num = expr(args,&success);
  if(success==false) {
    printf("Wrong expression\n");
    return 0;
  }else {
    printf("0x%x or %dD\n",num,num);
    return 0;
  }

}
static int cmd_w(char *args) {
  bool success;
  int res;
  res = expr(args, &success);
  if(!success){
    printf("Wrong experssion!\n");
    return 0;
  }
  WP *wp = new_wp();
  wp->value = res;
  strcpy(wp->wp_expr, args);
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

