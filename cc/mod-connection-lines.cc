#include <map>
#include <array>
#include <cmath>

#include "acmacs-base/stream.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-base/string.hh"
#include "acmacs-map-draw/mod-connection-lines.hh"
#include "acmacs-map-draw/select.hh"

static std::pair<acmacs::chart::Indexes, acmacs::chart::Indexes> select_antigens_sera_for_connection_lines(ChartDraw& aChartDraw, const rjson::value& select_antigens, const rjson::value& select_sera);

// ----------------------------------------------------------------------

void ModConnectionLines::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto [antigen_indexes, serum_indexes] = select_antigens_sera_for_connection_lines(aChartDraw, args()["antigens"], args()["sera"]);

    const Color line_color{rjson::get_or(args(), "color", "black")};
    const double line_width{rjson::get_or(args(), "line_width", 1.0)};

    auto layout = aChartDraw.transformed_layout();
    auto titers = aChartDraw.chart().titers();
    for (const auto ag_no : antigen_indexes) {
        for (const auto sr_no : serum_indexes) {
            if (const auto titer = titers->titer(ag_no, sr_no); !titer.is_dont_care()) {
                // std::cerr << "DEBUG: " << ag_no << ' ' << sr_no << ' ' << titer << '\n';
                if (const auto from = layout->at(ag_no), to = layout->at(sr_no + aChartDraw.number_of_antigens()); from.exists() && to.exists())
                    aChartDraw.line(from, to).color(line_color).line_width(line_width);
            }
        }
    }

} // ModConnectionLines::apply

// ----------------------------------------------------------------------

void ModColorByNumberOfConnectionLines::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    // ['#ffffcc','#ffeda0','#fed976','#feb24c','#fd8d3c','#fc4e2a','#e31a1c','#bd0026','#800026']

    // const std::array<Color, 11> colors{0x313695,0x4575b4,0x74add1,0xabd9e9,0xe0f3f8,0xffffbf,0xfee090,0xfdae61,0xf46d43,0xd73027,0xa50026};
    // const std::array<Color, 9> colors{0xfff5f0,0xfee0d2,0xfcbba1,0xfc9272,0xfb6a4a,0xef3b2c,0xcb181d,0xa50f15,0x67000d};
    const std::array<Color, 9> colors{0x313695,0x74add1,0xfcbba1,0xfc9272,0xfb6a4a,0xef3b2c,0xcb181d,0xa50f15,0x67000d};
    const auto [antigen_indexes, serum_indexes] = select_antigens_sera_for_connection_lines(aChartDraw, args()["antigens"], args()["sera"]);

    std::map<size_t, size_t> antigens_titers, sera_titers;
    auto titers = aChartDraw.chart().titers();
    for (const auto ag_no : antigen_indexes) {
        for (const auto sr_no : serum_indexes) {
            if (const auto titer = titers->titer(ag_no, sr_no); !titer.is_dont_care()) {
                ++antigens_titers[ag_no];
                ++sera_titers[sr_no];
            }
        }
    }
    // std::cerr << "DEBUG: antigens: " << antigens_titers << '\n';
    // std::cerr << "DEBUG: sera: " << sera_titers << '\n';

    const auto max_antigen_titers = std::max_element(std::begin(antigens_titers), std::end(antigens_titers), [](const auto& e1, const auto& e2) { return e1.second < e2.second; })->second;
    const auto bin_size = static_cast<size_t>(std::ceil(double(max_antigen_titers + 1) / colors.size()));
    // std::cerr << "DEBUG: max_antigen_titers: " << max_antigen_titers << '\n';

    std::vector<acmacs::chart::Indexes> antigens_per_bin(colors.size());
    for (const auto& [ag_no, num_titers] : antigens_titers)
        antigens_per_bin.at(num_titers / bin_size).insert(ag_no);
    for (auto [color_no, antigens] : acmacs::enumerate(antigens_per_bin)) {
        acmacs::PointStyleModified style;
        // style.outline = BLACK;
        style.fill(acmacs::color::Modifier{colors.at(color_no)});
        style.outline(acmacs::color::Modifier{colors.at(color_no)});
        aChartDraw.modify(antigens, style, PointDrawingOrder::Raise);
        add_legend(aChartDraw, antigens, style, acmacs::string::concat(color_no * bin_size, "-", (color_no + 1) * bin_size - 1, " titrations"), rjson::object{});
    }

} // ModColorByNumberOfConnectionLines::apply

