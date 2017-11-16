# -*- Makefile -*-
# Eugene Skepner 2017
# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

TARGETS = \
    $(ACMACS_MAP_DRAW_LIB) \
    $(BACKEND) \
    $(DIST)/chart-select \
    $(DIST)/chart-info \
    $(DIST)/map-draw \
    $(DIST)/geographic-draw

LIB_SOURCES = draw.cc point-style-draw.cc map-elements.cc labels.cc geographic-map.cc time-series.cc \
	      vaccines.cc vaccine-matcher.cc settings.cc select.cc mod-applicator.cc setup-dbs.cc geographic-settings.cc
PY_SOURCES = py.cc $(LIB_SOURCES)
BACKEND = $(DIST)/acmacs_map_draw_backend$(PYTHON_MODULE_SUFFIX)

# ----------------------------------------------------------------------

include $(ACMACSD_ROOT)/share/makefiles/Makefile.g++
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.vars

PYTHON_VERSION = $(shell python3 -c 'import sys; print("{0.major}.{0.minor}".format(sys.version_info))')
PYTHON_CONFIG = python$(PYTHON_VERSION)-config
PYTHON_MODULE_SUFFIX = $(shell $(PYTHON_CONFIG) --extension-suffix)

ACMACS_MAP_DRAW_LIB = $(DIST)/libacmacsmapdraw.so

CXXFLAGS = -g -MMD $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WARNINGS) -Icc -I$(AD_INCLUDE) $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LDLIBS = $(AD_LIB)/$(call shared_lib_name,libacmacsbase,1,0) \
	 $(AD_LIB)/$(call shared_lib_name,liblocationdb,1,0) \
	 $(AD_LIB)/$(call shared_lib_name,libacmacschart,1,0) \
	 $(AD_LIB)/$(call shared_lib_name,libhidb,1,0) \
	 $(AD_LIB)/$(call shared_lib_name,libseqdb,1,0) \
	 -L$(AD_LIB) -lacmacsdraw -lboost_date_time $$(pkg-config --libs cairo) $$(pkg-config --libs liblzma) $$($(PYTHON_CONFIG) --ldflags | sed -E 's/-Wl,-stack_size,[0-9]+//') $(CXX_LIB)

PKG_INCLUDES = $(shell pkg-config --cflags cairo) $(shell pkg-config --cflags liblzma) $(shell $(PYTHON_CONFIG) --includes)

# ----------------------------------------------------------------------


all: check-acmacsd-root $(TARGETS)

install: check-acmacsd-root install-headers $(TARGETS)
	$(call install_lib,$(ACMACS_MAP_DRAW_LIB))
	ln -sf $(BACKEND) $(AD_PY)
	ln -sf $(abspath py)/* $(AD_PY)
	ln -sf $(abspath bin)/acmacs-map-* $(AD_BIN)
	ln -sf $(abspath bin)/geographic-* $(AD_BIN)
	ln -sf $(abspath $(DIST))/{map,chart,geographic}-* $(AD_BIN)

test: install
	test/test -v

# ----------------------------------------------------------------------

-include $(BUILD)/*.d
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.rules
include $(ACMACSD_ROOT)/share/makefiles/Makefile.rtags

# ----------------------------------------------------------------------

$(ACMACS_MAP_DRAW_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(LIB_SOURCES)) | $(DIST)
	@printf "%-16s %s\n" "SHARED" $@
	@$(CXX) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BACKEND): $(patsubst %.cc,$(BUILD)/%.o,$(PY_SOURCES)) | $(DIST)
	@printf "%-16s %s\n" "SHARED" $@
	@$(CXX) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(DIST)/%: $(BUILD)/%.o | $(ACMACS_MAP_DRAW_LIB)
	@printf "%-16s %s\n" "LINK" $@
	@$(CXX) $(LDFLAGS) -o $@ $^ $(ACMACS_MAP_DRAW_LIB) $(LDLIBS)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
