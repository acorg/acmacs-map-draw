import * as acv_utils from "./utils.js";
import * as ace_surface from "./ace-surface.js";
import * as acv_toolkit from "./toolkit.js";

// ----------------------------------------------------------------------

class AMW201805
{
    constructor() {
        this.last_id_ = 0;
        this.options = {
            projection_no: 0,
            drawing_mode: "best-projection",
            point_scale: 5,
            point_info_on_hover_delay: 500,
            mouse_popup_offset: {left: 10, top: 20},
            canvas_size: {width: 0, height: 0},
            min_viewport_size: 1.0,
            show_as_background: {fill: "#E0E0E0", outline: "#E0E0E0"},
            show_as_background_shade: 0.8
        };
    }

    new_id() {
        return "" + (++this.last_id_);
    }
}

if (window.amw201805 === undefined)
    window.amw201805 = new AMW201805();

// ----------------------------------------------------------------------

const BurgerMenu_html = "\
<ul class='a-level-0'>\
  <li class='a-disabled'><a href='search'>Search</a></li>\
  <li><a href='#'>Coloring</a><span class='a-right-arrow'>&#9654;</span>\
    <ul class='a-level-1'>\
      <li class='a-disabled' name='color-by-clade'><a href='color-by-clade'>by Clade</a></li>\
      <li class='a-disabled' name='color-by-continent'><a href='color-by-continent'>by Geography</a></li>\
      <li                    name='color-by-default'><a href='color-by-default'>reset to default</a></li>\
    </ul>\
  </li>\
  <li><a href='#'>Show</a><span class='a-right-arrow'>&#9654;</span>\
    <ul class='a-level-1'>\
      <li class='a-disabled' name='best-projection'><a href='best-projection'>Best projection</a></li>\
      <li class='a-separator'></li>\
      <li class='a-disabled' name='time-series'><a href='time-series'>Time series</a></li>\
      <li class='a-disabled' name='time-series-shade'><a href='time-series-shade'>Time series (shade)</a></li>\
      <li class='a-disabled' name='time-series-grey'><a href='time-series-grey'>Time series (grey)</a></li>\
      <li class='a-separator'></li>\
      <li class='a-disabled' name='table-series'><a href='table-series'>Table series</a></li>\
      <li class='a-disabled' name='table-series-shade'><a href='table-series-shade'>Table series (shade)</a></li>\
      <!-- <li class='a-disabled' name='clade-series'><a href='clade-series'>Clade series</a></li> -->\
    </ul>\
  </li>\
  <li><a href='raw'>Raw</a></li>\
  <li class='a-separator'></li>\
  <li><a href='help'>Help</a></li>\
</ul>\
";

class BurgerMenu extends acv_toolkit.Modal
{
    constructor(parent) {
        super(BurgerMenu_html);
        this.parent = parent;
        this.classes("a-window-shadow");
        this.find("ul").addClass("a-window-shadow");
        this.enable_features();
        this.bind();
    }

    bind() {
        // this.find("a[href='raw']").on("click", evt => this.forward(evt, () => acv_toolkit.movable_window_with_json(this.parent.data, evt.currentTarget, "map view raw data")));
        this.find("a[href='raw']").on("click", evt => this.forward(evt, () => {
            console.log("amw raw data", this.parent.data);
            alert("Please see raw data in the console");
        }));

        this.find("a[href='search']").on("click", evt => this.forward(evt, () => console.log("search")));
        this.find("a[href='color-by-clade']").on("click", evt => this.forward(evt, () => this.parent.set_coloring("clade")));
        this.find("a[href='color-by-continent']").on("click", evt => this.forward(evt, () => this.parent.set_coloring("continent")));
        this.find("a[href='color-by-default']").on("click", evt => this.forward(evt, () => this.parent.set_coloring("default")));
        this.find("a[href='best-projection']").on("click", evt => this.forward(evt, () => this.parent.set_drawing_mode("best-projection")));
        this.find("a[href='time-series']").on("click", evt => this.forward(evt, () => this.parent.set_drawing_mode("time-series")));
        this.find("a[href='time-series-shade']").on("click", evt => this.forward(evt, () => this.parent.set_drawing_mode("time-series-shade")));
        this.find("a[href='time-series-grey']").on("click", evt => this.forward(evt, () => this.parent.set_drawing_mode("time-series-grey")));
        this.find("a[href='table-series']").on("click", evt => this.forward(evt, () => this.parent.set_drawing_mode("table-series")));
        this.find("a[href='table-series-shade']").on("click", evt => this.forward(evt, () => this.parent.set_drawing_mode("table-series-shade")));
        this.find("a[href='clade-series']").on("click", evt => this.forward(evt, () => this.parent.set_drawing_mode("series-clade")));
        this.find("a[href='help']").on("click", evt => this.forward(evt, () => console.log("help")));

        this.find("li.a-disabled a").off("click").on("click", evt => this.forward(evt));
    }

