#include "nemu.h"
#include "monitor/monitor.h"
#include <unistd.h>

void init_log(const char *log_file);
void init_isa();
void init_regex();
void init_wp_pool();
void init_device();
void init_difftest(char *ref_so_file, long img_size);

static char *mainargs = "";
static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static int is_batch_mode = false;

static inline void welcome() {
#ifdef DEBUG
  Log("Debug: \33[1;32m%s\33[0m", "ON");
  Log("If debug mode is on, A log file will be generated to record every instruction NEMU executes. "
      "This may lead to a large log file. "
      "If it is not necessary, you can turn it off in include/common.h.");
#else
  Log("Debug: \33[1;32m%s\33[0m", "OFF");
#endif

  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to \33[1;41m\33[1;33m%s\33[0m-NEMU!\n", str(__ISA__));
  printf("For help, type \"help\"\n");
}

static inline long load_img() {
  long size;
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    extern uint8_t isa_default_img[];
    extern long isa_default_img_size;
    size = isa_default_img_size;
    memcpy(guest_to_host(IMAGE_START), isa_default_img, size); // 读取镜像到内存
  }
  else {
    int ret;

    FILE *fp = fopen(img_file, "rb");
    Assert(fp, "Can not open '%s'", img_file); //失败的话报错
    
    Log("The image is %s", img_file);

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);

    fseek(fp, 0, SEEK_SET);
    ret = fread(guest_to_host(IMAGE_START), size, 1, fp); // 读取镜像到内存
    assert(ret == 1);

    fclose(fp);

    // mainargs
    strcpy(guest_to_host(0), mainargs);
  }
  return size;
}

static inline void parse_args(int argc, char *argv[]) {
  int o;
  while ( (o = getopt(argc, argv, "-bl:d:a:")) != -1) {
    switch (o) {
      case 'b': is_batch_mode = true; break;
      case 'a': mainargs = optarg; break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 1:
                if (img_file != NULL) Log("too much argument '%s', ignored", optarg);
                else img_file = optarg;
                break;
      default:
                panic("Usage: %s [-b] [-l log_file] [img_file]", argv[0]);
    }
  }
}
int init_monitor(int argc, char *argv[]) {
  /* 进行一些全局初始化 */

  /* 解析参数 */
  parse_args(argc, argv);

  /* 打开日志文件 */
  init_log(log_file);

  /* 将image加载到内存 */
  long img_size = load_img();

  /* 执行依赖于ISA的初始化 */
  init_isa();

  /* 编译正则表达式 */
  init_regex();

  /* 初始化watchpoint pool */
  init_wp_pool();

  /* 初始化设备 */
  init_device();

  /* 初始化差分测试 */
  init_difftest(diff_so_file, img_size);

  /* Display welcome message. */
  welcome();

  return is_batch_mode;
}