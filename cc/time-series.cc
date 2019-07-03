#include <iostream>

#include "acmacs-base/time-series.hh"
#include "acmacs-map-draw/time-series.hh"

// ----------------------------------------------------------------------

void test_time_series()
{
    date::year_month_day frmstr{date::from_string("2017-03-06")};
    std::cout << "frmstr: " << frmstr << " ok?:" << frmstr.ok() << std::endl;
    std::cout << "frmstr 3 months before: " << date::decrement_month(frmstr, 3) << " ok?:" << frmstr.ok() << std::endl;
    date::year_month_day today = date::today();
    std::cout << "today: " << today << std::endl;
    std::cout << "today month3: " << date::month_3(today) << " year2: " << date::year_2(today) << " month_year: " << date::month3_year2(today) << std::endl;
    std::cout << "beginning_of_month: " << date::beginning_of_month(today) << std::endl;
    std::cout << "decrement_month: " << date::decrement_month(today) << std::endl;
    std::cout << "beginning of prev month: " << date::beginning_of_month(today) << std::endl;
    std::cout << "increment 2 month: " << date::increment_month(today, 2) << std::endl;

    date::year_month_day d1{date::beginning_of_month(date::today())}, d2{date::months_ago(date::beginning_of_month(d1), 7)};
    std::cout << "months between " << d1 << " and " << d2 << ": " << date::months_between_dates(d1, d2) << std::endl;
    std::cout << std::endl;

    // MonthlyTimeSeries mts0;
    // fmt::print("MonthlyTimeSeries0: {}\n", mts0);
    MonthlyTimeSeries mts1{date::months_ago(date::today(), 6), date::today()};
    fmt::print("MonthlyTimeSeries1: {}\n", mts1);
    for (auto it = mts1.begin(); it != mts1.end(); ++it)
        fmt::print(" {}\n", *it);

    YearlyTimeSeries yts1{date::years_ago(date::today(), 6), date::today()};
    fmt::print("YearlyTimeSeries1: {}\n", yts1);
    for (auto it = yts1.begin(); it != yts1.end(); ++it)
        fmt::print(" {}\n", *it);

    WeeklyTimeSeries wts1{date::weeks_ago(date::today(), 20), date::today()};
    fmt::print("WeeklyTimeSeries1: {}\n", wts1);
    for (auto it = wts1.begin(); it != wts1.end(); ++it)
        fmt::print(" {}\n", *it);

} // test_time_series

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
