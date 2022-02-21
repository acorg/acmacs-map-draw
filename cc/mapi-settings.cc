#include "acmacs-base/rjson-v3.hh"
#include "acmacs-base/timeit.hh"
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
        else if (name == "remove-procrustes-arrows"sv || name == "remove_procrustes_arrows"sv)
            return apply_remove_procrustes();
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
        return acmacs::settings::v3::Data::apply_built_in(name);
    }
    catch (std::exception& err) {
        throw error{fmt::format("cannot apply mapi built in \"{}\": {}", name, err)};
    }

} // acmacs::mapi::v1::Settings::apply_built_in

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::update_env(env_put_antigen_serum_names epasn)
{
    using namespace std::string_view_literals;
    const auto& chart = chart_draw().chart(0).chart();
    setenv("name"sv, chart.make_name());

    auto info{chart.info()};
    setenv("virus"sv, info->virus());

    const std::string virus_type{info->virus_type()};
    setenv("virus-type"sv, virus_type);

    const std::string lineage{chart.lineage()};
    const auto subset{info->subset(acmacs::chart::Info::Compute::Yes)};
    setenv("lineage"sv, lineage);
    setenv("lineage-cap"sv, ::string::capitalize(lineage));
    setenv("subset"sv, subset);
    setenv("subset-up"sv, ::string::upper(subset));
    if (virus_type == "A(H1N1)"sv) {
        setenv("virus-type/lineage"sv, virus_type);
        setenv("virus-type/lineage-subset"sv, virus_type + subset);
        if (subset == "2009pdm"sv)
            setenv("virus-type-lineage-subset-short-low"sv, "h1pdm"sv);
        else
            setenv("virus-type-lineage-subset-short-low"sv, "h1"sv);
    }
    else if (virus_type == "A(H3N2)"sv) {
        setenv("virus-type/lineage"sv, virus_type);
        setenv("virus-type/lineage-subset"sv, virus_type);
        setenv("virus-type-lineage-subset-short-low"sv, "h3"sv);
    }
    else if (virus_type == "B"sv) {
        const auto vtl = fmt::format("{}/{}", virus_type, ::string::capitalize(lineage.substr(0, 3)));
        setenv("virus-type/lineage"sv, vtl);
        setenv("virus-type/lineage-subset"sv, vtl);
        setenv("virus-type-lineage-subset-short-low"sv, fmt::format("b{}", ::string::lower(lineage.substr(0, 3))));
    }
    else {
        std::string vtl{virus_type};
        if (!lineage.empty())
            vtl = fmt::format("{}/{}", virus_type, ::string::capitalize(lineage.substr(0, 3)));
        setenv("virus-type/lineage"sv, vtl);
        setenv("virus-type/lineage-subset"sv, fmt::format("{}{}", vtl, subset));
        setenv("virus-type-lineage-subset-short-low"sv, ::string::lower(virus_type));
    }

    const auto assay{info->assay()};
    setenv("assay-full"sv, *assay);
    const std::string assay_cap{assay.HI_or_Neut()};
    setenv("assay-cap"sv, assay_cap);
    const std::string assay_neut{assay.hi_or_neut()};
    setenv("assay-low"sv, assay_neut);
    if (assay_neut == "hi" && virus_type != "A(H3N2)") {
        setenv("assay-no-hi-low"sv, ""sv);
        setenv("assay-no-hi-cap"sv, ""sv);
    }
    else {
        setenv("assay-no-hi-low"sv, assay_neut);
        setenv("assay-no-hi-cap"sv, assay_cap);
    }

    auto lab{info->lab()};
    if (lab == "MELB"sv || lab == "MELB+VIDRL"sv)
        lab = Lab{"VIDRL"};
    setenv("lab"sv, lab);
    setenv("lab-low"sv, ::string::lower(lab));

    setenv("rbc"sv, info->rbc_species());
    if (assay_neut == "hi" && !info->rbc_species().empty())
        setenv("assay-rbc"sv, fmt::format("{}-{}", assay_neut, ::string::lower(info->rbc_species())));
    else
        setenv("assay-rbc"sv, assay_neut);

    setenv("assay-low"sv, assay_neut);

    setenv("table-date"sv, info->date(acmacs::chart::Info::Compute::Yes));

    setenv("num-ag"sv, fmt::format("{}", chart.number_of_antigens()));
    setenv("num-sr"sv, fmt::format("{}", chart.number_of_sera()));

    auto titers{chart.titers()};
    setenv("num-layers"sv, fmt::format("{}", titers->number_of_layers()));

    const auto& projection = chart_draw().chart(0).modified_projection();
    const auto minimum_column_basis = static_cast<std::string>(projection.minimum_column_basis());
    setenv("minimum-column-basis"sv, minimum_column_basis);
    setenv("mcb"sv, minimum_column_basis);

    if (epasn == env_put_antigen_serum_names::yes) {
        auto antigens = chart.antigens();
        for (auto [no, antigen] : acmacs::enumerate(*antigens)) {
            setenv(fmt::format("ag-{}-full-name", no), antigen->format("{name_full}"));
            setenv(fmt::format("ag-{}-full-name-without-passage", no), antigen->format("{name_full}"));
            setenv(fmt::format("ag-{}-full-name-with-passage", no), antigen->format("{name_full}"));
            setenv(fmt::format("ag-{}-abbreviated-name", no), antigen->format("{name_abbreviated}"));
            setenv(fmt::format("ag-{}-designation", no), antigen->format("{designation}"));
            setenv(fmt::format("ag-{}-clades", no), antigen->format("{clades}"));
            setenv(fmt::format("ag-{}-clade", no), antigen->format("{clade}")); // longest clade name
        }

        auto sera = chart.sera();
        for (auto [no, serum] : acmacs::enumerate(*sera)) {
            setenv(fmt::format("sr-{}-full-name", no), serum->format("{name_full}"));
            setenv(fmt::format("sr-{}-full-name-without-passage", no), serum->format("{name_full}"));
            setenv(fmt::format("sr-{}-full-name-with-passage", no), serum->format("{name_full_passage}"));
            setenv(fmt::format("sr-{}-abbreviated-name", no), serum->format("{name_abbreviated}"));
            setenv(fmt::format("sr-{}-abbreviated-name-with-serum-id", no), serum->format("{name_full}{ }{serum_id}", acmacs::chart::collapse_spaces_t::yes));
            setenv(fmt::format("sr-{}-designation", no), serum->format("{designation}"));
            setenv(fmt::format("sr-{}-designation-without-serum-id", no), serum->format("{designation_without_serum_id}"));
            setenv(fmt::format("sr-{}-clades", no), serum->format("{clades}"));
            setenv(fmt::format("sr-{}-clade", no), serum->format("{clade}")); // longest clade name
        }
    }

    update_env_upon_projection_change();

} // acmacs::mapi::v1::Settings::update_env

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::update_env_upon_projection_change()
{
    using namespace std::string_view_literals;
    const auto& projection = chart_draw().chart(0).modified_projection();

    setenv("stress"sv, fmt::format("{:.4f}", projection.stress()), settings::v3::replace::yes);
    setenv("stress-full"sv, fmt::format("{}", projection.stress()), settings::v3::replace::yes);

    // AD_DEBUG("env stress: {}", getenv("stress"sv));

} // acmacs::mapi::v1::Settings::update_env_upon_projection_change

// ----------------------------------------------------------------------
