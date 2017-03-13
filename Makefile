# -*- Makefile -*-
# Eugene Skepner 2016

# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

SOURCES = draw.cc point-style-draw.cc map-elements.cc labels.cc geographic-map.cc time-series.cc
PY_SOURCES = py.cc $(SOURCES)
BACKEND = $(DIST)/acmacs_map_draw_backend$(PYTHON_MODULE_SUFFIX)

# ----------------------------------------------------------------------

CLANG = $(shell if g++ --version 2>&1 | grep -i llvm >/dev/null; then echo Y; else echo N; fi)
ifeq ($(CLANG),Y)
  WEVERYTHING = -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded
  WARNINGS = -Wno-weak-vtables # -Wno-padded
  STD = c++14
else
  WEVERYTHING = -Wall -Wextra
  WARNINGS =
  STD = c++14
endif

PYTHON_VERSION = $(shell python3 -c 'import sys; print("{0.major}.{0.minor}".format(sys.version_info))')
PYTHON_CONFIG = python$(PYTHON_VERSION)-config
PYTHON_MODULE_SUFFIX = $(shell $(PYTHON_CONFIG) --extension-suffix)

LIB_DIR = $(ACMACSD_ROOT)/lib

# -fvisibility=hidden and -flto make resulting lib smaller (pybind11) but linking is much slower
OPTIMIZATION = -O3 #-fvisibility=hidden -flto
PROFILE = # -pg
CXXFLAGS = -g -MMD $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WEVERYTHING) $(WARNINGS) -Icc -I$(BUILD)/include -I$(ACMACSD_ROOT)/include $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LDLIBS = -L$(LIB_DIR) -lacmacsbase -lacmacschart -lacmacsdraw -llocationdb -lhidb -lseqdb -lboost_date_time -lboost_filesystem -lboost_system $$(pkg-config --libs cairo) $$(pkg-config --libs liblzma) $$($(PYTHON_CONFIG) --ldflags | sed -E 's/-Wl,-stack_size,[0-9]+//')

PKG_INCLUDES = $$(pkg-config --cflags cairo) $$(pkg-config --cflags liblzma) $$($(PYTHON_CONFIG) --includes)

# ----------------------------------------------------------------------

BUILD = build
DIST = $(abspath dist)

all: check-acmacsd-root $(BACKEND)

install: check-acmacsd-root install-headers $(BACKEND)
	ln -sf $(BACKEND) $(ACMACSD_ROOT)/lib
	ln -sf $(BACKEND) $(ACMACSD_ROOT)/py
	ln -sf $(abspath py)/* $(ACMACSD_ROOT)/py
	ln -sf $(abspath bin)/acmacs-map-* $(ACMACSD_ROOT)/bin
	ln -sf $(abspath bin)/geographic-map-draw $(ACMACSD_ROOT)/bin

install-headers: check-acmacsd-root
	if [ ! -d $(ACMACSD_ROOT)/include/acmacs-map-draw ]; then mkdir $(ACMACSD_ROOT)/include/acmacs-map-draw; fi
	ln -sf $(abspath cc)/*.hh $(ACMACSD_ROOT)/include/acmacs-map-draw

test: install
	test/test

# ----------------------------------------------------------------------

-include $(BUILD)/*.d

# ----------------------------------------------------------------------

$(BACKEND): $(patsubst %.cc,$(BUILD)/%.o,$(PY_SOURCES)) | $(DIST)
	g++ -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -rf $(DIST) $(BUILD)/*.o $(BUILD)/*.d

distclean: clean
	rm -rf $(BUILD)

# ----------------------------------------------------------------------

$(BUILD)/%.o: cc/%.cc | $(BUILD) install-headers
	@echo $<
	@g++ $(CXXFLAGS) -c -o $@ $<

# ----------------------------------------------------------------------

check-acmacsd-root:
ifndef ACMACSD_ROOT
	$(error ACMACSD_ROOT is not set)
endif

$(DIST):
	mkdir -p $(DIST)

$(BUILD):
	mkdir -p $(BUILD)

.PHONY: check-acmacsd-root

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
