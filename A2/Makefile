PROJECT ?= lab2
EXT = cc
TARGET ?= $(PROJECT)
PROFILE ?= debug
OUT_DIR ?= build
SRC_DIR ?= src
CC=g++
LD=$(CC)

#INCLUDES += -I /path/to/include
#LIBS += -L /path/to/libs

#VS_DIR=/home/networker/Desktop/VS
#CAF_DIR=$(VS_DIR)/actor-framework

INCLUDES += -I $(SRC_DIR)
#INCLUDES += -I $(VS_DIR)/vslab-caf/third_party/
#INCLUDES += -I include/
#INCLUDES += -I $(CAF_DIR)/build/libcaf_core/
#INCLUDES += -I $(CAF_DIR)/libcaf_core/
#INCLUDES += -I $(CAF_DIR)/build/libcaf_io/
#INCLUDES += -I $(CAF_DIR)/libcaf_io/

#LIBS += -L $(CAF_DIR)/build/lib/
LIBS += -lcaf_core -lcaf_io

CFLAGS_release += -O3
CFLAGS_debug += -ggdb -O0
CFLAGS_all += -Wall

CFLAGS = $(CFLAGS_all) $(CFLAGS_$(PROFILE))

LDFLAGS = $(LDFLAGS_all) $(LDFLAGS_$(PROFILE))

DEPS = -Wp,-MMD,$(@:%.o=%.d),-MT,$@

rwildcard = $(wildcard $(addprefix $1/*.,$2)) $(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2))

SRC = $(call rwildcard, $(SRC_DIR), $(EXT))
OBJ = $(addprefix $(OUT_DIR)/,$(addsuffix .o,$(basename $(SRC:$(SRC_DIR)/%=%))))

.PHONY: all clean rebuild

$(OUT_DIR)/%.o: $(SRC_DIR)/%.$(EXT)
	@mkdir -p $(dir $@)
	$(CC) -c $< $(DEPS) $(CFLAGS) $(INCLUDES) -o $@

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) -o $(TARGET) $(OBJ) $(LDFLAGS) $(LIBS)

clean:
	rm -rf $(OUT_DIR) $(TARGET)

rebuild: clean all

-include $(OBJ:%.o=%.d)
