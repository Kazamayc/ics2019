#include "nemu.h"
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_DEC, TK_NEG,
  /* TODO: Add more token types */

};
typedef struct token {
  int type;
  char str[32];
} Token;
int check_parentheses(Token* start, Token* end);
Token* calc_op(Token* start, Token* end);
void check_Negative(Token* start, Token* end);
int eval(Token* start, Token* end);

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {"[0-9]+", TK_DEC},     // dec
  {" +", TK_NOTYPE},      // spaces
  {"\\*", '*'},           // mul
  {"/", '/'},             // div
  {"\\(", '('},           // bra1
  {"\\)", ')'},           // bra2
  {"-", '-'},             // sub
  {"\\+", '+'},           // plusd
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    // 把我们自己订的规则rules存入re数组
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}



static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        // 把字符串逐个识别成token，存在pmatch
        char *substr_start = e + position;
        // 把token对应的起始字符串地址存入substr_start
        int substr_len = pmatch.rm_eo;
        // 把token长度存入substr_len
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
          case '+':
          case '-':
          case ')':
          case '(':
          case '/':
          case '*':
          case TK_DEC:
          	tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token++].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            break;
          case TK_NOTYPE:
            break;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  check_Negative(tokens,tokens+nr_token-1);
  // 找到负数将符号类型进行替换
  int num = eval(tokens,tokens+nr_token-1);
  return num;
}

int eval(Token* start, Token* end) {
  if (start == end) {
    return atoi(start->str);
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
  }
  else if(check_parentheses(start, end) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
    * If that is the case, just throw away the parentheses.
    */
    return eval(start + 1, end - 1);
  }
  else{
    int val1=0,val2=0;
    Token *op = calc_op(start, end);
    if(op->type != TK_NEG) {
      val1 = eval(start, op - 1);  
    } 
    val2 = eval(op + 1, end);
    switch (op->type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      case TK_NEG : return val2*-1;
      default: panic("Error expression");
    }
  }
}

int check_parentheses(Token* start, Token* end) {
  int sign = 0;
  int count = 0;
  if (start->type!='(' || end->type!=')' ) {
    return false;
  }
  for(Token* sym = start; sym<end; sym++) {
    if(sym->type == '(') {
      count++;
    }
    else if(sym->type ==')') {
      count--;
    }
    if(count==0) {
      sign=1;
    }
  }
  if(count==1&&sign==0) {
    return true;
  }
  if(count==1&&sign==1) {
    return false;
  }
  panic("Error expression");
}
Token* calc_op(Token* start, Token* end) {
  int sign = 0;
  int count = 0;
  Token* op = NULL;
  for (Token* sym = start; sym<=end; sym++) {
    if (sym->type=='(') {
      count++;
      continue;
    }
    if (sym->type==')') {
      count--;
      continue;
    }
    if(count!=0) {
      continue;
    }
    if(sym->type==TK_DEC) {
      continue;
    }
    if(sign<=2&&(sym->type=='+'||sym->type=='-')) {
      op=sym;
      sign = 2;
    }
    else if(sign<=1&&(sym->type=='*'||sym->type=='/')) {
      op=sym;
      sign=1;
    }else if(sign==0&&(sym->type==TK_NEG)) {
      op=sym;
    }
  }
  return op;
}

void check_Negative(Token* start, Token* end) {
  for(; start<=end; start++) {
    if(start->type=='-' && (start-1)->type!=TK_DEC && (start-1)->type!=')') {
      start->type=TK_NEG;
    }
  }
  return;
}
