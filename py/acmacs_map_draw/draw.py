# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import os, copy, operator, pprint
from pathlib import Path
import logging; module_logger = logging.getLogger(__name__)
from .hidb_access import get_hidb
from .locdb_access import get_locdb
from .vaccines import vaccines
from acmacs_map_draw_backend import ChartDraw, PointStyle, find_vaccines_in_chart, LocDb, distinct_colors

# ----------------------------------------------------------------------


# ----------------------------------------------------------------------

def draw_chart(output_file, chart, settings, output_width, verbose=False):
    chart_draw = ChartDraw(chart)
    chart_draw.prepare()
    # chart_draw.background_color("green")
    # chart_draw.grid("red", 1)
    # chart_draw.border("orange", 2)
    chart_draw.mark_egg_antigens()
    chart_draw.mark_reassortant_antigens()
    #chart_draw.scale_points(3)
    chart_draw.all_grey()
    # chart_draw.modify_point_by_index(0, PointStyle().fill("blue").outline("black").size(20))
    # chart_draw.modify_point_by_index(0, make_point_style({"fill": "blue", "outline": "black", "size": 20}))
    # chart_draw.modify_point_by_index(10, acmacs_chart.PointStyle(fill="red", outline="black", size=20))
    # chart_draw.rotate(1.57)
    # chart_draw.flip_ns()
    # chart_draw.flip_ew()
    # chart_draw.flip(-1, 1)                # flip about diagonal from [0,0] to [1,1], i.e. flip in direction [-1,1]

    # mark_continents(chart_draw=chart_draw, chart=chart)

    # mark_clades(chart_draw=chart_draw, chart=chart, legend_settings=settings["legend"], verbose=verbose)

    mark_aa_substitutions(chart_draw=chart_draw, chart=chart, positions=[158, 159], legend_settings=settings["legend"], verbose=verbose)

    if False:
        # labels
        locdb = get_locdb()

        indices = chart.antigens().find_by_name_matching("SW/9715293")
        for index in indices:
            ag = chart.antigen(index)
            module_logger.info('Label {:4d} {}'.format(index, f"{ag.name()} {ag.reassortant()} {ag.passage()} {ag.annotations() or ''} [{ag.date()}] {ag.lab_id()}"))
            chart_draw.label(index).display_name(ag.abbreviated_name_with_passage_type(locdb)) # .offset(0, -1) #.color("red").size(10).offset(0.01, 2.01)
        # chart_draw.label(1) #.color("red").size(10).offset(0.01, 2.01)

        indices = chart.sera().find_by_name_matching("SW/9715293 2015-027")
        for index in indices:
            sr = chart.serum(index)
            module_logger.info('Label {:4d} {}'.format(index, f"{sr.name()} {sr.reassortant()} {sr.annotations() or ''} [{sr.serum_id()}]"))
            chart_draw.label(index + chart.number_of_antigens()).display_name(sr.abbreviated_name(locdb)).color("orange") # .offset(0, -1) #.color("red").size(10).offset(0.01, 2.01)

    chart_draw.title().add_line(chart.make_name())
    # chart_draw.title().add_line("WHOA")

    mark_vaccines(chart_draw=chart_draw, chart=chart)
    chart_draw.draw(str(output_file), output_width)

# ----------------------------------------------------------------------

sStyleByVaccineType = {
    "previous": {
        "egg": {"fill": "blue", "outline": "black", "aspect": 0.75},
        "reassortant": {"fill": "blue", "outline": "black", "aspect": 0.75, "rotation": 0.5},
        "cell": {"fill": "blue", "outline": "black"}
        },
    "current": {
        "egg": {"fill": "red", "outline": "black", "aspect": 0.75},
        "reassortant": {"fill": "green", "outline": "black", "aspect": 0.75, "rotation": 0.5},
        "cell": {"fill": "red", "outline": "black"}
        },
    "surrogate": {
        "egg": {"fill": "pink", "outline": "black", "aspect": 0.75},
        "reassortant": {"fill": "pink", "outline": "black", "aspect": 0.75, "rotation": 0.5},
        "cell": {"fill": "pink", "outline": "black"}
        },
    }

def mark_vaccines(chart_draw, chart, style={"size": 15}, raise_=True):
    hidb = get_hidb(chart=chart)
    for vaccine_entry in vaccines(chart=chart):
        # module_logger.debug('{}'.format(vaccine_entry))
        antigens = find_vaccines_in_chart(vaccine_entry["name"], chart, hidb)
        for passage_type in ["egg", "reassortant", "cell"]:
            vaccine_data = getattr(antigens, passage_type)()
            if vaccine_data:
                vstyle = sStyleByVaccineType[vaccine_entry["type"]][passage_type] # copy.deepcopy
                if style:
                    vstyle.update(style)
                module_logger.info('Marking vaccine {} {}'.format(vaccine_data.antigen_index, vaccine_data.antigen.full_name()))
                chart_draw.modify_point_by_index(vaccine_data.antigen_index, make_point_style(vstyle), raise_=raise_)
                # chart_draw.label(vaccine_data.antigen_index).offset(-0.1, 1).color("red")

