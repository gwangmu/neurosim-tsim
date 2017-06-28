BIN=neurosim-tsim

### Auxiliaries ###
# characters
NULL:=
COMMA:=,
SPACE:=$(NULL) #

# bash commands
MKDIR=mkdir -p
RM=rm -rf

# extensions
CPPEXT=cpp
HDREXT=h
OBJEXT=o
###################

### Toolchain ###
# compiler commands
CXX=clang++
LD=clang++
AR=ar

# compiler flags
CXXFLAGS= -g --std=c++14 -ferror-limit=3 $(if $(NDEBUG),-DNDEBUG)
LDFLAGS=
ARFLAGS=-rc
#################

### Paths ###
# root directories
SRCDIR=src
OBJDIR=obj
HDRDIR=include
LIBDIR=lib

ifdef EXAMPLE
SRCDIR=example/src
HDRDIR=example/include
endif

# library paths
TSIM=TSim
TSIM_DIR=$(LIBDIR)/tsim
TSIM_HDRDIR=$(TSIM_DIR)/include/TSim
TSIM_LIB=$(TSIM_DIR)/libtsim.a

# TODO: add variables for new library
#[lib]=[lib_name]
#[lib]_DIR=$(LIBDIR)/[lib_dirname]
#[lib]_HDRDIR=$([lib]_DIR)/[lib_hdrdir_name]
#[lib]_LIB=$([lib]_DIR)/[lib_bindir_name]

# libraies in use
USING_LIBS=TSIM  		# TODO: add [lib] in this list.
#############

### Helper functions ###
print=printf "info: %-15s $(if $(2),[,) $(2)$(if $(3), --> $(3),) $(if $(2),],)\n" $(1)
to_comma_list=$(subst $(SPACE),$(COMMA) ,$(1))
########################


### Makefile lists ###
# source and object files
SRCSUBDIRS:=$(shell find $(SRCDIR) -type d)
OBJSUBDIRS:=$(subst $(SRCDIR),$(OBJDIR),$(SRCSUBDIRS))
CODEFILES:=$(addsuffix /*,$(SRCSUBDIRS))
CODEFILES:=$(wildcard $(CODEFILES))

SRCFILES:=$(filter %.$(CPPEXT),$(CODEFILES))
OBJFILES:=$(subst $(SRCDIR),$(OBJDIR),$(SRCFILES:%.$(CPPEXT)=%.$(OBJEXT)))
RMFILES:=$(OBJDIR) $(BIN)

LIBS:=$(foreach LIB,$(USING_LIBS),$($(LIB)_LIB))
######################


### rules

.PHONY: $(USING_LIBS)

all: $(USING_LIBS) $(OBJSUBDIRS) $(FRAMEWORK) $(BIN)

TSIM:
	@ $(if $(wildcard $(TSIM_HDRDIR)),,$(error non-existing $(TSIM_HDRDIR)))
	@ $(shell $(RM) $(HDRDIR)/$(TSIM))
	@ $(call print,"symlinking..",$(TSIM_HDRDIR),$(HDRDIR)/$(TSIM))
	@ $(shell ln -s $(abspath $(TSIM_HDRDIR)) $(HDRDIR)/$(TSIM))

$(OBJSUBDIRS):
	@ $(call print,"creating..",$(call to_comma_list,$@))
	@ $(MKDIR) $@

$(BIN): $(OBJFILES) $(LIBS)
	@ $(call print,"linking..",$(call to_comma_list,$^),$@)
	@ $(LD) $^ -o $@

$(OBJDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(CPPEXT)
	@ $(call print,"compiling..",$<,$@)
	@ $(CXX) -I$(HDRDIR) $(CXXFLAGS) -c $^ -o $@


clean:
	@ $(call print,"removing..",$(call to_comma_list,$(RMFILES)))
	@ $(RM) $(RMFILES)
	@ $(call print,"done.")
