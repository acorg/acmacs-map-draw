#include "acmacs-base/time-series.hh"
#include "acmacs-map-draw/time-series.hh"

// ----------------------------------------------------------------------

void test_time_series()
{
    Date empty;
    std::cout << "default: " << empty << " empty?:" << empty.empty() << std::endl;
    Date frmstr("2017-03-06");
    std::cout << "frmstr: " << frmstr << " empty?:" << frmstr.empty() << std::endl;
    std::cout << "frmstr 3 months before: " << frmstr.decrement_month(3) << " empty?:" << frmstr.empty() << std::endl;
    Date today(Date::Today);
    std::cout << "today: " << today << std::endl;
    std::cout << "today month3: " << today.month_3() << " year2: " << today.year_2() << " month_year: " << today.month3_year2() << std::endl;
    std::cout << "beginning_of_month: " << today.beginning_of_month() << std::endl;
    std::cout << "decrement_month: " << today.decrement_month() << std::endl;
    std::cout << "beginning of prev month: " << today.beginning_of_month() << std::endl;
    std::cout << "increment 2 month: " << today.increment_month(2) << std::endl;

    Date d1{Date(Date::Today).beginning_of_month()}, d2{d1.beginning_of_month().decrement_month(7)};
    std::cout << "months between " << d1 << " and " << d2 << ": " << months_between_dates(d1, d2) << std::endl;
    std::cout << std::endl;

    MonthlyTimeSeries mts0;
    std::cout << "MonthlyTimeSeries0: " << mts0 << std::endl;
    MonthlyTimeSeries mts1{Date::months_ago(6), Date::today()};
    std::cout << "MonthlyTimeSeries1: " << mts1 << std::endl;
    for (auto it = mts1.begin(); it != mts1.end(); ++it)
        std::cout << "  " << *it << std::endl;

    YearlyTimeSeries yts1{Date::years_ago(6), Date::today()};
    std::cout << "YearlyTimeSeries1: " << yts1 << std::endl;
    for (auto it = yts1.begin(); it != yts1.end(); ++it)
        std::cout << "  " << *it << std::endl;

} // test_time_series

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
