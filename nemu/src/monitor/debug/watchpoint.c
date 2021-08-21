#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
WP* new_wp();
void free_wp(WP *wp);

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    // 简简单单小链表
    // 0 -> 1 -> 2 -> 3 -> ... -> 31 -> null
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
  if(!free_) {
    panic("Error: Wp_pool is empty!\n");
  }
  WP *tmp = free_;
  // tmp=NO.0
  free_ = free_->next;
  // free=NO.1
  tmp->next = head;
  // 第一个链表指向的地址为空，接下来依次指向上一个
  // null <- 0 <- 1 <- 2 <- 3
  head = tmp;
  // head=NO.0
  return tmp;
}

void free_wp(WP *wp) {
  if(!wp)
    return;
  // 检测是不是空链表
  int flag = 0;
  WP *tmp = head;
  // tmp is the latest element in the wp list
  // e.g. null < 0 < 1 < 2 tmp=2
  for(;tmp;tmp = tmp->next, flag++){
    if(tmp == wp)
      break;
  }
  if(!tmp)
    return;
  // haven't found wp, no need to delete
  if(flag == 0)
    head = head->next;
  //wp is the first element in wp list
  else{
    tmp = head;
    for(int i = 0; i < flag-1; i++){
      tmp = tmp->next;
    }
    tmp->next = tmp->next->next;
  }
  wp->next = free_;
  free_ = wp;
}



