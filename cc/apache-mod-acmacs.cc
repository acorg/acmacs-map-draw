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
#include "acmacs-base/gzip.hh"
#include "acmacs-base/range.hh"
//#include "acmacs-base/virus-name.hh"
// #include "locationdb/locdb.hh"
#include "seqdb/seqdb.hh"
#include "acmacs-chart-2/chart-modify.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-chart-2/ace-export.hh"

static void register_hooks(apr_pool_t *pool);
static int acmacs_handler(request_rec *r);
static void make_html(request_rec *r, const char* view_mode, const char* coloring);
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
            const char* view_mode = apr_table_get(GET, "view-mode");
            const char* coloring = apr_table_get(GET, "coloring");
            make_html(r, view_mode ? view_mode : "best-projection", coloring ? coloring : "default");
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
     const options = {view_mode: "%s", coloring: "%s"};
     $(document).ready(() => new acv_m.AntigenicMapWidget($("#map1"), "%s?acv=ace", options));
   </script>
  </head>
  <body>
    <div id="map1"></div>
  </body>
</html>
)";

void make_html(request_rec *r, const char* view_mode, const char* coloring)
{
    // ap_log_rerror(AP_WARN, r, "uri: %s", r->uri);
    // ap_log_rerror(AP_WARN, r, "path_info: %s", r->path_info);

    ap_set_content_type(r, "text/html");
    ap_rprintf(r, sHtml, r->filename, view_mode, coloring, r->uri);

} // make_html

// ----------------------------------------------------------------------

void make_ace(request_rec* r)
{

    acmacs::chart::ChartModify chart(acmacs::chart::import_from_file(r->filename, acmacs::chart::Verify::None, report_time::No));
    auto antigens = chart.antigens_modify();

      // set continent info
    antigens->set_continent();
    // const auto& locdb = get_locdb(report_time::Yes);
    // for (auto antigen_no : acmacs::range(antigens->size())) {
    //     auto& antigen = antigens->at(antigen_no);
    //     if (antigen.continent().empty()) {
    //         try {
    //             antigen.continent(locdb.continent(virus_name::location(antigen.name())));
    //         }
    //         catch (std::exception& err) {
    //             ap_log_rerror(AP_WARN, r, "cannot figure out continent for \"%s\": %s", antigen.name().data(), err.what());
    //         }
    //         catch (...) {
    //             ap_log_rerror(AP_WARN, r, "cannot figure out continent for \"%s\": unknown exception", antigen.name().data());
    //         }
    //     }
    // }

    // set clade info
    seqdb::add_clades(chart);

    // const auto& seqdb = seqdb::get(report_time::Yes);
    // for (auto antigen_no : acmacs::range(antigens->size())) {
    //     auto& antigen = antigens->at(antigen_no);
    //     try {
    //         const auto* entry_seq = seqdb.find_hi_name(antigen.full_name());
    //         if (entry_seq) {
    //             for (const auto& clade : entry_seq->seq().clades()) {
    //                 antigen.add_clade(clade);
    //             }
    //         }
    //     }
    //     catch (std::exception& err) {
    //         ap_log_rerror(AP_WARN, r, "cannot figure out clade for \"%s\": %s", antigen.name().data(), err.what());
    //     }
    //     catch (...) {
    //         ap_log_rerror(AP_WARN, r, "cannot figure out clade for \"%s\": unknown exception", antigen.name().data());
    //     }
    // }

    ap_set_content_type(r, "application/json");
    r->content_encoding = "gzip";
    const auto exported = ace_export(chart, "mod_acmacs", 0);
    const auto compressed = acmacs::file::gzip_compress(exported);
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // make_ace

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
