#include "acmacs-base/fmt.hh"
#include "acmacs-base/argv.hh"
#include "acmacs-chart-2/factory-import.hh"

#include "setup-dbs.hh"
#include "select.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<bool> sera{*this, 's', "sera", desc{"select sera"}};
    option<bool> just_indexes{*this, "just-indexes", desc{"report just indexes, comma separated"}};
    option<bool> verbose{*this, 'v', "verbose"};
    option<size_t> projection{*this, "projection", dflt{0UL}};

    argument<str> chart{*this, arg_name{"chart-file"}, mandatory};
    argument<str> command{*this, arg_name{"command-in-json-format"}, mandatory};
};

static int do_select(const Options& opt);

int main(int argc, char* const argv[])
{
    int exit_code = 1;
    try {
        Options opt(argc, argv);
        exit_code = do_select(opt);
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------

int do_select(const Options& opt)
{
    const auto selector = rjson::parse_string(opt.command);
    auto chart = acmacs::chart::import_from_file(opt.chart);
    if (chart->number_of_projections() < 1 || chart->number_of_projections() < *opt.projection)
        throw std::runtime_error(fmt::format("chart has too few projections: {}", chart->number_of_projections()));
    ChartSelectInterface chart_select(std::make_shared<acmacs::chart::ChartModify>(chart), opt.projection);
    if (!opt.sera) {
        const auto num_digits = static_cast<int>(std::log10(chart->number_of_antigens())) + 1;
        const auto indices = SelectAntigens(acmacs::verbose_from(opt.verbose)).select(chart_select, selector);
        std::cout << string::join(",", indices) << '\n';
        if (!opt.just_indexes) {
            for (auto index : indices)
                fmt::print("AG {:{}d} {}\n", index, num_digits, chart->antigen(index)->full_name());
        }
    }
    else {
        const auto num_digits = static_cast<int>(std::log10(chart->number_of_sera())) + 1;
        const auto indices = SelectSera(acmacs::verbose_from(opt.verbose)).select(chart_select, selector);
        if (!opt.just_indexes) {
            fmt::print("{}\n", string::join(",", indices));
        }
        else {
            for (auto index : indices)
                fmt::print("SR {:{}d} {}\n", index, num_digits, chart->serum(index)->full_name());
        }
    }
    return 0;

} // do_select

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
