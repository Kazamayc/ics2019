#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  int value;
  char wp_expr[100];
  /* TODO: Add more members if necessary */


} WP;

#endif
