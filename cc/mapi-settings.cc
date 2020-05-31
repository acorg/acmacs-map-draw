#include "acmacs-base/acmacsd.hh"
#include "acmacs-base/filesystem.hh"
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
        // else if (name == "time-series"sv || name == "time_series"sv)
        //     return apply_time_series();
        // else if (name == ""sv)
        //     return apply_();
        return acmacs::settings::Settings::apply_built_in(name);
    }
    catch (std::exception& err) {
        throw error{fmt::format("cannot apply \"{}\": {} while reading {}", name, err, getenv_toplevel())};
    }

} // acmacs::mapi::v1::Settings::apply_built_in

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::load(const std::vector<std::string_view>& setting_files, const std::vector<std::string_view>& defines)
{
    using namespace std::string_view_literals;
    for (const auto& settings_file_name : {"clades.json"sv, "vaccines.json"sv, "mapi.json"sv}) {
        if (const auto filename = fmt::format("{}/share/conf/{}", acmacs::acmacsd_root(), settings_file_name); fs::exists(filename)) {
            AD_LOG(acmacs::log::settings, "loading {}", filename);
            load(filename);
        }
        else
            AD_WARNING("cannot load \"{}\": file not found", filename);
    }
    load(setting_files);
    for (const auto& def : defines) {
        if (const auto pos = def.find('='); pos != std::string_view::npos) {
            const auto val_s = def.substr(pos + 1);
            if (val_s == "-") { // parsed as -0
                setenv(def.substr(0, pos), rjson::v3::parse_string(fmt::format("\"{}\"", val_s)));
            }
            else {
                try {
                    setenv(def.substr(0, pos), rjson::v3::parse_string(val_s));
                }
                catch (std::exception&) {
                    setenv(def.substr(0, pos), rjson::v3::parse_string(fmt::format("\"{}\"", val_s)));
                }
            }
        }
        else
            setenv(def, "true"sv);
    }

} // acmacs::mapi::v1::Settings::load

// ----------------------------------------------------------------------

std::string acmacs::mapi::v1::Settings::substitute_chart_metadata(std::string_view pattern, const ChartAccess& chart_access) const
{
    using namespace std::string_view_literals;

    const auto& chart{chart_access.chart()};
    auto info{chart.info()};
    auto titers{chart.titers()};
    const auto& projection = chart_access.modified_projection();

    const std::string virus_type{info->virus_type()}, lineage{chart.lineage()};
    const auto assay{info->assay()};
    const auto subset{info->subset(chart::Info::Compute::Yes)};

    std::string virus_type_lineage;
    if (lineage.empty())
        virus_type_lineage = virus_type;
    else
        virus_type_lineage = fmt::format("{}/{}", virus_type, ::string::capitalize(lineage.substr(0, 3)));
    std::string virus_type_lineage_subset_short;
    if (virus_type == "A(H1N1)"sv) {
        if (subset == "2009pdm"sv)
            virus_type_lineage_subset_short = "h1pdm";
        else
            virus_type_lineage_subset_short = "h1";
    }
    else if (virus_type == "A(H3N2)"sv)
        virus_type_lineage_subset_short = "h3";
    else if (virus_type == "B"sv)
        virus_type_lineage_subset_short = fmt::format("b{}", ::string::lower(lineage.substr(0, 3)));
    else
        virus_type_lineage_subset_short = ::string::lower(virus_type);

    try {
        return fmt::format(pattern,
                           fmt::arg("virus", info->virus()),
                           fmt::arg("virus_type", virus_type),
                           fmt::arg("virus_type_lineage", virus_type_lineage),
                           fmt::arg("lineage", lineage),
                           fmt::arg("subset", subset),
                           fmt::arg("virus_type_lineage_subset_short", virus_type_lineage_subset_short),
                           fmt::arg("assay", assay.hi_or_neut()),
                           fmt::arg("assay_full", assay),
                           fmt::arg("lab", info->lab()),
                           fmt::arg("lab_lower", ::string::lower(info->lab())),
                           fmt::arg("rbc", info->rbc_species()),
                           fmt::arg("table_date", info->date(chart::Info::Compute::Yes)),
                           fmt::arg("number_of_antigens", chart.number_of_antigens()),
                           fmt::arg("number_of_sera", chart.number_of_sera()),
                           fmt::arg("number_of_layers", titers->number_of_layers()),
                           fmt::arg("minimum_column_basis", static_cast<std::string>(projection.minimum_column_basis())),
                           fmt::arg("stress", projection.stress()),
                           fmt::arg("name", chart.make_name())
                           );
    }
    catch (std::exception& err) {
        AD_ERROR("fmt cannot substitute in \"{}\": {}", pattern, err);
        throw;
    }

} // acmacs::mapi::v1::Settings::substitute_chart_metadata

// ----------------------------------------------------------------------

constexpr static const char* sUpdateEnvPattern = R"({{
"init": [
{{"N": "set",
"virus": "{virus}",
"virus-type": "{virus_type}",
"virus-type/lineage": "{virus_type_lineage}",
"lineage": "{lineage}",
"subset": "{subset}",
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
    const auto json{substitute_chart_metadata(sUpdateEnvPattern, chart_draw().chart(0))};
    // AD_DEBUG("update_env\n{}", json);
    load_from_string(json);

} // acmacs::mapi::v1::Settings::update_env

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
