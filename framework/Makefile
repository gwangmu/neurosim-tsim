LIB=neurosim-tsim-fw.a

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
AR=ar

# compiler flags
CXXFLAGS=--std=c++14 -ferror-limit=3
LDFLAGS=
ARFLAGS=-rc

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
RMFILES:=$(OBJDIR)/* $(LIB)

# auxiliary functions
print=printf "info: %-15s $(if $(2),[,) $(2)$(if $(3), --> $(3),) $(if $(2),],)\n" $(1)
to_comma_list=$(subst $(SPACE),$(COMMA) ,$(1))


### rules

all: $(OBJSUBDIRS) $(LIB)

$(OBJSUBDIRS):
	@ $(call print,"creating..",$(call to_comma_list,$@))
	@ $(MKDIR) $@

$(LIB): $(OBJFILES)
	@ $(call print,"linking..",$(call to_comma_list,$^),$@)
	@ $(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(CPPEXT)
	@ $(call print,"compiling..",$<,$@)
	@ $(CXX) -I$(HDRDIR) $(CXXFLAGS) -c $^ -o $@


clean:
	@ $(call print,"removing..",$(call to_comma_list,$(RMFILES)))
	@ $(RM) $(RMFILES)
	@ $(call print,"done.")