# ----------------------------------------------------------------------

sStyleByContinent = {
    "EUROPE":            {"fill": "green"},
    "CENTRAL-AMERICA":   {"fill": "#AAF9FF"},
    "MIDDLE-EAST":       {"fill": "#8000FF"},
    "NORTH-AMERICA":     {"fill": "blue4"},
    "AFRICA":            {"fill": "darkorange1"},
    "ASIA":              {"fill": "red"},
    "RUSSIA":            {"fill": "maroon"},
    "AUSTRALIA-OCEANIA": {"fill": "hotpink"},
    "SOUTH-AMERICA":     {"fill": "turquoise"},
    "ANTARCTICA":        {"fill": "grey50"},
    "":                  {"fill": "grey50"},
    }

def mark_continents(chart_draw, chart):
    from .locdb_access import get_locdb
    data = chart.antigens().continents(get_locdb())
    module_logger.info('[Continents] {}'.format(" ".join(f"{continent}:{len(data[continent])}" for continent in sorted(data))))
    global sStyleByContinent
    for continent, indices in data.items():
        chart_draw.modify_points_by_indices(indices, make_point_style(sStyleByContinent[continent]))
    chart_draw.continent_map([0, -50], 100)

# ----------------------------------------------------------------------

sStyleByClade = {
    # H3
    "3C3":   {"fill": "cornflowerblue", "outline": "black"},
    "3C2a":  {"fill": "red", "outline": "black"},
    "3C2a1": {"fill": "darkred", "outline": "black"},
    "3C3a":  {"fill": "green", "outline": "black"},
    "3C3b":  {"fill": "blue", "outline": "black"},
    # H1pdm
    "6B1": {"fill": "blue", "outline": "black"},
    "6B2": {"fill": "red", "outline": "black"},
    # B/Yam
    "Y2": {"fill": "cornflowerblue", "outline": "black"},
    "Y3": {"fill": "red", "outline": "black"},
    # B/Vic
    "1": {"fill": "blue", "outline": "black"},
    "1A": {"fill": "cornflowerblue", "outline": "black"},
    "1B": {"fill": "red", "outline": "black"},
    "": {"fill": "green", "outline": "black"},                # sequenced but not in any clade
    }

def mark_clades(chart_draw, chart, legend_settings, verbose=False):
    from .seqdb_access import antigen_clades
    clade_data = antigen_clades(chart, verbose=verbose)
    # pprint.pprint(clade_data)
    global sStyleByClade
    clades_used = {}                      # for legend
    for clade, indices in clade_data.items():
        style = sStyleByClade.get(clade)
        if style:
            chart_draw.modify_points_by_indices(indices, make_point_style(style), raise_=True)
            clades_used[clade] = [style, len(indices), indices if len(indices) < 10 else []]
    # pprint.pprint(clades_used)
    module_logger.info('Clades {}'.format(sorted(clades_used)))

    make_legend(chart_draw,
                    legend_data=sorted(({"label": clade if clade else "sequenced", **data[0]} for clade, data in clades_used.items()), key=operator.itemgetter("label")),
                    legend_settings=legend_settings)

# ----------------------------------------------------------------------

def mark_aa_substitutions(chart_draw, chart, positions, legend_settings, verbose=False):
    from . import seqdb_access

    aa_indices = seqdb_access.aa_at_positions(chart=chart, positions=positions, verbose=verbose)
    aa_order = sorted(aa_indices, key=lambda aa: - len(aa_indices[aa]))
    dc = distinct_colors()
    aa_color = {aa: dc[no] for no, aa in enumerate(aa_order)}
    if "X" in aa_color:
        aa_color["X"] = "grey25"
    # print(aa_color)
    for aa in aa_order:
        chart_draw.modify_points_by_indices(aa_indices[aa], make_point_style({"outline": "black", "fill": aa_color[aa]}), raise_=True)

    make_legend(chart_draw,
                    legend_data=[{"label": "{} {:3d}".format(aa, len(aa_indices[aa])), "outline": "black", "fill": aa_color[aa]} for aa in aa_order],
                    legend_settings=legend_settings)

# ----------------------------------------------------------------------

def make_legend(chart_draw, legend_data, legend_settings):
    # pprint.pprint(legend_data)
    if legend_settings and legend_settings.get("show", True) and legend_data:
        legend_box = chart_draw.legend(legend_settings.get("offset", [-10, -10]))
        for k in ["label_size", "background", "border_color", "border_width", "label_size", "point_size"]:
            if k in legend_settings:
                getattr(legend_box, k)(legend_settings[k])
        for legend_entry in legend_data:
            legend_box.add_line(**legend_entry)

# ----------------------------------------------------------------------

def make_point_style(data):
    ps = PointStyle()
    for k, v in data.items():
        setter = getattr(ps, k, None)
        if setter:
            setter(v)
    return ps

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
