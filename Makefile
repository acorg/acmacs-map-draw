# -*- Makefile -*-
# Eugene Skepner 2016

# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

SOURCES = draw.cc point-style-draw.cc map-elements.cc labels.cc geographic-map.cc time-series.cc vaccines.cc
PY_SOURCES = py.cc $(SOURCES)
BACKEND = $(DIST)/acmacs_map_draw_backend$(PYTHON_MODULE_SUFFIX)

# ----------------------------------------------------------------------

include $(ACMACSD_ROOT)/share/Makefile.g++
include $(ACMACSD_ROOT)/share/Makefile.dist-build.vars

PYTHON_VERSION = $(shell python3 -c 'import sys; print("{0.major}.{0.minor}".format(sys.version_info))')
PYTHON_CONFIG = python$(PYTHON_VERSION)-config
PYTHON_MODULE_SUFFIX = $(shell $(PYTHON_CONFIG) --extension-suffix)

LIB_DIR = $(ACMACSD_ROOT)/lib
ACMACS_MAP_DRAW_LIB = $(DIST)/libacmacsmapdraw.so

# -fvisibility=hidden and -flto make resulting lib smaller (pybind11) but linking is much slower
OPTIMIZATION = -O3 #-fvisibility=hidden -flto
PROFILE = # -pg
CXXFLAGS = -g -MMD $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WEVERYTHING) $(WARNINGS) -Icc -I$(BUILD)/include -I$(ACMACSD_ROOT)/include $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LDLIBS = -L$(LIB_DIR) -lacmacsbase -lacmacschart -lacmacsdraw -llocationdb -lhidb -lseqdb -lboost_date_time -lboost_filesystem -lboost_system $$(pkg-config --libs cairo) $$(pkg-config --libs liblzma) $$($(PYTHON_CONFIG) --ldflags | sed -E 's/-Wl,-stack_size,[0-9]+//')

PKG_INCLUDES = $(shell pkg-config --cflags cairo) $(shell pkg-config --cflags liblzma) $(shell $(PYTHON_CONFIG) --includes)

# ----------------------------------------------------------------------

# BUILD = build
# DIST = $(abspath dist)

all: check-acmacsd-root $(ACMACS_MAP_DRAW_LIB) $(BACKEND)

install: check-acmacsd-root install-headers $(ACMACS_MAP_DRAW_LIB) $(BACKEND)
	ln -sf $(ACMACS_MAP_DRAW_LIB) $(ACMACSD_ROOT)/lib
	if [ $$(uname) = "Darwin" ]; then /usr/bin/install_name_tool -id $(ACMACSD_ROOT)/lib/$(notdir $(ACMACS_MAP_DRAW_LIB)) $(ACMACSD_ROOT)/lib/$(notdir $(ACMACS_MAP_DRAW_LIB)); fi
	ln -sf $(BACKEND) $(ACMACSD_ROOT)/py
	ln -sf $(abspath py)/* $(ACMACSD_ROOT)/py
	ln -sf $(abspath bin)/acmacs-map-* $(ACMACSD_ROOT)/bin
	ln -sf $(abspath bin)/geographic-* $(ACMACSD_ROOT)/bin

install-headers: check-acmacsd-root
	if [ ! -d $(ACMACSD_ROOT)/include/acmacs-map-draw ]; then mkdir $(ACMACSD_ROOT)/include/acmacs-map-draw; fi
	ln -sf $(abspath cc)/*.hh $(ACMACSD_ROOT)/include/acmacs-map-draw

test: install
	test/test -v

include $(ACMACSD_ROOT)/share/Makefile.rtags

# ----------------------------------------------------------------------

-include $(BUILD)/*.d

# ----------------------------------------------------------------------

$(ACMACS_MAP_DRAW_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(SOURCES)) | $(DIST)
	$(CXX) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BACKEND): $(patsubst %.cc,$(BUILD)/%.o,$(PY_SOURCES)) | $(DIST)
	$(CXX) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

# ----------------------------------------------------------------------

$(BUILD)/%.o: cc/%.cc | $(BUILD) install-headers
	@echo $<
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

# ----------------------------------------------------------------------

check-acmacsd-root:
ifndef ACMACSD_ROOT
	$(error ACMACSD_ROOT is not set)
endif

include $(ACMACSD_ROOT)/share/Makefile.dist-build.rules

.PHONY: check-acmacsd-root

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
