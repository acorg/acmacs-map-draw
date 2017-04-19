# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import os, copy, math, operator, datetime, pprint
from pathlib import Path
import logging; module_logger = logging.getLogger(__name__)
from .hidb_access import get_hidb
from .locdb_access import get_locdb
from . import coloring

# ----------------------------------------------------------------------

class UnrecognizedMod (ValueError): pass

# ----------------------------------------------------------------------

def draw_chart(output_file, chart, settings, output_width, previous_chart=None, draw_map=True, seqdb_file=None, verbose=False):
    from acmacs_map_draw_backend import ChartDraw
    chart_draw = ChartDraw(chart)
    chart_draw.prepare()
    # module_logger.debug('{} {} {}'.format(chart.chart_info().lab(), chart.chart_info().virus_type(), chart.chart_info().assay()))

    applicator = ModApplicator(chart_draw=chart_draw, chart=chart, previous_chart=previous_chart, projection_no=0, seqdb_file=seqdb_file, verbose=verbose)
    for mod in settings.get("mods", []):
        if isinstance(mod, str):
            if mod and mod[0] != "?" and mod[-1] != "?":
                try:
                    getattr(applicator, mod)(N=mod)
                except:
                    raise UnrecognizedMod(mod)
        elif isinstance(mod, dict):
            if "N" in mod and mod["N"] and mod["N"][0] != "?":      # no "N" - commented out
                try:
                    getattr(applicator, mod["N"])(**mod)
                except Exception as err:
                    raise UnrecognizedMod(mod, err)
        else:
            raise UnrecognizedMod(mod)
    chart_draw.calculate_viewport()
    if draw_map:
        chart_draw.draw(str(output_file), output_width)
    return chart_draw, applicator._antigens_shown_on_all

# ----------------------------------------------------------------------

def antigenic_time_series(output_prefix, chart, period, start_date, end_date, output_width, settings, title_prefix=None, previous_chart=None, seqdb_file=None, verbose=False):
    from acmacs_map_draw_backend import PointStyle
    chart_draw, antigens_shown_on_all = draw_chart(output_file=None, chart=chart, previous_chart=previous_chart, settings=settings, output_width=None, draw_map=False, seqdb_file=seqdb_file, verbose=verbose)
    if period == "month":
        from acmacs_map_draw_backend import MonthlyTimeSeries
        ts = MonthlyTimeSeries(start=start_date, end=end_date)
    elif period == "year":
        from acmacs_map_draw_backend import YearlyTimeSeries
        ts = YearlyTimeSeries(start=start_date, end=end_date)
    elif period == "week":
        from acmacs_map_draw_backend import WeeklyTimeSeries
        ts = WeeklyTimeSeries(start=start_date, end=end_date)
    else:
        raise ValueError("Unsupported period: " + repr(period) + ", expected \"month\", \"year\", \"week\"")
    number_of_antigens = chart.number_of_antigens()
    shown_on_all = sorted(antigens_shown_on_all | set(chart.antigens().reference_indices()) | set(sr_no + number_of_antigens for sr_no in range(chart.number_of_sera())))
    # module_logger.debug('Shown_on_all {}'.format(shown_on_all))
    for ts_entry in ts:
        module_logger.debug('TS {!r} {!r} {}..{}'.format(ts_entry.numeric_name(), ts_entry.text_name(), ts_entry.first_date(), ts_entry.after_last_date()))
        chart_draw.hide_all_except(shown_on_all)
        chart_draw.modify(indices=chart.antigens().date_range_indices(ts_entry.first_date(), ts_entry.after_last_date()), style=PointStyle().show(True))
        chart_draw.title().remove_all_lines().add_line((title_prefix or "") + ts_entry.text_name());
        chart_draw.draw("{}-{}.pdf".format(output_prefix, ts_entry.numeric_name()), output_width)

# ----------------------------------------------------------------------

