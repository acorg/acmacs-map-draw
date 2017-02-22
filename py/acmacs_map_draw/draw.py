# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import os, copy, math, operator, pprint
from pathlib import Path
import logging; module_logger = logging.getLogger(__name__)
from .hidb_access import get_hidb
from .locdb_access import get_locdb
from .vaccines import vaccines
from acmacs_map_draw_backend import ChartDraw, PointStyle, find_vaccines_in_chart, LocDb, distinct_colors

# ----------------------------------------------------------------------

class UnrecognizedMod (ValueError): pass

# ----------------------------------------------------------------------

def draw_chart(output_file, chart, settings, output_width, verbose=False):
    chart_draw = ChartDraw(chart)
    chart_draw.prepare()

    applicator = ModApplicator(chart_draw=chart_draw, chart=chart, verbose=verbose)
    for mod in settings.get("mods", []):
        if isinstance(mod, str):
            try:
                getattr(applicator, mod)()
            except:
                raise UnrecognizedMod(mod)
        elif isinstance(mod, dict):
            try:
                getattr(applicator, mod["N"])(**mod)
            except:
                raise UnrecognizedMod(mod)
        else:
            raise UnrecognizedMod(mod)

    if False: # mods
        mark_clades(chart_draw=chart_draw, chart=chart, legend_settings=settings["legend"], verbose=verbose)
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

    chart_draw.draw(str(output_file), output_width)

# ----------------------------------------------------------------------