    enable_features() {
        for (let feature in this.parent.features) {
            if (this.parent.features[feature]) {
                this.find(`li[name='${feature}']`).removeClass('a-disabled');
            }
        }
    }

    forward(evt, callback) {
        evt.stopPropagation();
        evt.preventDefault();
        if (callback) {
            callback();
            this.destroy();
        }
    }
}

// ----------------------------------------------------------------------

const AntigenicMapWidget_left_arrow = "&#x21E6;"; // "&#x27F8;";
const AntigenicMapWidget_right_arrow = "&#x21E8;"; // "&#x27F9;";

const AntigenicMapWidget_content_html = `\
<table>\
  <tr>\
    <td class='a-title'>\
      <div class='a-left'>\
        <div class='a-arrow a-left-arrow a-unselectable'>${AntigenicMapWidget_left_arrow}</div>\
      </div>\
      <div class='a-title-title'></div>\
      <div class='a-right'>\
        <div class='a-arrow a-right-arrow a-unselectable'>${AntigenicMapWidget_right_arrow}</div>\
        <div class='a-burger'>&#x2630;</div>\
      </div>\
    </td>\
  </tr>\
  <tr>\
    <td class='a-canvas'>\
      <canvas></canvas>\
      <div class='a-window-resizer'></div>\
    </td>\
  </tr>\
</table>\
`;

// ----------------------------------------------------------------------

export class AntigenicMapWidget
{
    constructor(div, data, options={}) { // drawing_mode: "table-series"}) {
        this.div = $(div);
        this.options = Object.assign({}, window.amw201805.options, options);
        acv_utils.load_css('/js/ad/map-draw/ace-view-1/ace-view.css');
        this.div.addClass("amw201805").attr("amw201805_id", window.amw201805.new_id()).append(AntigenicMapWidget_content_html);
        this.canvas = this.div.find("canvas");
        if (!this.options.canvas_size || !this.options.canvas_size.width || !this.options.canvas_size.height) {
            const w_size = Math.max(Math.min($(window).width(), $(window).height()) - 40, 200);
            this.options.canvas_size = {width: w_size, height: w_size};
        }
        this.surface = new ace_surface.Surface(this.canvas, {canvas: this.options.canvas_size});
        this.load_and_draw(data);
        this.bind();
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

        this.canvas.on("mousedown", evt => {
            if (evt.altKey) {   // Alt-Drag - pan
                let mousedown_pos = {left: evt.clientX, top: evt.clientY};
                document.onmouseup = () => { document.onmouseup = document.onmousemove = null; };
                document.onmousemove = evt => {
                    this.surface.move_viewport(mousedown_pos.left - evt.clientX, mousedown_pos.top - evt.clientY);
                    mousedown_pos = {left: evt.clientX, top: evt.clientY};
                    this.draw();
                };
            }
        });

        this.div.find(".a-burger").on("click", evt => new BurgerMenu(this).show($(evt.target)));

        this.div.find(".a-window-resizer").on("mousedown", evt => {
            let mouse_pos = {left: evt.clientX, top: evt.clientY};
            document.onmouseup = () => {
                document.onmouseup = document.onmousemove = null;
                // this.div.find(".a-content").removeClass("a-unselectable");
                $("body").removeClass("a-unselectable");
            };
            document.onmousemove = evt2 => {
                // this.div.find(".a-content").addClass("a-unselectable");
                $("body").addClass("a-unselectable");
                this.resize({left: mouse_pos.left - evt2.clientX, top: mouse_pos.top - evt2.clientY});
                mouse_pos = {left: evt2.clientX, top: evt2.clientY};
            };
        });

        this.set_point_info_on_hover();
    }

