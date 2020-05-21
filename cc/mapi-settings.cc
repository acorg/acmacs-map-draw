#include "acmacs-base/acmacsd.hh"
#include "acmacs-base/filesystem.hh"
#include "acmacs-map-draw/mapi-settings.hh"
// #include "acmacs-map-draw/draw.hh"

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
        return acmacs::settings::Settings::apply_built_in(name);
    }
    catch (std::exception& err) {
        throw error{fmt::format("cannot apply \"{}\": {}", name, err)};
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

void acmacs::mapi::v1::Settings::update_env()
{
    // const auto virus_type = tal_.tree().virus_type();
    // if (!virus_type.empty()) {  // might be updated later upon seqdb matching
    //     const auto lineage = tal_.tree().lineage();
    //     AD_LOG(acmacs::log::settings, "tree virus type: \"{}\" lineage: \"\"", virus_type, lineage);
    //     setenv_toplevel("virus-type", virus_type);
    //     setenv_toplevel("lineage", lineage);
    //     if (lineage.empty())
    //         setenv_toplevel("virus-type/lineage", virus_type);
    //     else
    //         setenv_toplevel("virus-type/lineage", fmt::format("{}/{}", virus_type, ::string::capitalize(lineage.substr(0, 3))));
    // }
    // setenv_toplevel("tree-has-sequences", tal_.tree().has_sequences());
    // setenv_toplevel("chart-present", tal_.chart_present());
    // if (tal_.chart_present())
    //     setenv_toplevel("chart-assay", tal_.chart().info()->assay().hi_or_neut());

} // acmacs::mapi::v1::Settings::update_env

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
