#include <string>
#include <fstream>

#include "acmacs-base/fmt.hh"
#include "acmacs-base/argv.hh"
#include "acmacs-chart-2/factory-import.hh"

#include "setup-dbs.hh"
#include "settings.hh"
#include "select.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<bool> sera{*this, 's', "sera", desc{"select sera"}};
    option<bool> just_indexes{*this, "just-indexes", desc{"report just indexes, comma separated"}};
    option<bool> verbose{*this, 'v', "verbose"};
    option<size_t> projection{*this, "projection", dflt{0UL}};
    option<bool> help_select{*this, "help-select"};

    argument<str> chart{*this, arg_name{"chart-file"}, mandatory};
    argument<str> command{*this, arg_name{"command-in-json-format"}, mandatory};
};

static int do_select(const Options& opt);

int main(int argc, char* const argv[])
{
    int exit_code = 1;
    try {
        Options opt(argc, argv);
        if (opt.help_select)
            std::cerr << settings_help_select();
        else
            exit_code = do_select(opt);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
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
        const auto indices = SelectAntigens(opt.verbose).select(chart_select, selector);
        std::cout << string::join(",", indices) << '\n';
        if (!opt.just_indexes) {
            for (auto index : indices)
                std::cout << "AG " << std::setfill(' ') << std::setw(num_digits) << index << ' ' << chart->antigen(index)->full_name() << '\n';
        }
    }
    else {
        const auto num_digits = static_cast<int>(std::log10(chart->number_of_sera())) + 1;
        const auto indices = SelectSera(opt.verbose).select(chart_select, selector);
        if (!opt.just_indexes) {
            std::cout << string::join(",", indices) << '\n';
        }
        else {
            for (auto index : indices)
                std::cout << "SR " << std::setfill(' ') << std::setw(num_digits) << index << ' ' << chart->serum(index)->full_name() << '\n';
        }
    }
    return 0;

} // do_select

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
