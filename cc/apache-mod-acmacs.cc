// -*- C++ -*-

// httpd.conf:
//
// LoadModule acmacs_module "mod_acmacs.so"
// AddHandler acmacs .ace .save .save.xz .acd1 .acd1.xz
//
// /etc/apache2/envvars:
//
// export ACMACSD_ROOT="/home/.../AD"

#include <iostream>
#include <string>
#include <typeinfo>

#include "apache2/httpd.h"
#include "apache2/http_core.h"
#include "apache2/http_protocol.h"
#include "apache2/http_request.h"
#include "apache2/http_log.h"
#include "apache2/util_script.h"

#include "acmacs-base/filesystem.hh"
#include "acmacs-chart-2/chart-modify.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-chart-2/ace-export.hh"

static void register_hooks(apr_pool_t *pool);
static int acmacs_handler(request_rec *r);
static void make_html(request_rec *r);
static void make_ace(request_rec *r);

// ----------------------------------------------------------------------

extern "C" module acmacs_module;
extern "C"  {
module AP_MODULE_DECLARE_DATA acmacs_module = {
    STANDARD20_MODULE_STUFF, nullptr, nullptr, nullptr, nullptr, nullptr, register_hooks
};
}
APLOG_USE_MODULE(acmacs);

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wused-but-marked-unused"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-macros"
#endif

static void register_hooks(apr_pool_t * /*pool*/) {
    ap_hook_handler(acmacs_handler, nullptr, nullptr, APR_HOOK_LAST);
}

#define AP_WARN APLOG_MARK, APLOG_WARNING, 0
#define AP_ERR APLOG_MARK, APLOG_ERR, 0

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------

static int acmacs_handler(request_rec *r) {
    if (!r->handler || r->handler != std::string("acmacs"))
        return DECLINED;

    apr_table_t *GET;
    ap_args_to_table(r, &GET);
    const char* acv_c = apr_table_get(GET, "acv");
    const std::string acv = acv_c ? acv_c : "";

    if (!fs::exists(fs::path(r->filename)))
        return HTTP_NOT_FOUND;

    try {
        if (acv == "html") {
            make_html(r);
        }
        else if (acv == "ace") {
            make_ace(r);
        }
        else {
            return DECLINED;
        }
        return OK;
    }
    catch (std::exception& err) {
        ap_log_rerror(AP_ERR, r, "%s: %s", typeid(err).name(), err.what());
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    return HTTP_INTERNAL_SERVER_ERROR;
}

// ----------------------------------------------------------------------

static const char* sHtml = R"(
<!DOCTYPE html>
<html>
  <head>
   <meta charset="utf-8" />
   <title>%s</title>
   <script src="https://code.jquery.com/jquery-3.3.1.min.js"></script>
   <script type="module">
     import * as acv_m from "/js/ad/map-draw/ace-view-1/ace-view.js";
     $(document).ready(() => new acv_m.AntigenicMapWidget($("#map1"), "%s?acv=ace"));
   </script>
  </head>
  <body>
    <div id="map1"></div>
  </body>
</html>
)";

void make_html(request_rec *r)
{
    // ap_log_rerror(AP_WARN, r, "uri: %s", r->uri);
    // ap_log_rerror(AP_WARN, r, "path_info: %s", r->path_info);

    ap_set_content_type(r, "text/html");
    ap_rprintf(r, sHtml, r->filename, r->uri);

} // make_html

// ----------------------------------------------------------------------

void make_ace(request_rec *r)
{
    acmacs::chart::ChartModify chart(acmacs::chart::import_from_file(r->filename, acmacs::chart::Verify::None, report_time::No));

    ap_set_content_type(r, "application/json");
    ap_rputs(ace_export(chart, "mod_acmacs").data(), r);

} // make_ace

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
