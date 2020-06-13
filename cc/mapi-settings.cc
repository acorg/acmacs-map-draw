#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_built_in(std::string_view name) // returns true if built-in command with that name found and applied
{
    using namespace std::string_view_literals;
    AD_LOG(acmacs::log::settings, "mapi::apply_built_in \"{}\"", name);
    AD_LOG_INDENT;
    try {
        // printenv();
        if (name == "antigens"sv)
            return apply_antigens();
        else if (name == "sera"sv)
            return apply_sera();
        else if (name == "vaccine"sv)
            return apply_vaccine();
        else if (name == "circle"sv)
            return apply_circle();
        else if (name == "path"sv)
            return apply_path();
        else if (name == "rotate"sv)
            return apply_rotate();
        else if (name == "flip"sv)
            return apply_flip();
        else if (name == "viewport"sv)
            return apply_viewport();
        else if (name == "background"sv)
            return apply_background();
        else if (name == "border"sv)
            return apply_border();
        else if (name == "grid"sv)
            return apply_grid();
        else if (name == "point-scale"sv || name == "point_scale"sv)
            return apply_point_scale();
        else if (name == "connection_lines"sv || name == "connection-lines"sv)
            return apply_connection_lines();
        else if (name == "error_lines"sv || name == "error-lines"sv)
            return apply_error_lines();
        else if (name == "legend"sv)
            return apply_legend();
        else if (name == "title"sv)
            return apply_title();
        else if (name == "serum-circles"sv || name == "serum-circle"sv || name == "serum_circles"sv || name == "serum_circle"sv)
            return apply_serum_circles();
        else if (name == "serum-coverage"sv || name == "serum_coverage"sv)
            return apply_serum_coverage();
        else if (name == "procrustes-arrows"sv || name == "procrustes_arrows"sv)
            return apply_procrustes();
        else if (name == "move"sv)
            return apply_move();
        else if (name == "reset"sv)
            return apply_reset();
        else if (name == "export"sv)
            return apply_export();
        else if (name == "pdf"sv)
            return apply_pdf();
        else if (name == "relax"sv)
            return apply_relax();
        else if (name == "compare-sequences"sv || name == "compare_sequences"sv)
            return apply_compare_sequences();
        else if (name == "time-series"sv || name == "time_series"sv)
            return apply_time_series();
        // else if (name == ""sv)
        //     return apply_();
        return acmacs::settings::Settings::apply_built_in(name);
    }
    catch (std::exception& err) {
        throw error{fmt::format("cannot apply \"{}\": {} while reading {}", name, err, getenv_toplevel())};
    }

} // acmacs::mapi::v1::Settings::apply_built_in

// ----------------------------------------------------------------------

constexpr static const char* sUpdateEnvPattern = R"({{
"init": [
{{"N": "set",
"virus": "{virus}",
"virus-type": "{virus_type}",
"virus-type/lineage": "{virus_type_lineage}",
"lineage": "{lineage}",
"subset": "{subset}",
"SUBSET": "{SUBSET}",
"assay": "{assay}",
"assay-full": "{assay_full}",
"lab": "{lab}",
"rbc": "{rbc}",
"table-date": "{table_date}",
"number-of-antigens": {number_of_antigens},
"number-of-sera": {number_of_sera},
"number-of-layers": {number_of_layers}
}}
]}})";

void acmacs::mapi::v1::Settings::update_env()
{
    const auto json{chart_draw().chart(0).substitute_metadata(sUpdateEnvPattern)};
    // AD_DEBUG("update_env\n{}", json);
    load_from_string(json);

} // acmacs::mapi::v1::Settings::update_env

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