class ModApplicator:

    # sStyleByContinent = {
    #     "EUROPE":            {"fill": "green"},
    #     "CENTRAL-AMERICA":   {"fill": "#AAF9FF"},
    #     "MIDDLE-EAST":       {"fill": "#8000FF"},
    #     "NORTH-AMERICA":     {"fill": "blue4"},
    #     "AFRICA":            {"fill": "darkorange1"},
    #     "ASIA":              {"fill": "red"},
    #     "RUSSIA":            {"fill": "maroon"},
    #     "AUSTRALIA-OCEANIA": {"fill": "hotpink"},
    #     "SOUTH-AMERICA":     {"fill": "turquoise"},
    #     "ANTARCTICA":        {"fill": "grey50"},
    #     "":                  {"fill": "grey50"},
    #     }

    sStyleByVaccineType_old = {
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

    sStyleByVaccineType = [
        {"type": "previous", "fill": "blue", "outline": "black"},
        {"type": "current", "fill": "red", "outline": "black"},
        {"type": "current", "passage": "reassortant", "fill": "green"},
        {"type": "surrogate", "fill": "pink", "outline": "black"},
        ]

    sStyleByClade = [                     # bottom raised entries above top entries
        {"N": "", "outline": "black"},                # sequenced but not in any clade
        # {"N": "gly", "fill": "grey50", "outline": "black"},
        # {"N": "no-gly", "fill": "grey50", "outline": "black"},
        # H3
        {"N": "3C3",   "outline": "black"},
        {"N": "3C2a",  "outline": "black"},
        {"N": "3C2a1", "outline": "black"},
        {"N": "3C3a",  "outline": "black"},
        {"N": "3C3b",  "outline": "black"},
        # H1pdm
        {"N": "6B1", "outline": "black"},
        {"N": "6B2", "outline": "black"},
        # B/Yam
        {"N": "Y2", "outline": "black"},
        {"N": "Y3", "outline": "black"},
        # B/Vic
        {"N": "1",  "outline": "black"},
        {"N": "1A", "outline": "black"},
        {"N": "1B", "outline": "black"},
        ]

    # ----------------------------------------------------------------------

    def __init__(self, chart_draw, chart, previous_chart, projection_no, seqdb_file, verbose=False):
        self._chart_draw = chart_draw
        self._chart = chart
        self._previous_chart = previous_chart
        self._projection_no = projection_no
        self._seqdb_file = seqdb_file
        self._verbose = verbose
        self._antigens_shown_on_all = set() # for time series

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

    def antigens(self, N, select, raise_if_not_found=True, raise_if_multiple=False, report=True, report_names_threshold=10, **args):
        indices = self._select_antigens(select=select, raise_if_not_found=raise_if_not_found, raise_if_multiple=raise_if_multiple)
        antigens = self._chart.antigens()
        if report:
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

    def sera(self, N, select, raise_if_not_found=True, raise_if_multiple=False, report=True, report_names_threshold=10, homologous_ag_no=None, **args):
        indices = self._select_sera(select=select, raise_if_not_found=raise_if_not_found, raise_if_multiple=raise_if_multiple)
        sera = self._chart.sera()
        if report:
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
                if args["label"].get("name_type") == "abbreviated_hom_max" and homologous_ag_no is not None and args["label"].get("display_name") is None:
                    args["label"]["display_name"] = "{} ({}; hom: {}; max: {})".format(self._chart.serum(index).abbreviated_name(get_locdb()),
                                                                          self._chart.antigen(homologous_ag_no).passage_type(),
                                                                          self._chart.titers().get(ag_no=homologous_ag_no, sr_no=index),
                                                                          self._chart.titers().max_for_serum(sr_no=index))
                self.label(index=index + number_of_antigens, **args["label"])

    def serology(self, N, name, mark_no=None, size=50, fill="orange", outline="black", raise_=True, report=True, report_names_threshold=10, **args):
        self.antigens(N=N, select={"name": name, "match_virus_name": True, "mark_no": mark_no}, size=size, fill=fill, outline=outline, raise_=raise_, raise_if_not_found=False, report=report, report_names_threshold=report_names_threshold, **args)

    def style(self, index=None, indices=None, raise_=None, lower=None, **args):
        # fill=None, outline=None, show=None, shape=None, size=None, outline_width=None, aspect=None, rotation=None
        if index is None:
            if indices is None:
                raise ValueError("One of index or indices must be provided")
            index = indices
        elif indices is not None:
            raise ValueError("Either index or indices must be provided")
        self._chart_draw.modify(index, style=self._make_point_style(args), raise_=bool(raise_), lower=bool(lower))

    def continents(self, legend=None, exclude_reference=True, **args):
        data = self._chart.antigens().continents(locdb=get_locdb(), exclude_reference=exclude_reference)
        module_logger.info('[Continents] {}'.format(" ".join("{}:{}".format(continent, len(data[continent])) for continent in sorted(data))))
        for continent, indices in data.items():
            self._chart_draw.modify(indices, self._make_point_style(args, {"fill": coloring.sContinentColor[continent]}))
        if legend and legend.get("show", True):
            self._chart_draw.continent_map(legend.get("offset", [0, 0]), legend.get("size", 100))

    def clades(self, light=None, legend=None, **args):
        from .seqdb_access import antigen_clades
        clade_data = antigen_clades(self._chart, seqdb_file=self._seqdb_file, verbose=self._verbose)
        # pprint.pprint(clade_data)
        clades_used = {}                      # for legend
        for clade_style in self.sStyleByClade:
            clade_style_mod = args.get(clade_style["N"], {"show": True})
            if clade_style_mod.get("show", True):
                indices = clade_data.get(clade_style["N"])
                lower = clade_style_mod.get("lower", False)
                raise_ = clade_style_mod.get("raise_", True) if not lower else False
                if indices:
                    style = {**clade_style, "fill": coloring.sCladeColor[clade_style["N"]], "light": light, **clade_style_mod}
                    self._chart_draw.modify(indices, self._make_point_style(style), raise_=raise_, lower=lower)
                    clades_used[clade_style["N"]] = [style, len(indices)]
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
        aa_indices = seqdb_access.aa_at_positions(chart=self._chart, positions=positions, seqdb_file=self._seqdb_file, verbose=self._verbose)
        aa_order = sorted(aa_indices, key=lambda aa: - len(aa_indices[aa]))
        from acmacs_map_draw_backend import distinct_colors
        dc = distinct_colors()
        aa_color = {aa: dc[no] for no, aa in enumerate(aa_order)}
        if "X" in aa_color:
            aa_color["X"] = "grey25"
        # print(aa_color)
        for aa in aa_order:
            self._chart_draw.modify(aa_indices[aa], self._make_point_style({"outline": "black", "fill": aa_color[aa]}), raise_=True)
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
            aa_indices = seqdb_access.aa_at_positions(chart=self._chart, positions=positions, seqdb_file=self._seqdb_file, verbose=self._verbose)
            if target_aas in aa_indices:
                self._chart_draw.modify(aa_indices[target_aas], self._make_point_style({"outline": "black", "fill": group["color"]}), raise_=True)
            else:
                module_logger.warning('No {}: {}'.format(target_aas, sorted(aa_indices)))

    def vaccines(self, raise_=True, mods=None, **args):
        # fill=None, outline=None, show=None, shape=None, size=None, outline_width=None, aspect=None, rotation=None

        def matcher_apply(matcher, prefix, **args):
            for k,v in args.items():
                f = getattr(matcher, k, None)
                if f is not None:
                    if k == "label":
                        matcher_apply(matcher.label(self._chart_draw), prefix=prefix + " " + k, **v)
                    else:
                        f(v)
                elif not k or (k[0] != "?" and k[-1] != "?" and k not in ["N"]):
                    module_logger.warning('{}: unrecognized {!r}:{!r}'.format(prefix, k, v))

        def make_matcher_apply(vaccs, prefix, name="", type="", passage_type="", passage=None, **args):
            matcher_apply(vaccs.match(name=name, type=type, passage_type=passage or passage_type), prefix=prefix, **args)

        hidb = get_hidb(chart=self._chart)
        from acmacs_map_draw_backend import Vaccines
        vaccs = Vaccines(chart=self._chart, hidb=hidb)
        for style_vac in self.sStyleByVaccineType:
            make_matcher_apply(vaccs, prefix="Vaccines", **style_vac)
        make_matcher_apply(vaccs, prefix="Vaccines", **args)
        for mod in (mods or []):
            make_matcher_apply(vaccs, prefix="Vaccine mod", **mod)

        # module_logger.debug('ALL\n{}'.format(vaccs.report_all(2)))
        module_logger.debug('FILTERED\n{}'.format(vaccs.report(2)))
        vaccs.plot(self._chart_draw)

        # ----------------------------------------------------------------------

        if False:
            def _collect_old():
                from hidb_backend import vaccines, find_vaccines_in_chart
                for vaccine_entry in vaccines(chart=self._chart):
                    antigens = find_vaccines_in_chart(vaccine_entry.name, self._chart, hidb)
                    for passage_type in ["egg", "reassortant", "cell"]:
                        num_entries = getattr(antigens, "number_of_" + passage_type + "s")()
                        if num_entries > 0:
                            passage_func = getattr(antigens, passage_type)
                            def make_vaccine(no):
                                vv = passage_func(no)
                                return {"antigen_index": vv.antigen_index, "number_of_tables": vv.antigen_data.number_of_tables(), "name": vv.antigen.full_name(), "most_recent_table": vv.antigen_data.most_recent_table().table_id()}
                            yield {"no": 0, "passage": passage_type, "type": vaccine_entry.type, "name": vaccine_entry.name, "vaccines": [make_vaccine(no) for no in range(num_entries)]}

            def _filter_old(vacs):
                if mods:
                    for vac in vacs:
                        vac_mod = copy.deepcopy(vac)
                        for mod in mods:
                            if (not mod.get("name") or mod["name"] in vac["name"]) and (not mod.get("passage") or mod["passage"] == vac["passage"]) and (not mod.get("type") or mod["type"] == vac["type"]):
                                if mod.get("label"):
                                    vac_mod["label"] = {**vac_mod.get("label", {}), **mod["label"]}
                                vac_mod.update({k: v for k,v in mod.items() if k not in ["label", "name", "passage", "type"]})
                        if vac_mod.get("show", True):
                            yield vac_mod
                else:
                    for vac in vacs:
                        yield vac

            def _add_plot_spec_old(vac):
                return {**vac, **self.sStyleByVaccineType_old[vac["type"]][vac["passage"]], **args}

            def _report_old(vacs):
                if self._verbose:
                    for vac in vacs:
                        longest_name = max(len(vv["name"]) for vv in vac["vaccines"])
                        module_logger.debug('Vaccine {} {} ({})\n  {}'.format(vac["type"], vac["passage"], len(vac["vaccines"]), "\n  ".join("{:2d}  {:5d} {:{}s} tabs:{:3d} recent:{}".format(no, vv["antigen_index"], vv["name"], longest_name, vv["number_of_tables"], vv["most_recent_table"]) for no, vv in enumerate(vac["vaccines"]))))

            def _plot_old(vac):
                module_logger.info('Vaccine {} {} {} => {} {}'.format(vac["type"], vac["passage"], vac["name"], vac["vaccines"][vac["no"]]["antigen_index"], vac["vaccines"][vac["no"]]["name"]))
                antigen_index = vac["vaccines"][vac["no"]]["antigen_index"]
                self._chart_draw.modify(antigen_index, self._make_point_style(vac), raise_=raise_)
                if vac.get("label"):
                    self.label(index=antigen_index, **vac["label"])

            vacs = list(_collect_old())
            #_report_old(vacs)
            vacs_with_plot_spec = [_add_plot_spec_old(vac) for vac in vacs]
            # module_logger.debug("Plot spec\n" + pprint.pformat(vacs_with_plot_spec))
            vacs_filtered = list(_filter_old(vacs_with_plot_spec))
            # module_logger.debug("Filtered\n" + pprint.pformat(vacs_filtered))
            for vac in vacs_filtered:
                _plot_old(vac)
                self._antigens_shown_on_all.add(vac["vaccines"][vac["no"]]["antigen_index"])

    def label(self, index, name_type="full", **args):
        lbl = self._chart_draw.label(index)
        if "display_name" not in args:
            if index < self._chart.number_of_antigens():
                if name_type == "abbreviated":
                    args["display_name"] = self._chart.antigen(index).abbreviated_name(get_locdb())
                elif name_type == "abbreviated_with_passage_type":
                    args["display_name"] = self._chart.antigen(index).abbreviated_name_with_passage_type(get_locdb())
                else:
                    if name_type != "full":
                        module_logger.warning('Unsupported name_type {!r}, "full" will be used'.format(name_type))
                    args["display_name"] = self._chart.antigen(index).full_name()
            else:
                if name_type in ["abbreviated", "abbreviated_with_passage_type"]:
                    args["display_name"] = self._chart.serum(index - self._chart.number_of_antigens()).abbreviated_name(get_locdb())
                else:
                    if name_type != "full":
                        module_logger.warning('Unsupported name_type {!r}, "full" will be used'.format(name_type))
                    args["display_name"] = self._chart.serum(index - self._chart.number_of_antigens()).full_name()
        for k, v in args.items():
            setter = getattr(lbl, k, None)
            if setter:
                setter(v)

    def title(self, display_name=None, remove_all_lines=False, **args):
        ttl = self._chart_draw.title()
        if remove_all_lines:
            ttl.remove_all_lines()
        if not remove_all_lines or display_name:
            ttl.add_line(display_name or self._chart.make_name(self._projection_no))
        for k, v in args.items():
            setter = getattr(ttl, k, None)
            if setter:
                setter(v)

    def serum_circle(self, serum, antigen=None, mark_serum=None, mark_antigen=None, circle={}, **args):
        if serum != "":
            serum_index = self._select_sera(select=serum, raise_if_not_found=True, raise_if_multiple=True)[0]
            antigen_indices = self._homologous_antigen_indices(serum_no=serum_index, select=antigen, raise_if_not_found=True, raise_if_multiple=False)
            if mark_serum:
                self.sera(N="sera", select=serum_index, homologous_ag_no=antigen_indices[0], **mark_serum)
            if mark_antigen:
                self.antigens(N="antigens", select=antigen_indices, **mark_antigen)
            radii = [self._chart.serum_circle_radius(serum_no=serum_index, antigen_no=ag_no, projection_no=self._projection_no, verbose=False) for ag_no in antigen_indices]
            radius = min((r for r in radii if r > 0), default=0)
            module_logger.info('serum_circle:\n  SR {} {}\n    {}\n  RADIUS: {}'.format(serum_index, self._chart.serum(serum_index).full_name(), "\n    ".join("AG {:4d} {} {}".format(ag_no, self._chart.antigen(ag_no).full_name(), radius) for ag_no, radius in zip(antigen_indices, radii)), radius))
            serum_circle = self._chart_draw.serum_circle(serum_no=serum_index, radius=radius)
            if circle.get("fill"):
                serum_circle.fill(color=circle["fill"])
            if circle.get("outline"):
                serum_circle.outline(color=circle["outline"], line_width=circle.get("outline_width", 1.0))
                serum_circle.radius_line(color=circle["outline"], line_width=circle.get("outline_width", 1.0))
            if circle.get("radius_line"):
                serum_circle.radius_line(color=circle["radius_line"], line_width=circle.get("radius_line", 1.0))
            if circle.get("radius_line_dash") is not None:
                if circle["radius_line_dash"] == "nodash" or not circle["radius_line_dash"]:
                    serum_circle.radius_line_no_dash()
                elif circle["radius_line_dash"] == "dash1":
                    serum_circle.radius_line_dash1()
                elif circle["radius_line_dash"] == "dash2":
                    serum_circle.radius_line_dash2()
            if circle.get("angle_degrees"):
                serum_circle.angles(circle["angle_degrees"][0] * math.pi / 180.0, circle["angle_degrees"][1] * math.pi / 180.0)

    def serum_coverage(self, serum, antigen=None, antigen_no=0, within_4fold={"outline": "pink", "outline_width": 5, "raise_": True}, outside_4fold={}, mark_serum=None, report=False, **args):
        serum_index = self._select_sera(select=serum, raise_if_not_found=True, raise_if_multiple=True)[0]
        antigen_index = self._homologous_antigen_indices(serum_no=serum_index, select=antigen, raise_if_not_found=True, raise_if_multiple=False)[antigen_no]
        if mark_serum:
            self.sera(N="sera", select=serum_index, homologous_ag_no=antigen_index, **mark_serum)
        within, outside = self._chart.serum_coverage(antigen_no=antigen_index, serum_no=serum_index)
        if report:
            module_logger.info('Antigens within 4fold: {}'.format(within))
            module_logger.info('Antigens outside 4fold: {}'.format(outside))
        if within:
            self.antigens(N="antigens", select=within, report=False, **within_4fold)
        if outside:
            self.antigens(N="antigens", select=outside, report=False, **outside_4fold)

    def compare_with_previous(self, new=None, old=None, ignore_reference=True, **args):
        if self._previous_chart is not None:
            new_indices = set(self._chart.antigens_not_found_in(self._previous_chart))
            # module_logger.debug('compare_with_previous new: {}'.format(new_indices))
            all_indices = set(range(self._chart.number_of_antigens()))
            test_indices = set(self._chart.antigens().test_indices()) if ignore_reference else all_indices
            if new is not None:
                self.antigens(N="antigens", select=list(new_indices | test_indices), report=False, **new)
            if old is not None:
                self.antigens(N="antigens", select=list(test_indices - new_indices), report=False, **old)

    def _make_point_style(self, *data):
        from acmacs_map_draw_backend import PointStyle
        style = PointStyle()
        for source in data:
            light = source.get("light")
            for k, v in source.items():
                setter = getattr(style, k, None)
                if setter:
                    if light is not None and k in ["fill", "outline"]:
                        setter(v, light)
                    else:
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
                if not isinstance(legend_entry.get("light"), float):
                    legend_entry.pop("light", None)
                legend_box.add_line(**legend_entry)

    def _homologous_antigen_indices(self, serum_no, select, raise_if_not_found, raise_if_multiple):
        if select is None:
            get_hidb(chart=self._chart).find_homologous_antigens_for_sera_of_chart(chart=self._chart)
            select = self._chart.serum(serum_no).homologous()
        return self._select_antigens(select=select, raise_if_not_found=raise_if_not_found, raise_if_multiple=raise_if_multiple)

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
                indices = antigens.find_by_name_matching(name=select, verbose=self._verbose)
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
                if select.get("match_virus_name"):
                    from acmacs_chart_backend import virus_name_match_threshold
                    score_threshold = virus_name_match_threshold(select["name"])
                else:
                    score_threshold = select.get("score_threshold", 0)
                module_logger.debug('score_threshold {} match_virus_name {}'.format(score_threshold, select.get("match_virus_name")))
                indices = antigens.find_by_name_matching(name=select["name"], score_threshold=score_threshold, verbose=self._verbose)
                if len(indices) > 1 and select.get("mark_no") is not None:
                    indices = [indices[select["mark_no"]]]
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
                raise ValueError("Multiple antigens selected using " + repr(select) + ": " + str(indices) + "\n  " + "\n  ".join("AG {} {}".format(antigen_no, antigens[antigen_no].full_name()) for antigen_no in indices))
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
                indices = sera.find_by_name_matching(name=select, verbose=self._verbose)
        elif isinstance(select, dict):
            if "name" in select:
                indices = sera.find_by_name_matching(name=select["name"], score_threshold=select.get("score_threshold", 0), verbose=self._verbose)
                if select.get("no") is not None:
                    indices = [indices[select["no"]]]
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
                raise ValueError("Multiple sera selected using " + repr(select) + ": " + str(indices) + "\n  " + "\n  ".join("SR {} {}".format(serum_no, sera[serum_no].full_name()) for serum_no in indices))
        elif raise_if_not_found:
            raise ValueError("No sera selected using " + repr(select))
        return indices

    def _valid_serum_index(self, index):
        return isinstance(index, int) and index >= 0 and index < self._chart.number_of_sera()

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
