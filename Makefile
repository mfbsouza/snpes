# Makefile by Matheus Souza (github.com/mfbsouza)

# project name
PROJECT := snpes
export

# paths
BUILDDIR := ./build
DBGDIR   := $(BUILDDIR)/debug
RELDIR   := $(BUILDDIR)/release
INCDIR   := ./src

# compiler and binutils
PREFIX :=
CC     := $(PREFIX)gcc
CXX    := $(PREFIX)g++
OD     := $(PREFIX)objdump

# flags
CFLAGS   := -Wall -I $(INCDIR) -MMD -MP
CXXLAGS  := -Wall -I $(INCDIR) -MMD -MP
LDFLAGS  :=

ifeq ($(RELEASE),1)
	BINDIR    := $(RELDIR)
	OBJDIR    := $(RELDIR)/obj
	CFLAGS    += -Os -DNDEBUG
	CXXFLAGS  += -Os -DNDEBUG
else
	BINDIR    := $(DBGDIR)
	OBJDIR    := $(DBGDIR)/obj
	CFLAGS    += -ggdb3 -O0 -DDEBUG
	CXXFLAGS  += -ggdb3 -O0 -DDEBUG
endif

# sources to compile
ALLCSRCS   += $(shell find ./src -type f -name *.c)
ALLCXXSRCS += $(shell find ./src -type f -name *.cpp)

# set the linker to g++ if there is any c++ source code
ifeq ($(ALLCXXSRCS),)
	LD := $(PREFIX)gcc
else
	LD := $(PREFIX)g++
endif

# objects settings
COBJS   := $(addprefix $(OBJDIR)/, $(notdir $(ALLCSRCS:.c=.o)))
CXXOBJS := $(addprefix $(OBJDIR)/, $(notdir $(ALLCXXSRCS:.cpp=.o)))
OBJS    := $(COBJS) $(CXXOBJS)
DEPS    := $(OBJS:.o=.d)

# paths where to search for sources
SRCPATHS := $(sort $(dir $(ALLCSRCS)) $(dir $(ALLCXXSRCS)))
VPATH     = $(SRCPATHS)

# output
OUTFILES := \
	$(BINDIR)/$(PROJECT).elf \
	$(BUILDDIR)/$(PROJECT).lst \

# targets
.PHONY: all clean tests

all: $(OBJDIR) $(BINDIR) $(OBJS) $(OUTFILES)

# targets for the dirs
$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(BINDIR):
	@mkdir -p $(BINDIR)

# target for c objects
$(COBJS) : $(OBJDIR)/%.o : %.c
ifeq ($(VERBOSE),1)
	$(CC) -c $(CFLAGS) $< -o $@
else
	@echo -n "[CC]\t$<\n"
	@$(CC) -c $(CFLAGS) $< -o $@
endif

# target for cpp objects
$(CXXOBJS) : $(OBJDIR)/%.o : %.cpp
ifeq ($(VERBOSE),1)
	$(CXX) -c $(CXXFLAGS) $< -o $@
else
	@echo -n "[CXX]\t$<\n"
	@$(CXX) -c $(CXXFLAGS) $< -o $@
endif

# target for ELF file
$(BINDIR)/$(PROJECT).elf: $(OBJS)
ifeq ($(VERBOSE),1)
	$(LD) $(LDFLAGS) $(OBJS) -o $@
else
	@echo -n "[LD]\t./$@\n"
	@$(LD) $(LDFLAGS) $(OBJS) -o $@
endif

# target for disassembly and sections header info
$(BUILDDIR)/$(PROJECT).lst: $(BINDIR)/$(PROJECT).elf
ifeq ($(VERBOSE),1)
	$(OD) -h -S $< > $@
else
	@echo -n "[OD]\t./$@\n"
	@$(OD) -h -S $< > $@
endif

# target for code style formatting
format:
	find . \
		-regex '.*\.\(cpp\|hpp\|cc\|cxx\|c\|h\)' \
		-exec clang-format -style=file -i {} \;

# target for building the docker image for testing
docker_image:
	docker build . -t snpes-tests

# target for running the tests in the a docker image
docker_tests: docker_image
	docker run -it snpes-tests

# target for unit tests
tests:
	make -f MakeTests.mk all

# target for code coverage
coverage: tests
	@lcov --capture --directory $(OBJDIR)/src \
		--output-file $(OBJDIR)/coverage.info
	@genhtml $(OBJDIR)/coverage.info --output-directory $(BUILDDIR)/coverage
	@echo
	@echo -n "Written coverage report to $(BUILDDIR)/coverage/index.html"
	@echo

# target for cleaning files
clean:
	rm -f $(PROJECT)_tests
	rm -rf $(BUILDDIR)

# Include the dependency files, should be the last of the makefile
-include $(DEPS)
