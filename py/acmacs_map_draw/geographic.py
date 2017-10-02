# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import pprint
import logging; module_logger = logging.getLogger(__name__)

from acmacs_base.timeit import timeit

from .hidb_access import get_hidb
from .locdb_access import get_locdb
from .seqdb_access import get_seqdb

# ----------------------------------------------------------------------

def geographic_map_default_setting():
    return {
        "coloring": "clade",
        "coloring?": ["continent", "clade", "lineage", "lineage-deletion-mutants"],
        "color_override?": {"B": {"?": "B/Vic deletion mutants", "?B/DOMINICAN REPUBLIC/9932/2016": "#00FFFF"}},
        "deletion_mutant_color": "cyan",
        "point_size_in_pixels": 4.0,
        "point_density": 0.8,
        "outline_color": "grey63",
        "outline_width": 0.5,
        "output_image_width": 800,
        "title": {"offset": [0, 0], "text_size": 20, "background": "transparent", "border_width": 0},
        }

# ----------------------------------------------------------------------

def geographic_map(output, virus_type, title, start_date, end_date, settings):
    with timeit("Drawing geographic map to " + str(output)):
        options = {}
        for key in ["point_size_in_pixels", "point_density", "outline_color", "outline_width"]:
            if settings.get(key) is not None:
                options[key] = settings[key]
        from acmacs_map_draw_backend import GeographicMapWithPointsFromHidb
        geographic_map = GeographicMapWithPointsFromHidb(hidb=get_hidb(virus_type=virus_type), locdb=get_locdb(), **options)
        if not settings.get("coloring") or settings["coloring"] == "continent":
            from .coloring import sContinentColor
            geographic_map.add_points_from_hidb_colored_by_continent(continent_color=sContinentColor, color_override={}, start_date=start_date, end_date=end_date)
        elif settings["coloring"] == "clade":
            from .coloring import sCladeColor
            geographic_map.add_points_from_hidb_colored_by_clade(clade_color=sCladeColor, color_override={}, seqdb=get_seqdb(), start_date=start_date, end_date=end_date)
        elif settings["coloring"] == "lineage":
            from .coloring import sLineageColor
            geographic_map.add_points_from_hidb_colored_by_lineage(lineage_color=sLineageColor, color_override={}, start_date=start_date, end_date=end_date)
        else:
            raise ValueError("Unsupported coloring: " + repr(settings["coloring"]))
        if title:
            _update_title(geographic_map.title(), settings).add_line(title)
        geographic_map.draw(str(output), settings.get("output_image_width", 800))

# ----------------------------------------------------------------------

def geographic_time_series(output_prefix, virus_type, period, start_date, end_date, settings):
    #pprint.pprint(settings)
    options = {key: settings[key] for key in ["point_size_in_pixels", "point_density", "outline_color", "outline_width"] if settings.get(key) is not None}
    if period == "month":
        from acmacs_map_draw_backend import geographic_time_series_monthly
        hidb = get_hidb(virus_type=virus_type, hidb_dir=settings.get("hidb_dir"))
        module_logger.info("hidb {}".format(hidb.all_antigens()))
        ts = geographic_time_series_monthly(start_date=start_date, end_date=end_date, hidb=hidb, locdb=get_locdb(), **options)
    elif period == "year":
        from acmacs_map_draw_backend import geographic_time_series_yearly
        ts = geographic_time_series_yearly(start_date=start_date, end_date=end_date, hidb=get_hidb(virus_type=virus_type, hidb_dir=settings.get("hidb_dir")), locdb=get_locdb(), **options)
    elif period == "week":
        from acmacs_map_draw_backend import geographic_time_series_weekly
        ts = geographic_time_series_weekly(start_date=start_date, end_date=end_date, hidb=get_hidb(virus_type=virus_type, hidb_dir=settings.get("hidb_dir")), locdb=get_locdb(), **options)
    else:
        raise ValueError("Unsupported period: " + repr(period) + ", expected \"month\", \"year\", \"week\"")
    _update_title(ts.title(), settings)
    if not settings.get("coloring") or settings["coloring"] == "continent":
        from .coloring import sContinentColor
        ts.draw_colored_by_continent(filename_prefix=output_prefix, continent_color=sContinentColor, color_override=settings.get("color_override", {}), image_width=settings.get("output_image_width", 800))
    elif settings["coloring"] == "clade":
        from .coloring import sCladeColor
        ts.draw_colored_by_clade(filename_prefix=output_prefix, clade_color=sCladeColor, seqdb=get_seqdb(seqdb_dir=settings.get("seqdb_dir")), color_override=settings.get("color_override", {}), image_width=settings.get("output_image_width", 800))
    elif settings["coloring"] == "lineage":
        from .coloring import sLineageColor
        ts.draw_colored_by_lineage(filename_prefix=output_prefix, lineage_color=sLineageColor, color_override=settings.get("color_override", {}), image_width=settings.get("output_image_width", 800))
    elif settings["coloring"] == "lineage-deletion-mutants":
        from .coloring import sLineageColor
        ts.draw_colored_by_lineage_and_deletion_mutants(filename_prefix=output_prefix, lineage_color=sLineageColor, deletion_mutant_color=settings.get("deletion_mutant_color", "cyan"), color_override=settings.get("color_override", {}), seqdb=get_seqdb(seqdb_dir=settings.get("seqdb_dir")), image_width=settings.get("output_image_width", 800))
    else:
        raise ValueError("Unsupported coloring: " + repr(settings["coloring"]))

# ----------------------------------------------------------------------

def _update_title(title, settings):
    for k, v in settings.get("title", {}).items():
        setter = getattr(title, k, None)
        if setter:
            if isinstance(v, list):
                setter(*v)
            else:
                setter(v)
    return title

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
