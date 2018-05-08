import * as acv_utils from "./utils.js";
import * as ace_surface from "./ace-surface.js";

// ----------------------------------------------------------------------

class AMW201805
{
    constructor() {
        this.last_id_ = 0;
        this.options = {
            projection_no: 0,
            point_scale: 5,
            point_info_on_hover_delay: 500,
            mouse_popup_offset: {left: 10, top: 20},
            canvas_size: {width: 500, height: 500},
            min_viewport_size: 1.0
        };

        // this.mouse_popup = new AMW_MousePopup();
    }

    new_id() {
        return "" + (++this.last_id_);
    }
}

if (window.amw201805 === undefined)
    window.amw201805 = new AMW201805();

// ----------------------------------------------------------------------

const AntigenicMapWidget_content_html = "\
<table>\
  <tr>\
    <td class='a-title'>\
      <span class='a-left'></span>\
      <span class='a-middle'></span>\
      <span class='a-right'>\
        <span class='a-right-left'></span>\
        <span class='a-burger'>&#x2630;</span>\
      </span>\
    </td>\
  </tr>\
  <tr>\
    <td>\
      <canvas></canvas>\
    </td>\
  </tr>\
</table>\
";

// ----------------------------------------------------------------------

export class AntigenicMapWidget
{
    constructor(div, data, options={}) {
        this.div = $(div);
        this.options = Object.assign({}, window.amw201805.options, options);
        acv_utils.load_css('/js/ad/map-draw/ace-view-1/ace-view.css');
        this.div.addClass("amw201805").attr("amw201805_id", window.amw201805.new_id()).append(AntigenicMapWidget_content_html);
        this.canvas = this.div.find("canvas");

        this.load_and_draw(data);

        this.surface = new ace_surface.Surface(this.canvas, {canvas: this.options.canvas_size});
        this.bind();

        // this.data = Array.isArray(data) ? data : [data];
        // this.data.unshift({point_scale: sval("point_scale", this.data, 1)});  // for interactive manipulations

        // this.surface = new Surface(this.canvas, {canvas: sval("canvas", data, this.options.canvas_size), viewport: sval("viewport", data, [0, 0, 10, 10])});
        // sval_call("border", data, v => this.surface.border(v));
        // sval_call("title", data, lines => {
        //     this.div.find(".amw201804-title-middle").append(lines[0].text);
        //     //this.div.find(".amw201804-title-left").append("LEFT");
        //     //this.div.find(".amw201804-title-right").append("\u219D");
        //     //this.div.find(".amw201804-title-burger-menu").append("\u2630");
        // });

        // this.events_ = [];
        // this.bind();
        // this.draw();
    }

    destroy() {
        this.div.empty();
    }

    bind() {
        this.canvas.on("wheel", evt => {
            if (evt.shiftKey) {
                // Shift-Wheel -> point_scale
                evt.stopPropagation();
                evt.preventDefault();
                this.point_scale(evt.originalEvent.wheelDelta > 0 ? 1.1 : (1 / 1.1));
            }
            else if (evt.altKey) {
                // Alt-Wheel -> zoom
                evt.stopPropagation();
                evt.preventDefault();
                this.zoom(this.mouse_offset(evt), evt.originalEvent.wheelDelta > 0 ? 1.05 : (1 / 1.05));
            }
        });
    }

    mouse_offset(mouse_event) {
        const border_width = parseFloat(this.canvas.css("border-width"));
        const offset_x = border_width + parseFloat(this.canvas.css("padding-left"));
        const offset_y = border_width + parseFloat(this.canvas.css("padding-top"));
        return {left: mouse_event.offsetX - offset_x, top: mouse_event.offsetY - offset_y};
    }

    load_and_draw(data) {
        if (typeof(data) === "string" && data.substr(data.length - 4) === ".ace") {
            $.getJSON(data, result => this.draw(result));
        }
        else if (typeof(data) === "object" && data.version === "acmacs-ace-v1") {
            this.draw(data);
        }
        else {
            console.error("unrecognized", data);
        }
    }

    draw(data) {
        if (data) {
            this.data = data;
            this.parameters = {point_scale: this.options.point_scale};
            this.surface.set_viewport(this.calculate_viewport());
        }
        // console.log("draw", this.data);

        this.surface.background();
        this.surface.grid();
        this.surface.border();

        this.surface.points({drawing_order: this.data.c.p.d,
                             layout: this.data.c.P[this.options.projection_no].l,
                             transformation: new ace_surface.Transformation(this.data.c.P[this.options.projection_no].t),
                             style_index: this.data.c.p.p,
                             styles: this.data.c.p.P,
                             point_scale: this.parameters.point_scale});
    }

    calculate_viewport() {
        const transformation = new ace_surface.Transformation(this.data.c.P[this.options.projection_no].t);
        const transformed_layout = transformation.transform_layout(this.data.c.P[this.options.projection_no].l);
        const corners = transformed_layout.reduce((target, coord) => {
            if (coord.length)
                return [[Math.min(target[0][0], coord[0]), Math.min(target[0][1], coord[1])], [Math.max(target[1][0], coord[0]), Math.max(target[1][1], coord[1])]];
            else
                return target;
        }, [[1e10, 1e10], [-1e10, -1e10]]);
        const size = [corners[1][0] - corners[0][0], corners[1][1] - corners[0][1]];
        const whole_size = Math.ceil(Math.max(size[0], size[1]));
        const to_whole = [(whole_size - size[0]) / 2, (whole_size - size[1]) / 2];
        return [corners[0][0] - to_whole[0], corners[0][1] - to_whole[1], whole_size, whole_size];
    }

    point_scale(multiply_by) {
        this.parameters.point_scale *= multiply_by;
        this.draw();
    }

    // value > 1 - zoom out, < 1 - zoom in
    zoom(offset, value) {
        let new_size = this.surface.viewport[2] * value;
        if (new_size < this.options.min_viewport_size)
            new_size = this.options.min_viewport_size;
        const scaled_offset = this.surface.translate_pixel_offset(offset);
        const prop = {left: (scaled_offset.left - this.surface.viewport[0]) / this.surface.viewport[2], top: (scaled_offset.top - this.surface.viewport[1]) / this.surface.viewport[2]};
        this.surface.set_viewport([scaled_offset.left - new_size * prop.left, scaled_offset.top - new_size * prop.top, new_size, new_size]);
        this.draw();
    }
}

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End: