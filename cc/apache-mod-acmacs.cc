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
#include "seqdb-3/seqdb.hh"
#include "acmacs-chart-2/chart-modify.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-chart-2/ace-export.hh"
#include "acmacs-chart-2/factory-export.hh"
#include "acmacs-chart-2/lispmds-export.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/export.hh"

static void register_hooks(apr_pool_t *pool);
static int acmacs_handler(request_rec *r);
static void make_html201805(request_rec *r, const char* view_mode, const char* coloring);
static void make_html201807(request_rec *r, const char* view_mode, const char* coloring, const char* viewport);
static void make_ace(request_rec *r);
static int process_post_request(request_rec *r);
static void command_download_pdf(request_rec *r, const rjson::value& args);
static void command_download_ace(request_rec *r, const rjson::value& args);
static void command_download_save(request_rec *r, const rjson::value& args);
static void command_download_layout(request_rec *r, const rjson::value& args);
static void command_download_table_map_distances(request_rec *r, const rjson::value& args);
static void command_download_distances_between_all_points(request_rec *r, const rjson::value& args);
static void command_download_error_lines(request_rec *r, const rjson::value& args);
static void command_sequences_of_chart(request_rec *r, const rjson::value& args);
static void command_download_sequences_of_chart_as_fasta(request_rec *r, const rjson::value& args);
static void command_download_layout_sequences_as_csv(request_rec *r, const rjson::value& args);

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
        if (acv == "html" || acv == "html201807") {
            // const char* view_mode = apr_table_get(GET, "view-mode");
            // const char* coloring = apr_table_get(GET, "coloring");
            const char* viewport = apr_table_get(GET, "viewport");
            make_html201807(r, "all", "original", viewport);
        }
        else if (acv == "html201805") {
            const char* view_mode = apr_table_get(GET, "view-mode");
            const char* coloring = apr_table_get(GET, "coloring");
            make_html201805(r, view_mode ? view_mode : "best-projection", coloring ? coloring : "default");
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

static const char* sHtml201807 = R"(
<!DOCTYPE html>
<html>
  <head>
   <meta charset="utf-8" />
   <title>%s</title>
   <script src="https://code.jquery.com/jquery-3.3.1.min.js"></script>
   <script type="module">
     import * as mod from "/js/ad/map-draw/ace-view/201807/apache-mod-acmacs.js";
     $(document).ready(() => mod.show_antigenic_map_widget({parent: $("#map1"), view_mode: {mode: "%s"}, coloring: "%s", viewport: "%s", uri: "%s"}));
   </script>
   <style>body { margin: 0; padding: 0; font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif; background-color: #F8F8F8; }</style>
  </head>
  <body>
    <div id="map1"></div>
  </body>
</html>
)";

void make_html201807(request_rec *r, const char* view_mode, const char* coloring, const char* viewport)
{
    ap_set_content_type(r, "text/html");
    ap_rprintf(r, sHtml201807, r->filename, view_mode, coloring, viewport ? viewport : "", r->uri);

} // make_html201807

// ----------------------------------------------------------------------

static const char* sHtml201805 = R"(
<!DOCTYPE html>
<html>
  <head>
   <meta charset="utf-8" />
   <title>%s</title>
   <script src="https://code.jquery.com/jquery-3.3.1.min.js"></script>
   <script type="module">
     import * as mod from "/js/ad/map-draw/ace-view/201805/apache-mod-acmacs.js";
     $(document).ready(() => mod.show_antigenic_map_widget({parent: $("#map1"), view_mode: {mode: "%s"}, coloring: "%s", uri: "%s"}));
   </script>
   <style>body { margin: 0; padding: 0; font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif; }</style>
  </head>
  <body>
    <div id="map1"></div>
  </body>
</html>
)";

void make_html201805(request_rec *r, const char* view_mode, const char* coloring)
{
    // ap_log_rerror(AP_WARN, r, "uri: %s", r->uri);
    // ap_log_rerror(AP_WARN, r, "path_info: %s", r->path_info);

    ap_set_content_type(r, "text/html");
    ap_rprintf(r, sHtml201805, r->filename, view_mode, coloring, r->uri);

} // make_html

// ----------------------------------------------------------------------

