import {sval, sval_call} from "../draw/utils.js";
import {Surface} from "../draw/acmacs-draw-surface.js";

// ----------------------------------------------------------------------

export class AntigenicMapWidget {
    constructor(div, data, options={}) {
        this.canvas = $("<canvas></canvas>").appendTo($(div));

        if (!Array.isArray(data)) {
            data = [data];
        }

        this.surface = new Surface(this.canvas, {canvas: sval("canvas", data, {width: 500, height: 500}), viewport: sval("viewport", data, [0, 0, 10, 10])});
        sval_call("border", data, v => this.surface.border(v));
        sval_call("background", data, v => this.surface.background(v));
        sval_call("grid", data, v => this.surface.grid(v));
        this.surface.points(sval("drawing_order", data), sval("layout", data), sval("transformation", data), sval("style_index", data), sval("styles", data), sval("point_scale", data, 1));
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
