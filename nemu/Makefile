# 开始是检测isa指令集
NAME = nemu

ifneq ($(MAKECMDGOALS),clean) # ignore check for make clean
# ifneq(arg1,arg2)，判断arg1和arg2的值是否相同，如果不同则为真。
# 只要shell输入的不是make clean,都会进入下面的内容
ISA ?= x86
# ?= 是如果没有被赋值过就赋予等号后面的值
ISAS = $(shell ls src/isa/)
$(info Building $(ISA)-$(NAME))
# 输出Building x86-nemu
ifeq ($(filter $(ISAS), $(ISA)), ) # ISA must be valid
# 查看isa是不是正确，不正确就报错
$(error Invalid ISA. Supported: $(ISAS))
# 输出警告信息，并且让程序退出
endif
endif



INC_DIR += ./include ./src/isa/$(ISA)/include

BUILD_DIR ?= ./build

ifdef SHARE
# 如果参数值为空，则为真
SO = -so
SO_CFLAGS = -fPIC -D_SHARE=1
SO_LDLAGS = -shared -fPIC
endif

OBJ_DIR ?= $(BUILD_DIR)/obj-$(ISA)$(SO)
BINARY ?= $(BUILD_DIR)/$(ISA)-$(NAME)$(SO)

include Makefile.git

.DEFAULT_GOAL = app

# Compilation flags
CC = gcc
LD = gcc
INCLUDES  = $(addprefix -I, $(INC_DIR))
CFLAGS   += -O2 -MMD -Wall -Werror -ggdb3 $(INCLUDES) -D__ISA__=$(ISA) -fomit-frame-pointer

QEMU_DIFF_PATH = $(NEMU_HOME)/tools/qemu-diff
QEMU_SO = $(QEMU_DIFF_PATH)/build/$(ISA)-qemu-so

$(QEMU_SO):
	$(MAKE) -C $(QEMU_DIFF_PATH)

# Files to be compiled
SRCS = $(shell find src/ -name "*.c" | grep -v "isa")
SRCS += $(shell find src/isa/$(ISA) -name "*.c")
OBJS = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)

# Compilation patterns
$(OBJ_DIR)/%.o: src/%.c
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(SO_CFLAGS) -c -o $@ $<


# Depencies
-include $(OBJS:.o=.d)

# Some convenient rules

.PHONY: app run gdb clean run-env $(QEMU_SO)
app: $(BINARY)

override ARGS ?= -l $(BUILD_DIR)/nemu-log.txt
override ARGS += -d $(QEMU_SO)

# Command to execute NEMU
IMG :=
NEMU_EXEC := $(BINARY) $(ARGS) $(IMG)

$(BINARY): $(OBJS)
	$(call git_commit, "compile")
	@echo + LD $@
	@$(LD) -O2 -rdynamic $(SO_LDLAGS) -o $@ $^ -lSDL2 -lreadline -ldl

run-env: $(BINARY) $(QEMU_SO)

run: run-env
	$(call git_commit, "run")
	$(NEMU_EXEC)

gdb: run-env
	$(call git_commit, "gdb")
	gdb -s $(BINARY) --args $(NEMU_EXEC)

clean:
	-rm -rf $(BUILD_DIR)
	$(MAKE) -C tools/gen-expr clean
	$(MAKE) -C tools/qemu-diff clean