class ModApplicator:

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

    sStyleByClade = [                     # bottom raised entries above top entries
        {"N": "", "fill": "grey50", "outline": "black"},                # sequenced but not in any clade
        # H3
        {"N": "3C3",   "fill": "cornflowerblue", "outline": "black"},
        {"N": "3C2a",  "fill": "red", "outline": "black"},
        {"N": "3C2a1", "fill": "darkred", "outline": "black"},
        {"N": "3C3a",  "fill": "green", "outline": "black"},
        {"N": "3C3b",  "fill": "blue", "outline": "black"},
        # H1pdm
        {"N": "6B1", "fill": "blue", "outline": "black"},
        {"N": "6B2", "fill": "red", "outline": "black"},
        # B/Yam
        {"N": "Y2", "fill": "cornflowerblue", "outline": "black"},
        {"N": "Y3", "fill": "red", "outline": "black"},
        # B/Vic
        {"N": "1",  "fill": "blue", "outline": "black"},
        {"N": "1A", "fill": "cornflowerblue", "outline": "black"},
        {"N": "1B", "fill": "red", "outline": "black"},
        ]

    # ----------------------------------------------------------------------

    def __init__(self, chart_draw, chart, verbose=False):
        self._chart_draw = chart_draw
        self._chart = chart
        self._verbose = verbose

    def flip_ns(self, **args):
        self._chart_draw.flip_ns()

    def flip_ew(self, **args):
        self._chart_draw.flip_ew()

    def flip(self, value, **args):
        if isinstance(value, list) and len(value) == 2:
            self._chart_draw.flip(*value)
        else:
            raise ValueError()

    def rotate_degrees(self, angle, **args):
        self._chart_draw.rotate(angle * math.pi / 180.0)

    def rotate_radians(self, angle, **args):
        self._chart_draw.rotate(angle)

    def viewport(self, value, **args):
        if isinstance(value, list) and len(value) == 3:
            self._chart_draw.viewport(value)
        else:
            raise ValueError()

    def egg(self, **args):
        self._chart_draw.mark_egg_antigens()

    def reassortant(self, **args):
        self._chart_draw.mark_reassortant_antigens()

    def all_grey(self, **args):
        self._chart_draw.all_grey()

    def background(self, color, **args):
        self._chart_draw.background_color(color=color)

    def grid(self, color="grey80", line_width=1, **args):
        self._chart_draw.grid(color=color, line_width=line_width)

    def border(self, color="grey80", line_width=1, **args):
        self._chart_draw.border(color=color, line_width=line_width)

    def point_scale(self, scale=1, outline_scale=1, **args):
        self._chart_draw.scale_points(scale=scale, outline_scale=outline_scale)

    def style(self, index=None, indices=None, raise_=None, lower=None, **args):
        # fill=None, outline=None, show=None, shape=None, size=None, outline_width=None, aspect=None, rotation=None
        if index is None:
            if indices is None:
                raise ValueError("One of index or indices must be provided")
            index = indices
        elif indices is not None:
            raise ValueError("Either index or indices must be provided")
        if isinstance(index, list):
            func = self._chart_draw.modify_points_by_indices
        else:
            func = self._chart_draw.modify_point_by_index
        func(index, style=self._make_point_style(args), raise_=bool(raise_), lower=bool(lower))

    def continents(self, legend=None, **args):
        data = self._chart.antigens().continents(get_locdb())
        module_logger.info('[Continents] {}'.format(" ".join(f"{continent}:{len(data[continent])}" for continent in sorted(data))))
        for continent, indices in data.items():
            self._chart_draw.modify_points_by_indices(indices, self._make_point_style(self.sStyleByContinent[continent]))
        if legend and legend.get("show", True):
            self._chart_draw.continent_map(legend.get("offset", [0, 0]), legend.get("size", 100))

    def clades(self, legend=None, **args):
        from .seqdb_access import antigen_clades
        clade_data = antigen_clades(self._chart, verbose=self._verbose)
        # pprint.pprint(clade_data)
        clades_used = {}                      # for legend
        for clade_style in self.sStyleByClade:
            indices = clade_data.get(clade_style["N"])
            if indices:
                self._chart_draw.modify_points_by_indices(indices, self._make_point_style(clade_style), raise_=True)
                clades_used[clade_style["N"]] = [clade_style, len(indices)]
        module_logger.info('Clades {}'.format(clades_used))
        # module_logger.info('Clades {}'.format(sorted(clades_used)))
        if legend and legend.get("show", True):
            def make_label(clade, count):
                label = clade or "sequenced"
                if legend.get("point_count"):
                    label += " ({})".format(count)
                return label
            self._make_legend(
                legend_data=sorted(({"label": make_label(clade, data[1]), **data[0]} for clade, data in clades_used.items()), key=operator.itemgetter("label")),
                legend_settings=legend)

    def vaccines(self, raise_=True, label=None, **args):
        # fill=None, outline=None, show=None, shape=None, size=None, outline_width=None, aspect=None, rotation=None
        hidb = get_hidb(chart=self._chart)
        for vaccine_entry in vaccines(chart=self._chart):
            # module_logger.debug('{}'.format(vaccine_entry))
            antigens = find_vaccines_in_chart(vaccine_entry["name"], self._chart, hidb)
            for passage_type in ["egg", "reassortant", "cell"]:
                vaccine_data = getattr(antigens, passage_type)()
                if vaccine_data:
                    module_logger.info('Marking vaccine {} {}'.format(vaccine_data.antigen_index, vaccine_data.antigen.full_name()))
                    self._chart_draw.modify_point_by_index(vaccine_data.antigen_index, self._make_point_style({**self.sStyleByVaccineType[vaccine_entry["type"]][passage_type], **args}), raise_=raise_)
                    label_key = vaccine_entry["type"] + "-" + passage_type
                    if label:
                        if label.get(label_key):
                            self.label(index=vaccine_data.antigen_index, **{**label.get("", {}), **label[label_key]})
                        elif label.get(""):
                            self.label(index=vaccine_data.antigen_index, **label[""])

    def label(self, index, name_type="full", **args):
        lbl = self._chart_draw.label(index)
        if "display_name" not in args:
            if index < self._chart.number_of_antigens():
                if name_type == "abbreviated":
                    args["display_name"] = self._chart.antigen(index).abbreviated_name(get_locdb())
                elif name_type == "abbreviated_with_passage_type":
                    args["display_name"] = self._chart.antigen(index).abbreviated_name_with_passage_type(get_locdb())
                else:
                    args["display_name"] = self._chart.antigen(index).full_name(get_locdb())
            else:
                if name_type in ["abbreviated", "abbreviated_with_passage_type"]:
                    args["display_name"] = self._chart.serum(index - self._chart.number_of_antigens()).abbreviated_name(get_locdb())
                else:
                    args["display_name"] = self._chart.serum(index - self._chart.number_of_antigens()).full_name(get_locdb())
        for k, v in args.items():
            setter = getattr(lbl, k, None)
            if setter:
                setter(v)

    def _make_point_style(self, data):
        style = PointStyle()
        for k, v in data.items():
            setter = getattr(style, k, None)
            if setter:
                setter(v)
        return style

    def _make_legend(self, legend_data, legend_settings):
        # pprint.pprint(legend_data)
        if legend_settings and legend_settings.get("show", True) and legend_data:
            legend_box = self._chart_draw.legend(legend_settings.get("offset", [-10, -10]))
            for k in ["label_size", "background", "border_color", "border_width", "label_size", "point_size"]:
                if k in legend_settings:
                    getattr(legend_box, k)(legend_settings[k])
            for legend_entry in legend_data:
                legend_box.add_line(**{k: v for k, v in legend_entry.items() if k not in ["N"]})

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

# def make_point_style(data):
#     ps = PointStyle()
#     for k, v in data.items():
#         setter = getattr(ps, k, None)
#         if setter:
#             setter(v)
#     return ps

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
