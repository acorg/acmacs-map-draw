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
#include "acmacs-base/rjson.hh"
//#include "acmacs-base/virus-name.hh"
// #include "locationdb/locdb.hh"
#include "seqdb/seqdb.hh"
#include "acmacs-chart-2/chart-modify.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-chart-2/ace-export.hh"
#include "acmacs-chart-2/factory-export.hh"
#include "acmacs-chart-2/lispmds-export.hh"
#include "acmacs-map-draw/draw.hh"

static void register_hooks(apr_pool_t *pool);
static int acmacs_handler(request_rec *r);
static void make_html(request_rec *r, const char* view_mode, const char* coloring);
static void make_ace(request_rec *r);
static int process_post_request(request_rec *r);
static void command_download_pdf(request_rec *r, const rjson::object& args);
static void command_download_ace(request_rec *r, const rjson::object& args);
static void command_download_save(request_rec *r, const rjson::object& args);
static void command_download_layout(request_rec *r, const rjson::object& args);
static void command_download_table_map_distances(request_rec *r, const rjson::object& args);
static void command_download_distances_between_all_points(request_rec *r, const rjson::object& args);
static void command_download_error_lines(request_rec *r, const rjson::object& args);

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
        int rc = OK;
        if (acv == "html") {
            const char* view_mode = apr_table_get(GET, "view-mode");
            const char* coloring = apr_table_get(GET, "coloring");
            make_html(r, view_mode ? view_mode : "best-projection", coloring ? coloring : "default");
        }
        else if (acv == "ace") {
            make_ace(r);
        }
        else if (acv == "post" && r->method_number == M_POST) {
            rc = process_post_request(r);
        }
        else {
            rc = DECLINED;
        }
        return rc;
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
     import * as mod from "/js/ad/map-draw/ace-view-1/apache-mod-acmacs.js";
     $(document).ready(() => mod.show_antigenic_map_widget({parent: $("#map1"), view_mode: {mode: "%s"}, coloring: "%s", uri: "%s"}));
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
    antigens->set_continent();
    seqdb::add_clades(chart, seqdb::ignore_errors::yes, seqdb::report::yes);

    ap_set_content_type(r, "application/json");
    r->content_encoding = "gzip";
    const auto exported = acmacs::chart::export_ace(chart, "mod_acmacs", 0);
    const auto compressed = acmacs::file::gzip_compress(exported);
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // make_ace

// ----------------------------------------------------------------------

int process_post_request(request_rec* r)
{
    std::string source_data;
    if (auto rc = ap_setup_client_block(r, REQUEST_CHUNKED_ERROR); rc != OK)
        return rc;
    if (ap_should_client_block(r)) {
        char buffer[HUGE_STRING_LEN];
        long block_size;
        while ((block_size = ap_get_client_block(r, buffer, sizeof(buffer))) > 0)
            source_data.append(buffer, static_cast<size_t>(block_size));
    }

    const auto data = rjson::parse_string(source_data);
    if (std::string command = data.get_or_default("C", ""); !command.empty()) {
        if (command == "download_pdf")
            command_download_pdf(r, data);
        else if (command == "download_ace")
            command_download_ace(r, data);
        else if (command == "download_save")
            command_download_save(r, data);
        else if (command == "download_layout")
            command_download_layout(r, data);
        else if (command == "download_table_map_distances")
            command_download_table_map_distances(r, data);
        else if (command == "download_distances_between_all_points")
            command_download_distances_between_all_points(r, data);
        else if (command == "download_error_lines")
            command_download_error_lines(r, data);
        else
            std::cerr << "ERROR: mod_acmacs: unrecognized command in the post request: " << source_data << '\n';
    }
    else {
        std::cerr << "ERROR: mod_acmacs: cannot get command in the post request: " << source_data << '\n';
    }
    return OK;

} // process_post_request

// ----------------------------------------------------------------------

void command_download_pdf(request_rec *r, const rjson::object& args)
{
    const auto projection_no = args.get_or_default("projection_no", 0UL);
    ChartDraw chart_draw(std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(r->filename, acmacs::chart::Verify::None, report_time::No)), projection_no);
    chart_draw.calculate_viewport();

    ap_set_content_type(r, "application/pdf");
    r->content_encoding = "gzip";
    const auto compressed = acmacs::file::gzip_compress(chart_draw.draw_pdf(800));
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // command_download_pdf

