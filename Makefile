BIN=neurosim-tsim

# auxiliaries
NULL:=
COMMA:=,
SPACE:=$(NULL) #

# extensions
CPPEXT=cpp
HDREXT=h
OBJEXT=o

# toolchain
CXX=clang++
LD=clang++

# compiler flags
CXXFLAGS=
LDFLAGS=

# bash commands
MKDIR=mkdir -p
RM=rm -rf

# root directories
SRCDIR=src
OBJDIR=obj
HDRDIR=include

# source and object files
SRCSUBDIRS:=$(shell find $(SRCDIR) -type d)
OBJSUBDIRS:=$(subst $(SRCDIR),$(OBJDIR),$(SRCSUBDIRS))
CODEFILES:=$(addsuffix /*,$(SRCSUBDIRS))
CODEFILES:=$(wildcard $(CODEFILES))

SRCFILES:=$(filter %.$(CPPEXT),$(CODEFILES))
OBJFILES:=$(subst $(SRCDIR),$(OBJDIR),$(SRCFILES:%.$(CPPEXT)=%.$(OBJEXT)))
RMFILES:=$(OBJDIR)/* $(BIN)

# auxiliary functions
print=printf "info: %-15s $(if $(2),[,) $(2)$(if $(3), --> $(3),) $(if $(2),],)\n" $(1)
to_comma_list=$(subst $(SPACE),$(COMMA) ,$(1))


### rules

all: $(OBJSUBDIRS) $(BIN)

$(OBJSUBDIRS):
	@ $(call print,"creating..",$(call to_comma_list,$@))
	@ $(MKDIR) $@

$(BIN): $(OBJFILES)
	@ $(call print,"linking..",$(call to_comma_list,$^),$@)
	@ $(LD) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(CPPEXT)
	@ $(call print,"compiling..",$<,$@)
	@ $(CXX) -I$(HDRDIR) $(CXXFLAGS) -c $^ -o $@


clean:
	@ $(call print,"removing..",$(call to_comma_list,$(RMFILES)))
	@ $(RM) $(RMFILES)
	@ $(call print,"done.")
