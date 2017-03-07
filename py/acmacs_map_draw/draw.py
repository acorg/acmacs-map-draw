# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import os, copy, math, operator, datetime, pprint
from pathlib import Path
import logging; module_logger = logging.getLogger(__name__)
from .hidb_access import get_hidb
from .locdb_access import get_locdb
from .vaccines import vaccines
from acmacs_map_draw_backend import ChartDraw, PointStyle, find_vaccines_in_chart, LocDb, distinct_colors

# ----------------------------------------------------------------------

class UnrecognizedMod (ValueError): pass

# ----------------------------------------------------------------------

def draw_chart(output_file, chart, settings, output_width, draw_map=True, verbose=False):
    chart_draw = ChartDraw(chart)
    chart_draw.prepare()

    applicator = ModApplicator(chart_draw=chart_draw, chart=chart, projection_no=0, verbose=verbose)
    for mod in settings.get("mods", []):
        if isinstance(mod, str):
            try:
                getattr(applicator, mod)(N=mod)
            except:
                raise UnrecognizedMod(mod)
        elif isinstance(mod, dict):
            if "N" in mod and mod["N"] and mod["N"][0] != "?":      # no "N" - commented out
                try:
                    getattr(applicator, mod["N"])(**mod)
                except:
                    raise UnrecognizedMod(mod)
        else:
            raise UnrecognizedMod(mod)
    if draw_map:
        chart_draw.draw(str(output_file), output_width)
    return chart_draw

    # if False:
    #     # labels
    #     locdb = get_locdb()

    #     indices = chart.antigens().find_by_name_matching("SW/9715293")
    #     for index in indices:
    #         ag = chart.antigen(index)
    #         module_logger.info('Label {:4d} {}'.format(index, f"{ag.name()} {ag.reassortant()} {ag.passage()} {ag.annotations() or ''} [{ag.date()}] {ag.lab_id()}"))
    #         chart_draw.label(index).display_name(ag.abbreviated_name_with_passage_type(locdb)) # .offset(0, -1) #.color("red").size(10).offset(0.01, 2.01)
    #     # chart_draw.label(1) #.color("red").size(10).offset(0.01, 2.01)

    #     indices = chart.sera().find_by_name_matching("SW/9715293 2015-027")
    #     for index in indices:
    #         sr = chart.serum(index)
    #         module_logger.info('Label {:4d} {}'.format(index, f"{sr.name()} {sr.reassortant()} {sr.annotations() or ''} [{sr.serum_id()}]"))
    #         chart_draw.label(index + chart.number_of_antigens()).display_name(sr.abbreviated_name(locdb)).color("orange") # .offset(0, -1) #.color("red").size(10).offset(0.01, 2.01)


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
        # {"N": "gly", "fill": "grey50", "outline": "black"},
        # {"N": "no-gly", "fill": "grey50", "outline": "black"},
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

    def __init__(self, chart_draw, chart, projection_no, verbose=False):
        self._chart_draw = chart_draw
        self._chart = chart
        self._projection_no = projection_no
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

    def antigens(self, N, select, report_names_threshold=10, **args):
        indices = self._select_antigens(select=select, raise_if_not_found=True, raise_if_multiple=False)
        antigens = self._chart.antigens()
        if report_names_threshold is None or (indices and len(indices) <= report_names_threshold):
            names = ["{:4d} {} [{}]".format(index, antigens[index].full_name(), antigens[index].date()) for index in indices]
            module_logger.info('Antigens {}: select:{!r} {}\n    {}'.format(len(indices), select, args, "\n    ".join(names)))
        elif indices and len(indices) < 20:
            module_logger.info('Antigens {}:{}: select:{!r} {}'.format(len(indices), indices, select, args))
        else:
            module_logger.info('Antigens {}: select:{!r} {}'.format(len(indices), select, args))
        self.style(index=indices, **args)
        if "label" in args and args["label"].get("show", True):
            for index in indices:
                self.label(index=index, **args["label"])

    def sera(self, N, select, report_names_threshold=10, **args):
        indices = self._select_sera(select=select, raise_if_not_found=True, raise_if_multiple=False)
        sera = self._chart.sera()
        if report_names_threshold is None or len(indices) <= report_names_threshold:
            names = ["{:3d} {}".format(index, sera[index].full_name()) for index in indices]
            module_logger.info('Sera {}: select:{!r} {}\n    {}'.format(len(indices), select, args, "\n    ".join(names)))
        elif len(indices) < 20:
            module_logger.info('Sera {}:{}: select:{!r} {}'.format(len(indices), indices, select, args))
        else:
            module_logger.info('Sera {}: select:{!r} {}'.format(len(indices), select, args))
        number_of_antigens = self._chart.number_of_antigens()
        self.style(index=[index + number_of_antigens for index in indices], **args)
        if "label" in args and args["label"].get("show", True):
            for index in indices:
                self.label(index=index + number_of_antigens, **args["label"])

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
            clade_style_mod = args.get(clade_style["N"], {"show": True})
            if clade_style_mod.get("show", True):
                indices = clade_data.get(clade_style["N"])
                lower = clade_style_mod.get("lower", False)
                raise_ = clade_style_mod.get("raise_", True) if not lower else False
                if indices:
                    self._chart_draw.modify_points_by_indices(indices, self._make_point_style(clade_style, clade_style_mod), raise_=raise_, lower=lower)
                    clades_used[clade_style["N"]] = [{**clade_style, **clade_style_mod}, len(indices)]
        module_logger.info('Clades\n{}'.format(pprint.pformat(clades_used)))
        # module_logger.info('Clades {}'.format(sorted(clades_used)))
        if legend and legend.get("show", True):
            def make_label(clade, count):
                label = clade or "sequenced"
                if legend.get("point_count"):
                    label += " ({})".format(count)
                return label
            self._make_legend(
                legend_data=sorted(({"label": make_label(clade, data[1]), **{k:v for k,v in data[0].items() if k not in ["N", "show"]}} for clade, data in clades_used.items()), key=operator.itemgetter("label")),
                legend_settings=legend)

    def aa_substitutions(self, positions, legend=None, **args):
        from . import seqdb_access
        aa_indices = seqdb_access.aa_at_positions(chart=self._chart, positions=positions, verbose=self._verbose)
        aa_order = sorted(aa_indices, key=lambda aa: - len(aa_indices[aa]))
        dc = distinct_colors()
        aa_color = {aa: dc[no] for no, aa in enumerate(aa_order)}
        if "X" in aa_color:
            aa_color["X"] = "grey25"
        # print(aa_color)
        for aa in aa_order:
            self._chart_draw.modify_points_by_indices(aa_indices[aa], self._make_point_style({"outline": "black", "fill": aa_color[aa]}), raise_=True)
        if legend and legend.get("show", True):
            self._make_legend(
                legend_data=[{"label": "{} {:3d}".format(aa, len(aa_indices[aa])), "outline": "black", "fill": aa_color[aa]} for aa in aa_order],
                legend_settings=legend)

    def aa_substitution_groups(self, groups, legend=None, **args):
        """groups: [{"pos_aa": ["121K", "144K"], "color": "#03569b"}]
        """
        from . import seqdb_access
        for group in groups:
            positions = [int(pos_aa[:-1]) for pos_aa in group["pos_aa"]]
            target_aas = "".join(pos_aa[-1] for pos_aa in group["pos_aa"])
            aa_indices = seqdb_access.aa_at_positions(chart=self._chart, positions=positions, verbose=self._verbose)
            if target_aas in aa_indices:
                self._chart_draw.modify_points_by_indices(aa_indices[target_aas], self._make_point_style({"outline": "black", "fill": group["color"]}), raise_=True)
            else:
                module_logger.warning('No {}: {}'.format(target_aas, sorted(aa_indices)))

    def vaccines(self, raise_=True, label=None, **args):
        # fill=None, outline=None, show=None, shape=None, size=None, outline_width=None, aspect=None, rotation=None
        hidb = get_hidb(chart=self._chart)
        for vaccine_entry in vaccines(chart=self._chart):
            # module_logger.debug('{}'.format(vaccine_entry))
            antigens = find_vaccines_in_chart(vaccine_entry["name"], self._chart, hidb)
            if self._verbose:
                module_logger.debug("Vaccines {}\n{}".format(vaccine_entry, antigens.report(indent=4)))
            for passage_type in ["egg", "reassortant", "cell"]:
                vaccine_key = vaccine_entry["type"] + "-" + passage_type
                vaccine_data = getattr(antigens, passage_type)(args.get(vaccine_key, {}).get("no", 0))
                if vaccine_data:
                    module_logger.info('Marking vaccine {} {}'.format(vaccine_data.antigen_index, vaccine_data.antigen.full_name()))
                    if args.get(vaccine_key, {}).get("show", True):
                        self._chart_draw.modify_point_by_index(vaccine_data.antigen_index, self._make_point_style({**self.sStyleByVaccineType[vaccine_entry["type"]][passage_type], **args}), raise_=raise_)
                        if label:
                            if label.get(vaccine_key):
                                self.label(index=vaccine_data.antigen_index, **{**label.get("", {}), **label[vaccine_key]})
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
                    args["display_name"] = self._chart.antigen(index).full_name()
            else:
                if name_type in ["abbreviated", "abbreviated_with_passage_type"]:
                    args["display_name"] = self._chart.serum(index - self._chart.number_of_antigens()).abbreviated_name(get_locdb())
                else:
                    args["display_name"] = self._chart.serum(index - self._chart.number_of_antigens()).full_name()
        for k, v in args.items():
            setter = getattr(lbl, k, None)
            if setter:
                setter(v)

    def title(self, display_name=None, **args):
        ttl = self._chart_draw.title()
        if isinstance(display_name, str):
            ttl.add_line(display_name)
        elif isinstance(display_name, list):
            for n in display_name:
                ttl.add_line(n)
        else:
            ttl.add_line(self._chart.make_name())
        for k, v in args.items():
            setter = getattr(ttl, k, None)
            if setter:
                setter(v)

    def serum_circle(self, serum, antigen=None, mark_serum=None, mark_antigen=None, **args):
        serum_index = self._select_sera(select=serum, raise_if_not_found=True, raise_if_multiple=True)[0]
        if mark_serum:
            self.sera(N="sera", select=serum_index, **mark_serum)
        sera = self._chart.sera()
        if antigen is None:
            get_hidb(chart=self._chart).find_homologous_antigens_for_sera_of_chart(chart=self._chart)
            antigen_indices = sera[serum_index].homologous()
        else:
            antigen_indices = self._select_antigens(select=antigen, raise_if_not_found=True, raise_if_multiple=False)
        if mark_antigen:
            self.antigens(N="antigens", select=antigen_indices, **mark_antigen)
        antigens = self._chart.antigens()
        radii = [self._chart.serum_circle_radius(serum_no=serum_index, antigen_no=ag_no, projection_no=self._projection_no) for ag_no in antigen_indices]
        radius = min(r for r in radii if r > 0)
        module_logger.info('serum_circle:\n  SR {} {}\n    {}\n  RADIUS: {}'.format(serum_index, sera[serum_index].full_name(), "\n    ".join("AG {:4d} {} {}".format(ag_no, antigens[ag_no].full_name(), radius) for ag_no, radius in zip(antigen_indices, radii)), radius))
        serum_circle = self._chart_draw.serum_circle(serum_no=serum_index, radius=radius)

    def _make_point_style(self, *data):
        style = PointStyle()
        for source in data:
            for k, v in source.items():
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
                legend_box.add_line(**legend_entry)

    def _select_antigens(self, select, raise_if_not_found, raise_if_multiple):
        indices = None
        antigens = self._chart.antigens()
        if isinstance(select, str):
            if select == "reference":
                indices = antigens.reference_indices()
            elif select == "test":
                indices = antigens.test_indices()
            elif select == "all":
                indices = list(range(self._chart.number_of_antigens()))
            elif select == "sequenced":
                from .seqdb_access import sequenced
                indices = sequenced(chart=self._chart, verbose=self._verbose)
            elif select == "not_sequenced":
                from .seqdb_access import not_sequenced
                indices = not_sequenced(chart=self._chart, verbose=self._verbose)
            else:
                indices = antigens.find_by_name_matching(select)
        elif isinstance(select, dict):
            if "date_range" in select:
                indices = antigens.date_range_indices(first=select["date_range"][0], after_last=select["date_range"][1])
            elif "older_than_days" in select:
                indices = antigens.date_range_indices(after_last=(datetime.date.today() - datetime.timedelta(days=select["older_than_days"])).strftime("%Y-%m-%d"))
            elif "younger_than_days" in select:
                indices = antigens.date_range_indices(first=(datetime.date.today() - datetime.timedelta(days=select["younger_than_days"])).strftime("%Y-%m-%d"))
            elif "passage_type" in select:
                if select["passage_type"] == "egg":
                    indices = antigens.egg_indices()
                elif select["passage_type"] == "cell":
                    indices = antigens.cell_indices()
                elif select["passage_type"] == "reassortant":
                    indices = antigens.reassortant_indices()
            elif "country" in select:
                indices = antigens.country(select["country"].upper(), get_locdb())
                if not indices:
                    countries = antigens.countries(get_locdb())
                    module_logger.warning('No antigens from {},\n   there are antigens from: {}'.format(select["country"].upper(), " ".join("{}={}".format(c, len(countries[c])) for c in sorted(countries))))
            elif "continent" in select:
                indices = antigens.continents(get_locdb())[select["continent"].upper()]
            elif "name" in select:
                indices = antigens.find_by_name_matching(select["name"])
            elif "index" in select:
                indices = select["index"]
                if isinstance(indices, int):
                    indices = [indices]
            elif "indices" in select:
                indices = select["indices"]
        elif isinstance(select, list) and all(self._valid_antigen_index(ind) for ind in select):
            indices = select
        elif self._valid_antigen_index(select):
            indices = [select]
        if indices:
            if raise_if_multiple and len(indices) > 1:
                raise ValueError("Multiple antigens selected using " + repr(select) + ": " + str(indices))
        elif raise_if_not_found:
            raise ValueError("No antigens selected using " + repr(select))
        return indices

    def _valid_antigen_index(self, index):
        return isinstance(index, int) and index >= 0 and index < self._chart.number_of_antigens()

    def _select_sera(self, select, raise_if_not_found, raise_if_multiple):
        indices = None
        sera = self._chart.sera()
        if isinstance(select, str):
            if select == "all":
                indices = list(range(self._chart.number_of_sera()))
            else:
                indices = sera.find_by_name_matching(select)
        elif isinstance(select, dict):
            if "name" in select:
                indices = sera.find_by_name_matching(select["name"])
            elif "index" in select:
                indices = select["index"]
                if isinstance(indices, int):
                    indices = [indices]
            elif "indices" in select:
                indices = select["indices"]
        elif isinstance(select, list) and all(self._valid_serum_index(ind) for ind in select):
            indices = select
        elif self._valid_serum_index(select):
            indices = [select]
        if indices:
            if raise_if_multiple and len(indices) > 1:
                raise ValueError("Multiple sera selected using " + repr(select) + ": " + str(indices))
        elif raise_if_not_found:
            raise ValueError("No sera selected using " + repr(select))
        return indices

    def _valid_serum_index(self, index):
        return isinstance(index, int) and index >= 0 and index < self._chart.number_of_sera()

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