// ----------------------------------------------------------------------

void command_download_ace(request_rec *r, const rjson::object& /*args*/)
{
    auto chart = acmacs::chart::import_from_file(r->filename, acmacs::chart::Verify::None, report_time::No);
    auto data = acmacs::chart::export_factory(*chart, acmacs::chart::export_format::ace, "apache-mod-acmacs", report_time::No);
    ap_set_content_type(r, "application/octet-stream");
    ap_rwrite(data.data(), static_cast<int>(data.size()), r);

} // command_download_ace

// ----------------------------------------------------------------------

void command_download_save(request_rec* r, const rjson::object& /*args*/)
{
    auto chart = acmacs::chart::import_from_file(r->filename, acmacs::chart::Verify::None, report_time::No);
    const auto compressed = acmacs::file::gzip_compress(acmacs::chart::export_lispmds(*chart, "acmacs-api"));
    ap_set_content_type(r, "application/octet-stream");
    r->content_encoding = "gzip";
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // command_download_save

// ----------------------------------------------------------------------

void command_download_layout(request_rec *r, const rjson::object& args)
{
    auto chart = acmacs::chart::import_from_file(r->filename, acmacs::chart::Verify::None, report_time::No);
    std::string layout;
    if (args.get_or_default("format", "text") == "csv")
        layout = acmacs::chart::export_layout<acmacs::DataFormatterCSV>(*chart, args.get_or_default("projection_no", 0UL));
    else
        layout = acmacs::chart::export_layout<acmacs::DataFormatterSpaceSeparated>(*chart, args.get_or_default("projection_no", 0UL));
    const auto compressed = acmacs::file::gzip_compress(layout);
    ap_set_content_type(r, "application/octet-stream");
    r->content_encoding = "gzip";
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // command_download_layout

// ----------------------------------------------------------------------

void command_download_table_map_distances(request_rec *r, const rjson::object& args)
{
    auto chart = acmacs::chart::import_from_file(r->filename, acmacs::chart::Verify::None, report_time::No);
    std::string distances;
    if (args.get_or_default("format", "text") == "csv")
        distances = acmacs::chart::export_table_map_distances<acmacs::DataFormatterCSV>(*chart, args.get_or_default("projection_no", 0UL));
    else
        distances = acmacs::chart::export_table_map_distances<acmacs::DataFormatterSpaceSeparated>(*chart, args.get_or_default("projection_no", 0UL));
    const auto compressed = acmacs::file::gzip_compress(distances);
    ap_set_content_type(r, "application/octet-stream");
    r->content_encoding = "gzip";
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // command_download_table_map_distances

// ----------------------------------------------------------------------

void command_download_distances_between_all_points(request_rec *r, const rjson::object& args)
{
    auto chart = acmacs::chart::import_from_file(r->filename, acmacs::chart::Verify::None, report_time::No);
    std::string distances;
    if (args.get_or_default("format", "text") == "csv")
        distances = acmacs::chart::export_distances_between_all_points<acmacs::DataFormatterCSV>(*chart, args.get_or_default("projection_no", 0UL));
    else
        distances = acmacs::chart::export_distances_between_all_points<acmacs::DataFormatterSpaceSeparated>(*chart, args.get_or_default("projection_no", 0UL));
    const auto compressed = acmacs::file::gzip_compress(distances);
    ap_set_content_type(r, "application/octet-stream");
    r->content_encoding = "gzip";
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // command_download_distances_between_all_points

// ----------------------------------------------------------------------

void command_download_error_lines(request_rec *r, const rjson::object& args)
{
    auto chart = acmacs::chart::import_from_file(r->filename, acmacs::chart::Verify::None, report_time::No);
    std::string error_lines;
    if (args.get_or_default("format", "text") == "csv")
        error_lines = acmacs::chart::export_error_lines<acmacs::DataFormatterCSV>(*chart, args.get_or_default("projection_no", 0UL));
    else
        error_lines = acmacs::chart::export_error_lines<acmacs::DataFormatterSpaceSeparated>(*chart, args.get_or_default("projection_no", 0UL));
    const auto compressed = acmacs::file::gzip_compress(error_lines);
    ap_set_content_type(r, "application/octet-stream");
    r->content_encoding = "gzip";
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // command_download_error_lines

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
