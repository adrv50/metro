TARGET		?= 	metrocc
DBGPREFIX	?=	d

TOPDIR		?= 	$(CURDIR)
BUILD		:= 	build
INCLUDE		:= 	include
SOURCE		:= 	src

CC			:=	gcc

OPTI		?=	-O0 -g -D_METRO_DEBUG_
COMMON		:=	$(OPTI) -Wall -Wextra -Wno-switch $(INCLUDES)
CFLAGS		:=	$(COMMON) -std=c17
LDFLAGS		:=
LIB			:=	-lmetro

%.o: %.c
	@echo $(notdir $<)
	@$(CC) -MP -MMD -MF $*.d $(CFLAGS) -c -o $@ $<

ifneq ($(BUILD), $(notdir $(CURDIR)))

CFILES			= $(notdir $(foreach dir,$(SOURCE),$(wildcard $(dir)/*.c)))

export OUTPUT		= $(TOPDIR)/$(TARGET)$(DBGPREFIX)
export VPATH		= $(foreach dir,$(SOURCE),$(TOPDIR)/$(dir))
export INCLUDES		= $(foreach dir,$(INCLUDE),-I$(TOPDIR)/$(dir))
export OFILES		= $(CFILES:.c=.o)

.PHONY: $(BUILD) all re clean

all: release

debug: $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(TOPDIR)/Makefile

release: $(BUILD)
	@$(MAKE) --no-print-directory OUTPUT="$(TOPDIR)/$(TARGET)" OPTI="-O8" \
		LDFLAGS="-Wl,--gc-sections,-s" -C $(BUILD) -f $(TOPDIR)/Makefile

$(BUILD):
	@[ -d $@ ] || mkdir -p $@

clean:
	rm -rf $(TARGET) $(TARGET)$(DBGPREFIX) $(BUILD)

re: clean all

else

DEPENDS		:=	$(OFILES:.o=.d)

$(OUTPUT): $(OFILES)
	@echo linking...
	@$(CC) -pthread $(LDFLAGS) -o $@ $^

-include $(DEPENDS)

endif