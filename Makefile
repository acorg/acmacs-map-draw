# -*- Makefile -*-
# ----------------------------------------------------------------------

TARGETS = \
  $(ACMACS_MAP_DRAW_LIB) \
  $(DIST)/mapi \
  $(DIST)/chart-select \
  $(DIST)/map-draw \
  $(DIST)/map-draw-interactive \
  $(DIST)/map-procrustes \
  $(DIST)/map-hemisphering \
  $(DIST)/geographic-draw \
  $(DIST)/chart-layout-sequences \
  $(DIST)/mod_acmacs.so

ACMACS_MAP_DRAW_SOURCES =        \
  mapi-settings-antigens.cc      \
  mapi-settings-serum-circles.cc \
  mapi-settings-drawing.cc       \
  mapi-settings.cc               \
  mapi-settings-legend.cc        \
  mapi-settings-labels.cc        \
  mod-applicator.cc              \
  select.cc                      \
  geographic-map.cc              \
  mod-amino-acids.cc             \
  select-filter.cc               \
  mod-serum.cc                   \
  mod-connection-lines.cc        \
  mod-blobs.cc                   \
  export.cc                      \
  map-elements.cc                \
  vaccines.cc                    \
  mod-procrustes.cc              \
  draw.cc                        \
  report-antigens.cc             \
  vaccine-matcher.cc             \
  labels.cc                      \
  setup-dbs.cc                   \
  point-style-draw.cc            \
  chart-select-interface.cc      \
  mapi-settings-procrustes.cc    \
  mapi-settings-sequences.cc     \
  mapi-settings-vaccine.cc       \
  mapi-settings-time-series.cc   \
  map-elements-v1.cc             \
  map-elements-v2.cc             \
  hemisphering-data.cc           \
  coordinates.cc                 \
  geographic-settings.cc

ACMACS_MAP_DRAW_LIB_MAJOR = 2
ACMACS_MAP_DRAW_LIB_MINOR = 0
ACMACS_MAP_DRAW_LIB_NAME = libacmacsmapdraw
ACMACS_MAP_DRAW_LIB = $(DIST)/$(call shared_lib_name,$(ACMACS_MAP_DRAW_LIB_NAME),$(ACMACS_MAP_DRAW_LIB_MAJOR),$(ACMACS_MAP_DRAW_LIB_MINOR))

# ----------------------------------------------------------------------

all: install

CONFIGURE_CAIRO = 1
include $(ACMACSD_ROOT)/share/Makefile.config

LDLIBS = \
  $(AD_LIB)/$(call shared_lib_name,libacmacsbase,1,0) \
  $(AD_LIB)/$(call shared_lib_name,liblocationdb,1,0) \
  $(AD_LIB)/$(call shared_lib_name,libacmacsvirus,1,0) \
  $(AD_LIB)/$(call shared_lib_name,libacmacschart,2,0) \
  $(AD_LIB)/$(call shared_lib_name,libacmacswhoccdata,1,0) \
  $(AD_LIB)/$(call shared_lib_name,libhidb,5,0) \
  $(AD_LIB)/$(call shared_lib_name,libseqdb,3,0) \
  $(AD_LIB)/$(call shared_lib_name,libacmacsdraw,1,0) \
  $(CAIRO_LIBS) $(XZ_LIBS) $(CXX_LIBS)

# ----------------------------------------------------------------------

install: install-headers install-acmacs-map-draw-lib $(TARGETS)
	$(call install_all,$(AD_PACKAGE_NAME))
	$(call install_wildcard,$(abspath js)/ace-view,$(AD_SHARE)/js/map-draw)
	# $(call symbolic_link_wildcard,$(abspath conf)/*.json,$(AD_CONF))
	# $(call symbolic_link_wildcard,$(abspath bin)/*,$(AD_BIN))
	# $(call symbolic_link_wildcard,$(DIST)/map*,$(AD_BIN))
	# $(call symbolic_link_wildcard,$(DIST)/chart-*,$(AD_BIN))
	# $(call symbolic_link_wildcard,$(DIST)/geographic-*,$(AD_BIN))
	# $(call symbolic_link,$(DIST)/mod_acmacs.so,$(AD_LIB))
	# $(call symbolic_link_wildcard,$(abspath doc)/*.org,$(AD_DOC))
    # $(call make_dir,$(AD_SHARE)/js/map-draw)
	# $(call symbolic_link_wildcard,$(abspath js)/ace-view,$(AD_SHARE)/js/map-draw)

# $(call make_dir,$(AD_TEMPLATES)/mapi)
# $(call symbolic_link_wildcard,$(abspath templates)/*,$(AD_TEMPLATES)/mapi)

install-acmacs-map-draw-lib: $(ACMACS_MAP_DRAW_LIB)
	$(call install_lib,$(ACMACS_MAP_DRAW_LIB))

test: install
	test/test -v
.PHONY: test

# ----------------------------------------------------------------------

$(ACMACS_MAP_DRAW_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_MAP_DRAW_SOURCES)) | $(DIST) install-headers
	$(call echo_shared_lib,$@)
	$(call make_shared_lib,$(ACMACS_MAP_DRAW_LIB_NAME),$(ACMACS_MAP_DRAW_LIB_MAJOR),$(ACMACS_MAP_DRAW_LIB_MINOR)) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(DIST)/%: $(BUILD)/%.o | $(ACMACS_MAP_DRAW_LIB) install-headers
	$(call echo_link_exe,$@)
	$(CXX) $(LDFLAGS) -o $@ $^ $(ACMACS_MAP_DRAW_LIB) $(LDLIBS) $(AD_RPATH)

# ----------------------------------------------------------------------

APXS_CXX = -S CC=$(CXX) -Wc,-xc++ -Wl,-shared
APXS_ENV = LTFLAGS="-v"
APXS_LIBS_NAMES = acmacsbase.1 acmacsvirus.1 acmacschart.2 acmacswhoccdata.1 acmacsmapdraw.2 locationdb.1 seqdb.3
ifeq ($(PLATFORM),darwin)
  APXS_LIBS = -L$(AD_LIB) $(APXS_LIBS_NAMES:%=-l%)
else
  APXS_LIBS_NAMES_FIXED = $(basename $(APXS_LIBS_NAMES))
  APXS_LIBS = -L$(AD_LIB) $(APXS_LIBS_NAMES_FIXED:%=-l%)
endif
APXS_CXXFLAGS = $(CXXFLAGS) -Wno-missing-field-initializers

$(DIST)/mod_acmacs.so: $(BUILD)/.libs/apache-mod-acmacs.so
	$(call symbolic_link,$^,$@)

$(BUILD)/.libs/apache-mod-acmacs.so: cc/apache-mod-acmacs.cc | $(ACMACS_MAP_DRAW_LIB)
	echo apxs does not not understand any file suffixes besides .c, so we have to use .c for C++
	$(call symbolic_link,$(abspath $^),$(BUILD)/$(basename $(notdir $^)).c)
	env $(APXS_ENV) apxs $(APXS_CXX) $(APXS_CXXFLAGS:%=-Wc,%) -n acmacs_module $(APXS_LIBS) -c $(BUILD)/$(basename $(notdir $^)).c

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
