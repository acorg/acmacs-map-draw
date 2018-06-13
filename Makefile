# -*- Makefile -*-
# Eugene Skepner 2017
# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

TARGETS = \
    $(ACMACS_MAP_DRAW_LIB) \
    $(DIST)/chart-select \
    $(DIST)/map-draw \
    $(DIST)/map-procrustes \
    $(DIST)/geographic-draw \
    $(DIST)/chart-vaccines \
    $(DIST)/mod_acmacs.so

# $(ACMACS_MAP_DRAW_PY_LIB)

ACMACS_MAP_DRAW_SOURCES = \
  draw.cc point-style-draw.cc map-elements.cc labels.cc geographic-map.cc time-series.cc \
  vaccines.cc vaccine-matcher.cc settings.cc select.cc setup-dbs.cc geographic-settings.cc \
  mod-applicator.cc mod-serum.cc mod-procrustes.cc mod-amino-acids.cc

ACMACS_MAP_DRAW_PY_SOURCES = py.cc $(ACMACS_MAP_DRAW_SOURCES)

ACMACS_MAP_DRAW_LIB_MAJOR = 2
ACMACS_MAP_DRAW_LIB_MINOR = 0
ACMACS_MAP_DRAW_LIB_NAME = libacmacsmapdraw
ACMACS_MAP_DRAW_LIB = $(DIST)/$(call shared_lib_name,$(ACMACS_MAP_DRAW_LIB_NAME),$(ACMACS_MAP_DRAW_LIB_MAJOR),$(ACMACS_MAP_DRAW_LIB_MINOR))

# ACMACS_MAP_DRAW_PY_LIB_MAJOR = 2
# ACMACS_MAP_DRAW_PY_LIB_MINOR = 0
# ACMACS_MAP_DRAW_PY_LIB_NAME = acmacs_map_draw_backend
# ACMACS_MAP_DRAW_PY_LIB = $(DIST)/$(ACMACS_MAP_DRAW_PY_LIB_NAME)$(PYTHON_MODULE_SUFFIX)

# ----------------------------------------------------------------------

include $(ACMACSD_ROOT)/share/makefiles/Makefile.g++
# include $(ACMACSD_ROOT)/share/makefiles/Makefile.python
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.vars

CXXFLAGS = -g -MMD $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WARNINGS) -Icc -I$(AD_INCLUDE) $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LDLIBS = \
  $(AD_LIB)/$(call shared_lib_name,libacmacsbase,1,0) \
  $(AD_LIB)/$(call shared_lib_name,liblocationdb,1,0) \
  $(AD_LIB)/$(call shared_lib_name,libacmacschart,2,0) \
  $(AD_LIB)/$(call shared_lib_name,libhidb,5,0) \
  $(AD_LIB)/$(call shared_lib_name,libseqdb,2,0) \
  $(AD_LIB)/$(call shared_lib_name,libacmacsdraw,1,0) \
  -L$(AD_LIB) -lboost_date_time $$(pkg-config --libs cairo) $$(pkg-config --libs liblzma) $(CXX_LIB)
# $$($(PYTHON_CONFIG) --ldflags | sed -E 's/-Wl,-stack_size,[0-9]+//')

PKG_INCLUDES = $(shell pkg-config --cflags cairo) $(shell pkg-config --cflags liblzma)
# $(PYTHON_INCLUDES)

# ----------------------------------------------------------------------

all: check-acmacsd-root $(TARGETS)

install: check-acmacsd-root install-headers install-acmacs-map-draw-lib $(TARGETS)
	ln -sf $(abspath bin)/* $(AD_BIN)
	ln -sf $(abspath $(DIST))/map-* $(AD_BIN)
	ln -sf $(abspath $(DIST))/chart-* $(AD_BIN)
	ln -sf $(abspath $(DIST))/geographic-* $(AD_BIN)
	ln -sf $(abspath $(DIST))/mod_acmacs.so $(AD_LIB)
	mkdir -p $(AD_SHARE)/js/map-draw; ln -sf $(shell pwd)/js/* $(AD_SHARE)/js/map-draw

install-acmacs-map-draw-lib: $(ACMACS_MAP_DRAW_LIB)
	$(call install_lib,$(ACMACS_MAP_DRAW_LIB))

test: install
	test/test -v

# ----------------------------------------------------------------------

-include $(BUILD)/*.d
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.rules
include $(ACMACSD_ROOT)/share/makefiles/Makefile.rtags

# ----------------------------------------------------------------------

$(ACMACS_MAP_DRAW_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_MAP_DRAW_SOURCES)) | $(DIST)
	@printf "%-16s %s\n" "SHARED" $@
	@$(call make_shared,$(ACMACS_MAP_DRAW_LIB_NAME),$(ACMACS_MAP_DRAW_LIB_MAJOR),$(ACMACS_MAP_DRAW_LIB_MINOR)) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# $(ACMACS_MAP_DRAW_PY_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_MAP_DRAW_PY_SOURCES)) | $(DIST)
#	@printf "%-16s %s\n" "SHARED" $@
#	@$(call make_shared,$(ACMACS_MAP_DRAW_PY_LIB_NAME),$(ACMACS_MAP_DRAW_PY_LIB_MAJOR),$(ACMACS_MAP_DRAW_PY_LIB_MINOR)) $(LDFLAGS) -o $@ $^ $(LDLIBS) $(PYTHON_LDLIBS)

$(DIST)/%: $(BUILD)/%.o | $(ACMACS_MAP_DRAW_LIB)
	@printf "%-16s %s\n" "LINK" $@
	@$(CXX) $(LDFLAGS) -o $@ $^ $(ACMACS_MAP_DRAW_LIB) $(LDLIBS) $(AD_RPATH)

# ----------------------------------------------------------------------

APXS_CXX = -S CC=$(CXX) -Wc,-xc++ -Wl,-shared
APXS_ENV = LTFLAGS="-v"
APXS_LIBS_NAMES = acmacsbase.1 acmacschart.2 acmacsmapdraw.2 locationdb.1 seqdb.2
ifeq (Darwin,$(shell uname))
  APXS_LIBS = -L$(AD_LIB) $(APXS_LIBS_NAMES:%=-l%)
else
  APXS_LIBS_NAMES_FIXED = $(basename $(APXS_LIBS_NAMES))
  APXS_LIBS = -L$(AD_LIB) $(APXS_LIBS_NAMES_FIXED:%=-l%)
endif

$(DIST)/mod_acmacs.so: $(BUILD)/.libs/apache-mod-acmacs.so
	ln -sf $^ $@

$(BUILD)/.libs/apache-mod-acmacs.so: cc/apache-mod-acmacs.cc | install-acmacs-map-draw-lib
	@echo apxs does not not understand any file suffixes besides .c, so we have to use .c for C++
	ln -sf $(abspath $^) $(BUILD)/$(basename $(notdir $^)).c
	env $(APXS_ENV) apxs $(APXS_CXX) $(CXXFLAGS:%=-Wc,%) -n acmacs_module $(APXS_LIBS) -c $(BUILD)/$(basename $(notdir $^)).c

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