void make_ace(request_rec* r)
{
    acmacs::chart::ChartModify chart(acmacs::chart::import_from_file(std::string(r->filename), acmacs::chart::Verify::None, report_time::no));
    auto antigens = chart.antigens_modify();
    antigens->set_continent();
    seqdb::add_clades(chart);

    ap_set_content_type(r, "application/json");
    r->content_encoding = "gzip";

      //rjson::v1 helping during rjson::v1 to rjson::v2 transition
    auto make_ace_helper = [](auto ace, const auto& group_sets) -> std::string {
        //rjson::v1 if constexpr (std::is_same_v<decltype(ace), rjson::v2::value>) {
            if (!group_sets.is_null())
                ace["c"]["group_sets"] = group_sets;
            return rjson::to_string(ace);
        //rjson::v1 }
        //rjson::v1 else {
        //rjson::v1     if (group_sets != rjson::v1::null{})
        //rjson::v1         ace["c"].set_field("group_sets", group_sets);
        //rjson::v1     return ace.to_json();
        //rjson::v1 }
    };

    const auto exported = make_ace_helper(acmacs::chart::export_ace_to_rjson(chart, "mod_acmacs"), chart.extension_field("group_sets"));
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
    if (const std::string_view command = data["C"]; !command.empty()) {
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
        else if (command == "sequences_of_chart")
            command_sequences_of_chart(r, data);
        else if (command == "download_sequences_of_chart_as_fasta")
            command_download_sequences_of_chart_as_fasta(r, data);
        else if (command == "download_layout_sequences_as_csv")
            command_download_layout_sequences_as_csv(r, data);
        else
            std::cerr << "ERROR: mod_acmacs: unrecognized command in the post request: " << source_data << '\n';
    }
    else {
        std::cerr << "ERROR: mod_acmacs: cannot get command in the post request: " << source_data << '\n';
    }
    return OK;

} // process_post_request

// ----------------------------------------------------------------------

void command_download_pdf(request_rec *r, const rjson::value& args)
{
    const auto projection_no = rjson::get_or(args, "projection_no", 0UL);
    ChartDraw chart_draw(std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(r->filename, acmacs::chart::Verify::None, report_time::no)), projection_no);
    chart_draw.calculate_viewport();

    ap_set_content_type(r, "application/pdf");
    r->content_encoding = "gzip";
    const auto compressed = acmacs::file::gzip_compress(chart_draw.draw_pdf(800));
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // command_download_pdf

// ----------------------------------------------------------------------

void command_download_ace(request_rec *r, const rjson::value& /*args*/)
{
    auto chart = acmacs::chart::import_from_file(std::string(r->filename), acmacs::chart::Verify::None, report_time::no);
    auto data = acmacs::chart::export_factory(*chart, acmacs::chart::export_format::ace, "apache-mod-acmacs", report_time::no);
    ap_set_content_type(r, "application/octet-stream");
    ap_rwrite(data.data(), static_cast<int>(data.size()), r);

} // command_download_ace

// ----------------------------------------------------------------------