// ----------------------------------------------------------------------

void ModErrorLines::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto [antigen_indexes, serum_indexes] = select_antigens_sera_for_connection_lines(aChartDraw, args()["antigens"], args()["sera"]);

    const Color  line_color_td_more{rjson::get_or(args(), "color", "red")};
    const Color  line_color_td_less{rjson::get_or(args(), "color", "blue")};
    const double line_width{rjson::get_or(args(), "line_width", 1.0)};
    const bool   report{rjson::get_or(args(), "report", false)};
    const auto error_lines = aChartDraw.projection().error_lines();

    // if (report) {
    //     fmt::print("INFO: antigens for error lines: {}\nINFO: sera for error lines: {}\n", antigen_indexes, serum_indexes);
    //     for (const auto& errl : error_lines)
    //         fmt::print("INFO: error line {} {} : {}\n", errl.point_1, errl.point_2, errl.error_line);
    // }

    auto layout = aChartDraw.transformed_layout();
    auto titers = aChartDraw.chart().titers();
    auto antigens = aChartDraw.chart().antigens();
    auto sera = aChartDraw.chart().sera();
    const auto number_of_antigens = antigens->size();
    for (const auto ag_no : antigen_indexes) {
        for (const auto sr_no : serum_indexes) {
            const auto p2_no = sr_no + number_of_antigens;
            if (const auto found =
                    std::find_if(std::begin(error_lines), std::end(error_lines), [p1_no=ag_no,p2_no](const auto& erl) { return erl.point_1 == p1_no && erl.point_2 == p2_no; });
                found != std::end(error_lines)) {
                if (const auto p1 = layout->at(ag_no), p2 = layout->at(p2_no); p1.exists() && p2.exists()) {
                    if (report)
                        fmt::print("INFO: error line {} {} -- {} {} : {}\n", ag_no, antigens->at(ag_no)->full_name(), sr_no, sera->at(sr_no)->full_name(), found->error_line);
                    //aChartDraw.line(p1, p2).color(GREY).line_width(line_width * 3);
                    const auto v3 = (p2 - p1) / distance(p1, p2) * (- found->error_line) / 2.0;
                    const auto& color = found->error_line > 0 ? line_color_td_more : line_color_td_less;
                    aChartDraw.line(p1, p1 + v3).color(color).line_width(line_width);
                    aChartDraw.line(p2, p2 - v3).color(color).line_width(line_width);
                    // std::cerr << "DEBUG: " << found->point_1 << ' ' << sr_no << ' ' << found->error_line << ' ' << v3 << '\n';
                }
            }
        }
    }

} // ModErrorLines::apply

// ----------------------------------------------------------------------

std::pair<acmacs::chart::Indexes, acmacs::chart::Indexes> select_antigens_sera_for_connection_lines(ChartDraw& aChartDraw, const rjson::value& select_antigens, const rjson::value& select_sera)
{
    acmacs::chart::Indexes antigen_indexes, serum_indexes;
    if (!select_antigens.is_null())
        antigen_indexes = SelectAntigens(acmacs::verbose::no, 100).select(aChartDraw, select_antigens);
    else
        antigen_indexes = aChartDraw.chart().antigens()->all_indexes();
    if (!select_sera.is_null())
        serum_indexes = SelectSera(acmacs::verbose::no, 100).select(aChartDraw, select_sera);
    else
        serum_indexes = aChartDraw.chart().sera()->all_indexes();
    // std::cerr << "DEBUG: antigens: " << antigen_indexes << '\n';
    // std::cerr << "DEBUG: sera: " << serum_indexes << '\n';
    return {antigen_indexes, serum_indexes};

} // select_antigens_sera_for_connection_lines

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