    mouse_offset(mouse_event) {
        const border_width = parseFloat(this.canvas.css("border-width"));
        const offset_x = border_width + parseFloat(this.canvas.css("padding-left"));
        const offset_y = border_width + parseFloat(this.canvas.css("padding-top"));
        return {left: mouse_event.offsetX - offset_x, top: mouse_event.offsetY - offset_y};
    }

    load_and_draw(data) {
        if (typeof(data) === "string" && RegExp("(\\.ace|\\?acv=ace)$").test(data)) {
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
            this.set_features();
            this.parameters = {point_scale: this.options.point_scale};
            this.surface.set_viewport(this.calculate_viewport());
            this.make_point_info_labels();
            this.set_drawing_mode(this.options.drawing_mode);
        }
        else {
            this.surface.background();
            this.surface.grid();
            this.surface.border();
            this.drawing_mode.draw();
            this.title();
            this.resize_title();
        }
    }

    set_drawing_mode(mode) {
        this.drawing_mode = select_drawing_mode(mode, this);
        this.draw();
    }

    set_coloring(coloring) {
        this.coloring = select_coloring(coloring, this);
        this.draw();
    }

    set_features() {
        this.features = {};
        const chart = this.data.c;
        if (chart.P && chart.P.length > 0)
            this.features["best-projection"] = true;
        if (chart.t.L && chart.t.L.length > 1)
            this.features["table-series"] = this.features["table-series-shade"] = true;
        if (chart.a.reduce((with_dates, antigen) => with_dates + (antigen.D ? 1 : 0), 0) > (chart.a.length * 0.25))
            this.features["time-series"] = this.features["time-series-shade"] = this.features["time-series-grey"] = true;
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

    title() {
        this.title_element().empty().append(this.drawing_mode.title());
        this.popup_on_hovering_title(this.drawing_mode.title_box());
    }

    resize(diff) {
        this.surface.resize(diff);
        this.draw();
    }

    title_element() {
        return this.div.find(".a-title-title");
    }

    resize_title() {
        const title_element = this.title_element();
        const title_left = this.div.find(".a-title > .a-left");
        const title_left_width = title_left.is(":visible") ? title_left.outerWidth(true) : 0;
        const title_right = this.div.find(".a-title > .a-right");
        const title_right_width = title_right.is(":visible") ? title_right.outerWidth(true) : 0;
        title_element.css("width", this.surface.width() - title_left_width - title_right_width - (title_element.outerWidth(true) - title_element.width()));
    }

    show_title_arrows(left_callback, right_callback) {
        const left_arrow = this.div.find(".a-title .a-left-arrow");
        left_arrow.off("click");
        if (left_callback)
            left_arrow.show().on("click", left_callback);
        else
            left_arrow.hide();
        const right_arrow = this.div.find(".a-title .a-right-arrow");
        right_arrow.off("click");
        if (right_callback)
            right_arrow.show().on("click", right_callback);
        else
            right_arrow.hide();
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

    set_point_info_on_hover() {

        const mouse_offset = (mouse_event) => {
            const border_width = parseFloat(this.canvas.css("border-width"));
            const offset_x = border_width + parseFloat(this.canvas.css("padding-left"));
            const offset_y = border_width + parseFloat(this.canvas.css("padding-top"));
            return {left: mouse_event.offsetX - offset_x, top: mouse_event.offsetY - offset_y};
        };

        const point_info_on_hover = (offset) => {
            const points = this.surface.find_points_at_pixel_offset(offset);
            if (points.length) {
                const names = points.map(point_no => this.point_info_labels_[point_no]);
                acv_toolkit.mouse_popup_show($("<ul class='point-info-on-hover'></ul>").append(names.map(text => "<li>" + text + "</li>").join("")), this.canvas, {left: offset.left + this.options.mouse_popup_offset.left, top: offset.top + this.options.mouse_popup_offset.top});
            }
            else {
                acv_toolkit.mouse_popup_hide();
            }
        };

        this.canvas.on("mousemove", evt => {
            window.clearTimeout(this.mouse_popup_timeout_id);
            const offset = mouse_offset(evt);
            this.mouse_popup_timeout_id = window.setTimeout(() => point_info_on_hover(offset), this.options.point_info_on_hover_delay);
        });
        this.canvas.on("mouseleave", () => window.clearTimeout(this.mouse_popup_timeout_id));
    }

    popup_on_hovering_title(content) {
        if (content) {
            const title = this.title_element();
            let popup_events = false;
            const hide_popup = () => {
                acv_toolkit.mouse_popup_hide().off("mouseenter").off("mouseleave");
                popup_events = false;
            };
            const mouse_leave = () => {
                window.clearTimeout(this.mouse_popup_timeout_id);
                this.mouse_popup_timeout_id = window.setTimeout(hide_popup, this.options.point_info_on_hover_delay);
            };
            title.on("mouseenter", evt => {
                window.clearTimeout(this.mouse_popup_timeout_id);
                this.mouse_popup_timeout_id = window.setTimeout(() => {
                    const popup_element = acv_toolkit.mouse_popup_show(content, title, {left:0, top: title.outerHeight()});
                    if (!popup_events) {
                        popup_element.on("mouseenter", () => window.clearTimeout(this.mouse_popup_timeout_id));
                        popup_element.on("mouseleave", mouse_leave);
                        popup_events = true;
                    }
                }, this.options.point_info_on_hover_delay);
            });
            title.on("mouseleave", mouse_leave);
        }
    }

    make_point_info_labels() {
        this.point_info_labels_ = [];
        for (let antigen of this.data.c.a)
            this.point_info_labels_.push(acv_utils.join_collapse([antigen.N, antigen.R].concat(antigen.a, antigen.P, antigen.D && "[" + antigen.D + "]")));
        for (let serum of this.data.c.s)
            this.point_info_labels_.push(acv_utils.join_collapse([serum.N, serum.R].concat(serum.a, serum.I)));
    }
}

// ----------------------------------------------------------------------

class DrawingMode_Base
{
    constructor(widget) {
        this.widget = widget;
        widget.show_title_arrows(null, null);
    }

    title_box() {
        const chart = this.widget.data.c;
        const projection_no = this.widget.options.projection_no;
        let title_box = $(`<ul class='a-title-mouse-popup'><li>Antigens: ${chart.a.length}</li><li>Sera: ${chart.s.length}</li></ul>`);
        if (chart.i.S) {
            const sources = chart.i.S;
            const first = sources[0], last = sources[sources.length - 1];
            title_box.prepend(`<li>${first.v || ""} ${first.V || ""} ${first.A || ""} ${first.r || ""}</li><li>Lab: ${first.l}</li>`);
            title_box.append(`<li>Tables: ${sources.length}</li>`);
            title_box.append(`<li>Dates: ${first.D} - ${last.D}</li>`);
        }
        else {
            const first = chart.i;
            title_box.prepend(`<li>${first.v} ${first.V} ${first.A} ${first.r || ""}</li><li>Lab: ${first.l}</li>`);
            title_box.append(`<li>Date: ${first.D}</li>`);
        }
        title_box.append(`<li>Projections: ${chart.P.length}</li>`);
        if (chart.t.L)
            title_box.append(`<li>Layers: ${chart.t.L.length}</li>`);
        return title_box;
    }
}

// ----------------------------------------------------------------------

class DrawingMode_Best_Projection extends DrawingMode_Base
{
    draw() {
        const chart = this.widget.data.c;
        const projection_no = this.widget.options.projection_no;
        this.widget.surface.points({drawing_order: chart.p.d,
                                    layout: chart.P[projection_no].l,
                                    transformation: new ace_surface.Transformation(chart.P[projection_no].t),
                                    style_index: chart.p.p,
                                    styles: chart.p.P,
                                    point_scale: this.widget.parameters.point_scale});
    }

    title() {
        const chart = this.widget.data.c;
        const projection_no = this.widget.options.projection_no;
        let stress = chart.P[projection_no].s;
        stress = stress ? stress.toFixed(4) : "";
        let mcb = chart.P[projection_no].m;
        mcb = mcb ? ">=" + mcb : ">=none";
        const prefix = acv_utils.join_collapse([stress, `A:${chart.a.length} S:${chart.s.length}`]);
        let title;
        if (chart.i.N) {
            title = acv_utils.join_collapse([prefix, chart.i.N]);
        }
        else if (chart.i.S) {
            const sources = chart.i.S;
            const first = sources[0], last = sources[sources.length - 1];
            title = acv_utils.join_collapse([prefix, first.l, first.V, first.A, first.D + "-" + last.D, mcb, `(${sources.length} tables)`]);
        }
        else {
            const first = chart.i;
            title = acv_utils.join_collapse([prefix, first.l, first.V, first.A, first.D, mcb]);
        }
        return title;
    }
}

// ----------------------------------------------------------------------

class DrawingMode_Series extends DrawingMode_Base
{
    constructor(widget) {
        super(widget);
        this.make_pages();
        this.set_page(this.pages.length - 1);
    }

    draw() {
        const chart = this.widget.data.c;
        const projection_no = this.widget.options.projection_no;
        if (this.drawing_order_background) {
            this.widget.surface.points({drawing_order: this.drawing_order_background,
                                        layout: chart.P[projection_no].l,
                                        transformation: new ace_surface.Transformation(chart.P[projection_no].t),
                                        style_index: chart.p.p,
                                        styles: chart.p.P,
                                        point_scale: this.widget.parameters.point_scale,
                                        show_as_background: this.show_as_background()
                                       });
        }
        this.widget.surface.points({drawing_order: this.drawing_order,
                                    layout: chart.P[projection_no].l,
                                    transformation: new ace_surface.Transformation(chart.P[projection_no].t),
                                    style_index: chart.p.p,
                                    styles: chart.p.P,
                                    point_scale: this.widget.parameters.point_scale});
    }

    title() {
        return this.pages[this.page_no];
    }

    set_page(page_no, redraw) {
        if (page_no >= 0 && page_no < this.pages.length) {
            this.page_no = page_no;
            this.make_drawing_order();
            this.widget.show_title_arrows(this.page_no > 0 ? () => this.set_page(this.page_no - 1, true) : null, this.page_no < (this.pages.length - 1) ? () => this.set_page(this.page_no + 1, true) : null);
            if (redraw)
                this.widget.draw();
        }
    }

    show_as_background() {
        return this.widget.options.show_as_background;
    }
}

// ----------------------------------------------------------------------

class DrawingMode_TimeSeries extends DrawingMode_Series
{
    make_pages() {
        let months = new Set();
        for (let antigen of this.widget.data.c.a) {
            const month = this.antigen_month(antigen);
            if (month)
                months.add(month);
        }
        this.pages = [...months].sort();
    }

    make_drawing_order() {
        const page_month = this.pages[this.page_no];
        const in_page = antigen => this.antigen_month(antigen) === page_month;
        const antigens = this.widget.data.c.a;
        this.drawing_order = [];
        this.drawing_order_background = undefined;
        for (let point_no of this.widget.data.c.p.d) {
            if (point_no >= antigens.length || (antigens[point_no].S && antigens[point_no].S.indexOf("R") >= 0 && !in_page(antigens[point_no])))
                this.drawing_order.push(point_no);
        }
        for (let point_no of this.widget.data.c.p.d) {
            if (point_no < antigens.length && in_page(antigens[point_no]))
                this.drawing_order.push(point_no);
        }
    }

    antigen_month(antigen) {
        return antigen.D && antigen.D.substr(0, 7);
    }
}

// ----------------------------------------------------------------------

class DrawingMode_TimeSeriesGrey extends DrawingMode_TimeSeries
{
    make_drawing_order() {
        const page_month = this.pages[this.page_no];
        const in_page = antigen => this.antigen_month(antigen) === page_month;
        const antigens = this.widget.data.c.a;
        this.drawing_order = [];
        this.drawing_order_background = [];
        for (let point_no of this.widget.data.c.p.d) {
            if (point_no < antigens.length && in_page(antigens[point_no]))
                this.drawing_order.push(point_no);
            else
                this.drawing_order_background.push(point_no);
        }
    }
}

// ----------------------------------------------------------------------

class DrawingMode_TimeSeriesShade extends DrawingMode_TimeSeriesGrey
{
    show_as_background() {
        return {shade: this.widget.options.show_as_background_shade};
    }
}

// ----------------------------------------------------------------------

class DrawingMode_TableSeries extends DrawingMode_Series
{
    make_pages() {
        const number_of_layers = this.widget.data.c.t.L.length;
        const make_name = (source, index) => {
            if (source && source.D)
                return `${source.D} (${index + 1}/${number_of_layers})`;
            else
                return `Table ${index + 1}/${number_of_layers}`;
        };
        if (this.widget.data.c.i.S)
            this.pages = this.widget.data.c.i.S.map(make_name);
        else
            this.pages = this.widget.data.c.t.L.map(make_name);
    }

    make_drawing_order() {
        const antigens = this.widget.data.c.a;
        const layer = this.widget.data.c.t.L[this.page_no];
        const antigen_in_layer = antigen_no => Object.keys(layer[antigen_no]).length > 0;
        const serum_in_layer = serum_no => {
            const serum_no_s = "" + serum_no;
            return layer.some(entry => !!entry[serum_no_s]);
        };
        const point_in_layer = point_no => point_no < antigens.length ? antigen_in_layer(point_no) : serum_in_layer(point_no - antigens.length);
        this.drawing_order = this.widget.data.c.p.d.filter(point_in_layer);
    }

}

// ----------------------------------------------------------------------

class DrawingMode_TableSeriesShade extends DrawingMode_TableSeries
{
    make_drawing_order() {
        const antigens = this.widget.data.c.a;
        const layer = this.widget.data.c.t.L[this.page_no];
        const antigen_in_layer = antigen_no => Object.keys(layer[antigen_no]).length > 0;
        const serum_in_layer = serum_no => {
            const serum_no_s = "" + serum_no;
            return layer.some(entry => !!entry[serum_no_s]);
        };
        const point_in_layer = point_no => point_no < antigens.length ? antigen_in_layer(point_no) : serum_in_layer(point_no - antigens.length);
        this.drawing_order = this.widget.data.c.p.d.filter(point_in_layer);
        this.drawing_order_background = this.widget.data.c.p.d.filter(point_no => !point_in_layer(point_no));
    }

    show_as_background() {
        return {shade: this.widget.options.show_as_background_shade};
    }
}

// ----------------------------------------------------------------------

const drawing_mode_selector_data = {
    "best-projection": DrawingMode_Best_Projection,
    "time-series": DrawingMode_TimeSeries,
    "time-series-shade": DrawingMode_TimeSeriesShade,
    "time-series-grey": DrawingMode_TimeSeriesGrey,
    "table-series": DrawingMode_TableSeries,
    "table-series-shade": DrawingMode_TableSeriesShade,
    null: DrawingMode_Best_Projection
};

function select_drawing_mode(mode, widget) {
    return new (drawing_mode_selector_data[mode] || drawing_mode_selector_data[null])(widget);
}

// ----------------------------------------------------------------------

class Coloring_Base
{
    constructor(widget) {
        this.widget = widget;
    }
}

// ----------------------------------------------------------------------

class Coloring_Default extends Coloring_Base
{
}

// ----------------------------------------------------------------------

class Coloring_Continent extends Coloring_Base
{
}

// ----------------------------------------------------------------------

class Coloring_Clade extends Coloring_Base
{
}

// ----------------------------------------------------------------------

const coloring_selector_data = {
    "clade": Coloring_Clade,
    "continent": Coloring_Continent,
    "default": Coloring_Default,
    null: Coloring_Default
};

function select_coloring(coloring, widget) {
    return new (coloring_selector_data[coloring] || coloring_selector_data[null])(widget);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
