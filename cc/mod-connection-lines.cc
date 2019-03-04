#include "acmacs-map-draw/mod-connection-lines.hh"
#include "acmacs-map-draw/select.hh"

// ----------------------------------------------------------------------

void ModConnectionLines::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    acmacs::chart::Indexes antigen_indexes, serum_indexes;
    if (const auto& select_antigens = args()["antigens"]; !select_antigens.is_null())
        antigen_indexes = SelectAntigens(false, 100).select(aChartDraw, select_antigens);
    else
        antigen_indexes = aChartDraw.chart().antigens()->all_indexes();
    if (const auto& select_sera = args()["sera"]; !select_sera.is_null())
        serum_indexes = SelectSera(false, 100).select(aChartDraw, select_sera);
    else
        serum_indexes = aChartDraw.chart().sera()->all_indexes();
    // std::cerr << "DEBUG: antigens: " << antigen_indexes << '\n';
    // std::cerr << "DEBUG: sera: " << serum_indexes << '\n';

    const Color line_color{rjson::get_or(args(), "color", "black")};
    const double line_width{rjson::get_or(args(), "width", 1.0)};

    auto layout = aChartDraw.layout();
    auto titers = aChartDraw.chart().titers();
    for (const auto ag_no : antigen_indexes) {
        for (const auto sr_no : serum_indexes) {
            if (const auto titer = titers->titer(ag_no, sr_no); !titer.is_dont_care()) {
                // std::cerr << "DEBUG: " << ag_no << ' ' << sr_no << ' ' << titer << '\n';
                if (const auto from = layout->get(ag_no), to = layout->get(sr_no + aChartDraw.number_of_antigens()); from.exists() && to.exists())
                    aChartDraw.line(from, to).color(line_color).line_width(line_width);
            }
        }
    }

} // ModConnectionLines::apply

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
