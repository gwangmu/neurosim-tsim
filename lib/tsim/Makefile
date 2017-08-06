LIB=libtsim.a

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
MAKE=make --no-print-directory

# compiler flags
CXXFLAGS=--std=c++11 -O3 -ferror-limit=3 $(if $(NDEBUG),-DNDEBUG) $(if $(NINFO), -DNINFO)
LDFLAGS=-O3 -lm
ARFLAGS=-rcT
#################

### Paths ###
# root directories
SRCDIR=src
OBJDIR=obj
HDRDIR=include
LIBDIR=lib

# library paths
BOOKSIM=BookSim2
BOOKSIM_DIR=$(LIBDIR)/booksim2
BOOKSIM_HDRDIR=$(BOOKSIM_DIR)
BOOKSIM_LIB=$(BOOKSIM_DIR)/libbooksim.a
BOOKSIM_MAKEFILE_DIR=$(BOOKSIM_DIR)

# libraies in use
USING_LIBS=BOOKSIM
USING_LIB_PATHS=$(foreach LIBVAR,$(USING_LIBS),$($(LIBVAR)_LIB))
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
RMFILES:=$(OBJDIR)/* $(LIB)
######################

$(info $(call to_lib_files,$(USING_LIBS)))
### rules
all: $(USING_LIBS) $(OBJSUBDIRS) $(LIB)


## libraries
$(USING_LIBS):
	@ $(call print,"making..",$($@))
	@ $(if $($@_MAKEFILE_DIR), \
		$(MAKE) -C $($@_MAKEFILE_DIR) $(if $(IS_CLEANING),clean,) \
			$(shell ln -sf $(abspath $($@_HDRDIR) $(HDRDIR)/$($@))), \
		$(if $(IS_CLEANING),$@_clean,$@))

# TODO: create '<LIB_VAR_NAME>_clean' rule here
#  if library has no makefile


## TSim
$(OBJSUBDIRS):
	@ $(call print,"creating..",$(call to_comma_list,$@))
	@ $(MKDIR) $@

$(LIB): $(OBJFILES) $(USING_LIB_PATHS)
	@ $(call print,"linking..",$(call to_comma_list,$^),$@)
	@ $(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(CPPEXT)
	@ $(call print,"compiling..",$<,$@)
	@ $(CXX) -I$(HDRDIR) $(CXXFLAGS) -c $^ -o $@


clean:
	@ $(call print,"removing..",$(call to_comma_list,$(RMFILES)))
	@ $(RM) $(RMFILES)
	@ $(call print,"done.")

superclean: 
	@ $(MAKE) $(USING_LIBS) IS_CLEANING=1
	@ $(MAKE) clean