void command_download_save(request_rec* r, const rjson::value& /*args*/)
{
    auto chart = acmacs::chart::import_from_file(std::string(r->filename), acmacs::chart::Verify::None, report_time::no);
    const auto compressed = acmacs::file::gzip_compress(acmacs::chart::export_lispmds(*chart, "acmacs-api"));
    ap_set_content_type(r, "application/octet-stream");
    r->content_encoding = "gzip";
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // command_download_save

// ----------------------------------------------------------------------

void command_download_layout(request_rec *r, const rjson::value& args)
{
    auto chart = acmacs::chart::import_from_file(std::string(r->filename), acmacs::chart::Verify::None, report_time::no);
    std::string layout;
    if (rjson::get_or(args, "format", "text") == "csv")
        layout = acmacs::chart::export_layout<acmacs::DataFormatterCSV>(*chart, rjson::get_or(args, "projection_no", 0UL));
    else
        layout = acmacs::chart::export_layout<acmacs::DataFormatterSpaceSeparated>(*chart, rjson::get_or(args, "projection_no", 0UL));
    const auto compressed = acmacs::file::gzip_compress(layout);
    ap_set_content_type(r, "application/octet-stream");
    r->content_encoding = "gzip";
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // command_download_layout

// ----------------------------------------------------------------------

void command_download_table_map_distances(request_rec *r, const rjson::value& args)
{
    auto chart = acmacs::chart::import_from_file(std::string(r->filename), acmacs::chart::Verify::None, report_time::no);
    std::string distances;
    if (rjson::get_or(args, "format", "text") == "csv")
        distances = acmacs::chart::export_table_map_distances<acmacs::DataFormatterCSV>(*chart, rjson::get_or(args, "projection_no", 0UL));
    else
        distances = acmacs::chart::export_table_map_distances<acmacs::DataFormatterSpaceSeparated>(*chart, rjson::get_or(args, "projection_no", 0UL));
    const auto compressed = acmacs::file::gzip_compress(distances);
    ap_set_content_type(r, "application/octet-stream");
    r->content_encoding = "gzip";
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // command_download_table_map_distances

// ----------------------------------------------------------------------

void command_download_distances_between_all_points(request_rec *r, const rjson::value& args)
{
    auto chart = acmacs::chart::import_from_file(std::string(r->filename), acmacs::chart::Verify::None, report_time::no);
    std::string distances;
    if (rjson::get_or(args, "format", "text") == "csv")
        distances = acmacs::chart::export_distances_between_all_points<acmacs::DataFormatterCSV>(*chart, rjson::get_or(args, "projection_no", 0UL));
    else
        distances = acmacs::chart::export_distances_between_all_points<acmacs::DataFormatterSpaceSeparated>(*chart, rjson::get_or(args, "projection_no", 0UL));
    const auto compressed = acmacs::file::gzip_compress(distances);
    ap_set_content_type(r, "application/octet-stream");
    r->content_encoding = "gzip";
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // command_download_distances_between_all_points

// ----------------------------------------------------------------------

void command_download_error_lines(request_rec *r, const rjson::value& args)
{
    auto chart = acmacs::chart::import_from_file(std::string(r->filename), acmacs::chart::Verify::None, report_time::no);
    std::string error_lines;
    if (rjson::get_or(args, "format", "text") == "csv")
        error_lines = acmacs::chart::export_error_lines<acmacs::DataFormatterCSV>(*chart, rjson::get_or(args, "projection_no", 0UL));
    else
        error_lines = acmacs::chart::export_error_lines<acmacs::DataFormatterSpaceSeparated>(*chart, rjson::get_or(args, "projection_no", 0UL));
    const auto compressed = acmacs::file::gzip_compress(error_lines);
    ap_set_content_type(r, "application/octet-stream");
    r->content_encoding = "gzip";
    ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);

} // command_download_error_lines

// ----------------------------------------------------------------------

void command_sequences_of_chart(request_rec* r, const rjson::value& /*args*/)
{
    try {
        auto chart = acmacs::chart::import_from_file(std::string(r->filename), acmacs::chart::Verify::None, report_time::no);
        const auto data = seqdb::sequences_of_chart_for_ace_view_1(*chart);
        ap_set_content_type(r, "application/json");
        ap_rwrite(data.data(), static_cast<int>(data.size()), r);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: command_sequences_of_chart: " << err.what() << '\n';
    }

} // command_sequences_of_chart

// ----------------------------------------------------------------------

void command_download_sequences_of_chart_as_fasta(request_rec *r, const rjson::value& /*args*/)
{
    try {
        auto chart = acmacs::chart::import_from_file(std::string(r->filename), acmacs::chart::Verify::None, report_time::no);
        const auto fasta = seqdb::sequences_of_chart_as_fasta(*chart);
        const auto compressed = acmacs::file::gzip_compress(fasta);
        ap_set_content_type(r, "application/octet-stream");
        r->content_encoding = "gzip";
        ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: command_download_sequences_of_chart_as_fasta: " << err.what() << '\n';
    }

} // command_download_sequences_of_chart_as_fasta

// ----------------------------------------------------------------------

void command_download_layout_sequences_as_csv(request_rec *r, const rjson::value& args)
{
    try {
        auto chart = acmacs::chart::import_from_file(std::string(r->filename), acmacs::chart::Verify::None, report_time::no);
        const auto data = export_layout_sequences_into_csv(std::string{}, *chart, rjson::get_or(args, "projection_no", 0UL));
        const auto compressed = acmacs::file::gzip_compress(data);
        ap_set_content_type(r, "application/octet-stream");
        r->content_encoding = "gzip";
        ap_rwrite(compressed.data(), static_cast<int>(compressed.size()), r);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: command_download_layout_sequences_as_csv: " << err.what() << '\n';
    }

} // command_download_layout_sequences_as_csv

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
