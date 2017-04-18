# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

# sVaccines = {
#     "A(H1N1)": [
#         {"name": "CALIFORNIA/7/2009", "type": "previous"},
#         {"name": "MICHIGAN/45/2015", "type": "current"},
#         ],
#     "A(H3N2)": [
#         {"name": "BRISBANE/10/2007",         "type": "previous"},
#         {"name": "PERTH/16/2009",            "type": "previous"},
#         {"name": "VICTORIA/361/2011",        "type": "previous"},
#         {"name": "TEXAS/50/2012",            "type": "previous"},
#         {"name": "SWITZERLAND/9715293/2013", "type": "previous"},
#         {"name": "HONG KONG/4801/2014",      "type": "current"},
#         {"name": "SAITAMA/103/2014",         "type": "surrogate"},
#         {"name": "HONG KONG/7295/2014",      "type": "surrogate"},
#         ],
#     "B/VICTORIA": [
#         {"name": "MALAYSIA/2506/2004",      "type": "previous"},
#         {"name": "BRISBANE/60/2008",        "type": "current"},
#         {"name": "PARIS/1762/2009",         "type": "current"},
#         {"name": "SOUTH AUSTRALIA/81/2012", "type": "surrogate"},
#         ],
#     "B/YAMAGATA": [
#         {"name": "FLORIDA/4/2006",       "type": "previous"},
#         {"name": "WISCONSIN/1/2010",     "type": "previous"},
#         {"name": "MASSACHUSETTS/2/2012", "type": "previous"},
#         {"name": "PHUKET/3073/2013",     "type": "current"},
#         ],
#     }

# ----------------------------------------------------------------------

def vaccines(virus_type=None, lineage=None, chart=None):
    from acmacs_chart_backend import vaccines
    if chart is not None:
        vv = vaccines(chart=chart)
    else:
        vv = vaccines(subtype=virus_type, lineage=lineage or "")
    return [{"name": v.name, "type": v.type} for v in vv]

    # if chart is not None:
    #     virus_type = chart.chart_info().virus_type()
    #     lineage = chart.lineage()
    # if lineage:
    #     virus_type += "/" + lineage
    # try:
    #     return sVaccines[virus_type]
    # except:
    #     raise RuntimeError("Unrecognized virus type for getting vaccines: {!r}\nAvailable virus types: {}".format(virus_type, " ".join(sorted(sVaccines))))

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
