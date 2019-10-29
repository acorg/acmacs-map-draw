import * as acv_utils from "./utils.js";
import * as ace_surface from "./ace-surface.js";
import * as acv_toolkit from "./toolkit.js";
import * as acv_point_style from "./point-style.js";

// ----------------------------------------------------------------------

class AMW201805
{
    constructor() {
        this.last_id_ = 0;
        this.options = {
            projection_no: 0,
            view_mode: {mode: "projection"},
            coloring: "default",
            point_scale: 5,
            point_info_on_hover_delay: 500,
            mouse_popup_offset: {left: 10, top: 20},
            canvas_size: {width: 0, height: 0},
            min_viewport_size: 1.0,
            show_as_background: {fill: acv_toolkit.sLIGHTGREY, outline: acv_toolkit.sLIGHTGREY},
            show_as_background_shade: 0.8,
            title_fields: ["stress", "antigens", "sera", "name", "lab", "virus_type", "assay", "date", "min_col_basis", "tables"],
            on_data_load_failure: uri => console.error("failed to load antigenic map data from ", uri)
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
  <!-- <li class='a-disabled'><a href='search'>Search</a></li> -->\
  <li><a href='view'>View ...</a></li>\
  <li><a href='#'>Download</a><span class='a-right-arrow'>&#9654;</span>\
    <ul class='a-level-1'>\
      <li class='a-disabled' name='download_pdf'><a href='download_pdf'>PDF</a></li>\
      <li class='a-separator'></li>\
      <li class='a-disabled' name='download_ace'><a href='download_ace'>ace</a></li>\
      <li class='a-disabled' name='download_save'><a href='download_save'>Lispmds Save</a></li>\
      <li class='a-separator'></li>\
      <li class='a-disabled' name='download_layout_plain'><a href='download_layout_plain'>Layout (plain text)</a></li>\
      <li class='a-disabled' name='download_layout_csv'><a href='download_layout_csv'>Layout (csv)</a></li>\
      <li class='a-separator'></li>\
      <li class='a-disabled' name='download_table_map_distances_plain'><a href='download_table_map_distances_plain'>Table vs. Map Distances (plain text)</a></li>\
      <li class='a-disabled' name='download_table_map_distances_csv'><a href='download_table_map_distances_csv'>Table vs. Map Distances (csv)</a></li>\
      <li class='a-disabled' name='download_error_lines'><a href='download_error_lines'>Error lines (csv)</a></li>\
      <li class='a-disabled' name='download_distances_between_all_points_plain'><a href='download_distances_between_all_points_plain'>Distances Between All Points (plain text)</a></li>\
      <li class='a-disabled' name='download_distances_between_all_points_csv'><a href='download_distances_between_all_points_csv'>Distances Between All Points (csv)</a></li>\
    </ul>\
  </li>\
  <li><a href='table'>Table</a></li>\
  <li><a href='raw'>Raw</a></li>\
  <li class='a-separator'></li>\
  <li><a href='help'>Help</a></li>\
</ul>\
";

const API_Features = [
    "download_pdf", "download_ace", "download_save",
    "download_layout_plain", "download_layout_csv",
    "download_table_map_distances_plain", "download_table_map_distances_csv", "download_error_lines",
    "download_distances_between_all_points_plain", "download_distances_between_all_points_csv"
];

class BurgerMenu extends acv_toolkit.Old_Modal
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
        const destroy = () => this.destroy();

        this.find("a[href='raw']").on("click", evt => acv_utils.forward_event(evt, () => {
            console.log("amw raw data", this.parent.data);
            // alert("Please see raw data in the console");
        }, destroy));

        this.find("a[href='search']").on("click", evt => acv_utils.forward_event(evt, () => console.log("search"), destroy));
        this.find("a[href='table']").on("click", evt => acv_utils.forward_event(evt, () => this.parent.show_table(evt.currentTarget), destroy));
        this.find("a[href='view']").on("click", evt => acv_utils.forward_event(evt, () => this.parent.show_view_dialog(evt.currentTarget), destroy));
        this.find("a[href='help']").on("click", evt => acv_utils.forward_event(evt, () => this.parent.show_help(evt.currentTarget), destroy));

        API_Features.forEach(api_feature => {
            this.find(`a[href='${api_feature}']`).on("click", evt => acv_utils.forward_event(evt, () => this.parent.external_api(api_feature), destroy));
        });

        this.find("li.a-disabled a").off("click").on("click", evt => acv_utils.forward_event(evt, destroy));
    }

    enable_features() {
        for (let feature in this.parent.features) {
            if (this.parent.features[feature]) {
                this.find(`li[name='${feature}']`).removeClass('a-disabled');
            }
        }
    }

}

// ----------------------------------------------------------------------

const AntigenicMapWidget_help_html = "\
<div class='a-help'>\
  <h3>Mouse Wheel and Drag</h3>\
  <ul>\
    <li>Change point size - Shift-Wheel</li>\
    <li>Zoom - Alt/Option-Wheel</li>\
    <li>Move map - Alt/Option-Drag</li>\
  </ul>\
  <h3>Table</h3>\
  <p>Table view can be opened via menu. If table view is on the screen,\
     hovering point(s) on map leads to highlighting corresponding\
     row/column in the table, table view is auto-scrolled.</p>\
</div>\
";

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
      <div class='a-title-title'><span></span></div>\
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
      <div class='a-loading-message'>Loading data, please wait ...</div>\
    </td>\
  </tr>\
</table>\
`;

// ----------------------------------------------------------------------

export class AntigenicMapWidget
{
    constructor(div, data, options={}) { // options: {view_mode: {mode: "table-series"}, coloring: "default", title_fields: [], api: object_providing_external_api}
        this.div = $(div);
        this.options = Object.assign({}, window.amw201805.options, options);
        acv_utils.load_css('/js/ad/map-draw/ace-view/201805/ace-view.css');
        this.div.addClass("amw201805").attr("amw201805_id", window.amw201805.new_id()).append(AntigenicMapWidget_content_html);
        this.canvas = this.div.find("canvas");
        if (!this.options.canvas_size || !this.options.canvas_size.width || !this.options.canvas_size.height) {
            const w_size = Math.max(Math.min($(window).width() - 100, $(window).height()) - 60, 200);
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
        this.canvas.on("wheel DOMMouseScroll", evt => {
            if (evt.shiftKey) {
                // Shift-Wheel -> point_scale
                evt.stopPropagation();
                evt.preventDefault();
                const scroll = evt.originalEvent.deltaX != 0 ? evt.originalEvent.deltaX : evt.originalEvent.deltaY; // depends if mouse or touchpad used
                this.point_scale(scroll > 0 ? 1.1 : (1 / 1.1));
                window.EE = evt;
            }
            else if (evt.altKey) {
                // Alt-Wheel -> zoom
                evt.stopPropagation();
                evt.preventDefault();
                const scroll = evt.originalEvent.deltaX != 0 ? evt.originalEvent.deltaX : evt.originalEvent.deltaY; // depends if mouse or touchpad used
                this.zoom(this.mouse_offset(evt), scroll > 0 ? 1.05 : (1 / 1.05));
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
            this.div.find(".a-loading-message").css({top: parseFloat(this.canvas.css("padding-top")) + this.canvas.height() / 2, width: this.canvas.width()});
            $.getJSON(data)
                .done(result => this.draw(result))
                .fail(deffered => this.options.on_data_load_failure && this.options.on_data_load_failure({source: data, statusText: deffered.statusText}));
        }
        else if (typeof(data) === "function" && data.constructor.name === 'AsyncFunction') {
            this.div.find(".a-loading-message").css({top: parseFloat(this.canvas.css("padding-top")) + this.canvas.height() / 2, width: this.canvas.width()});
            data().then(result => this.draw(result));
        }
        else if (typeof(data) === "object" && (data.version || data["  version"]) === "acmacs-ace-v1") {
            this.draw(data);
        }
        else {
            console.error("unrecognized", data);
        }
    }

    draw(data) {
        if (data) {
            this.div.find(".a-loading-message").remove();
            this.data = data;
            this.set_features();
            this.set_plot_spec();
            this.parameters = {point_scale: this.options.point_scale};
            this.surface.set_viewport(this.calculate_viewport());
            this.set_coloring(this.options.coloring, false);
            this.set_view_mode(this.options.view_mode);
        }
        else {
            acv_toolkit.mouse_popup_hide();
            this.surface.background();
            this.surface.grid();
            this.surface.border();
            this.view_mode.draw();
            this.update_view_dialog();
            this.title();
            this.resize_title();
        }
    }

    set_view_mode(args={}) {
        const mode = args.mode || (this.view_mode && this.view_mode.mode()) || "projection";
        const shading = args.shading || (this.view_mode && this.view_mode.shading()) || "shade";
        const period = args.period || (this.view_mode && this.view_mode.period());
        const page = args.page || (this.view_mode && this.view_mode.current_page());
        const fixed_args = Object.assign({}, args, {period: period, page: page});
        switch (mode) {
        case "time-series":
            this.view_mode = new DrawingMode_TimeSeries(this, fixed_args);
            break;
        case "table-series":
            this.view_mode = new DrawingMode_TableSeries(this, fixed_args);
            break;
        case "group-series":
            if (this.view_mode instanceof DrawingMode_GroupSeries) {
                this.view_mode.set(args);
                this.view_mode.make_drawing_order();
            }
            else
                this.view_mode = new DrawingMode_GroupSeries(this, fixed_args);
            break;
        case "selection":
            this.view_mode = new DrawingMode_Selection(this, fixed_args);
            break;
        case "projection":
        default:
            this.view_mode = new DrawingMode_Best_Projection(this, fixed_args);
            break;
        }
        if (args.projection_no !== undefined)
            this.view_mode.projection_no_ = args.projection_no;
        this.draw();
    }

    set_coloring(coloring, draw=true) {
        this.coloring = select_coloring(coloring, this);
        if (draw)
            this.draw();
    }

    set_features() {
        this.features = {};
        const chart = this.data.c;
        if (chart.P && chart.P.length > 0)
            this.features["best-projection"] = true;
        if (chart.t.L && chart.t.L.length > 1)
            this.features["table-series"] = true;
        if (chart.a.reduce((with_dates, antigen) => with_dates + (antigen.D ? 1 : 0), 0) > (chart.a.length * 0.25))
            this.features["time-series"] = true;
        if (chart.a.reduce((with_clades, antigen) => with_clades + (antigen.c && antigen.c.length > 0 ? 1 : 0), 0) > 0)
            this.features["clades"] = true;
        if (chart.a.reduce((with_continents, antigen) => with_continents + (antigen.C ? 1 : 0), 0) > 0)
            this.features["continents"] = true;
        if (this.options.api) {
            API_Features.filter(feature => !!this.options.api[feature]).forEach(feature => { this.features[feature] = true; });
        }
    }

    set_plot_spec() {
        const chart = this.data.c;
        if (!chart.p || !chart.p.p || !chart.p.P) {
            chart.p = {P: [{s: 1.5}, {F: "#00FF00"}, {s: 1.5, S: "B"}], p: [], d: []};
            chart.a.forEach((antigen, antigen_no) => { if (!antigen.S || antigen.S.indexOf("R") < 0) { chart.p.d.push(antigen_no); chart.p.p[antigen_no] = 1; } });
            chart.s.forEach((serum, serum_no) => { const point_no = serum_no + chart.a.length; chart.p.d.push(point_no); chart.p.p[point_no] = 2; });
            chart.a.forEach((antigen, antigen_no) => { if (antigen.S && antigen.S.indexOf("R") >= 0) { chart.p.d.push(antigen_no); chart.p.p[antigen_no] = 0; } });
        }
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
        this.title_element().find("span").empty().append(this.view_mode.title({title_fields: this.options.title_fields}));
        this.popup_on_hovering_title(this.view_mode.title_box());
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

        const mouse_offset = mouse_event => {
            const border_width = Number.parseFloat(this.canvas.css("border-width") || "0");
            const offset_x = border_width + parseFloat(this.canvas.css("padding-left") || "0");
            const offset_y = border_width + parseFloat(this.canvas.css("padding-top") || "0");
            return {left: mouse_event.offsetX - offset_x, top: mouse_event.offsetY - offset_y};
        };

        const point_info_on_hover = offset => {
            const points = this.surface.find_points_at_pixel_offset(offset);
            if (points.length) {
                const chart = this.data.c;
                const full_name = point_no => point_no < chart.a.length ? acv_utils.ace_antigen_full_name(chart.a[point_no], {escape: true}) : acv_utils.ace_serum_full_name(chart.s[point_no - chart.a.length], {escape: true});
                const point_entries = points.map(point_no => { return {name: full_name(point_no), no: point_no}; });
                const popup = acv_toolkit.mouse_popup_show($("<ul class='point-info-on-hover'></ul>").append(point_entries.map(make_point_name_row).join("")), this.canvas, {left: offset.left + this.options.mouse_popup_offset.left, top: offset.top + this.options.mouse_popup_offset.top});
                if (this.options.point_on_click) {
                    popup.find("a").on("click", evt => {
                        acv_utils.forward_event(evt, evt => show_antigen_serum_info_from_hidb($(evt.target), this.data.c, this.canvas, this.options.point_on_click));
                        window.setTimeout(acv_toolkit.mouse_popup_hide, this.options.point_info_on_hover_delay);
                    });
                }
            }
            else {
                acv_toolkit.mouse_popup_hide();
            }
            const table = this.find_table();
            if (table)
                table.show_points(points);
        };

        const make_point_name_row = point_entry => {
            if (this.options.point_on_click)
                return `<li><a href="show-info-on-this-name" point_no="${point_entry.no}" point_name="${point_entry.name}">${point_entry.name}</a></li>`;
            else
                return `<li>${point_entry.name}</li>`;
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
            const title = this.title_element().find("span");
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

    show_help(parent) {
        new acv_toolkit.MovableWindow({title: "Help", content: AntigenicMapWidget_help_html, parent: parent, id: "AntigenicMapWidget_help"});
    }

    dialog_id(prefix) {
        return prefix + "-" + this.div.attr("amw201805_id");
    }

    table_id() {
        return this.dialog_id("antigenic-table");
    }

    show_table(parent) {
        if (this.data)
            return new AntigenicTable({populate: true, widget: this, parent: $(parent), chart: this.data.c, id: this.table_id()});
        else
            return null;
    }

    find_table(parent) {
        if (this.data)
            return new AntigenicTable({find: true, widget: this, chart: this.data.c, id: this.table_id()});
        else
            return null;
    }

    show_view_dialog(parent) {
        if (this.data) {
            if (!this.view_dialog_) {
                this.view_dialog_ = new ViewDialog({
                    widget: this,
                    parent: $(parent),
                    chart: this.data.c,
                    id: this.dialog_id("view"),
                    on_destroy: () => delete this.view_dialog_
                });
            }
            else {
                this.view_dialog_.find_me();
            }
            return this.view_dialog_;
        }
        else
            return null;
    }

    external_api(api_feature) {
        switch (api_feature) {
        case "download_pdf":
            this.options.api.download_pdf({
                drawing_order_background: this.view_mode.drawing_order_background(),
                drawing_order: this.view_mode.drawing_order(),
                projection_no: this.view_mode.projection_no(),
                styles: this.coloring.styles(),
                point_scale: this.view_mode.point_scale()
            });
            break;
        default:
            if (this.options.api[api_feature])
                this.options.api[api_feature]();
            else
                console.warn("unrecognized api_feature: " + api_feature);
            break;
        }
    }

    async sequences() {
        if (this.sequences_ === undefined) {
            this.sequences_ = "loading, please wait";
            return this.options.api.get_sequences().then(
                data => {
                    this.sequences_ = data.sequences;
                    return new Promise(resolve => resolve(this.sequences_));
                },
                error => {
                    this.sequences_ = "Error: " + error;
                    return new Promise((_, reject) => reject(error));
                });
        }
        else {
            return new Promise((resolve, reject) => {
                if (typeof(this.sequences_) === "string" && this.sequences_.substr(0, 6) === "Error:")
                    reject(this.sequences_);
                else
                    resolve(this.sequences_);
            });
        }
    }

    update_view_dialog() {
        if (this.view_dialog_)
            this.view_dialog_.show_legend();
    }
}

// ----------------------------------------------------------------------

class DrawingMode_Base
{
    constructor(widget, args={}) {
        this.widget = widget;
        this.shading_ = args.shading || "shade";
        if (widget) {
            this.projection_no_ = widget.options.projection_no;
            widget.show_title_arrows(null, null);
        }
    }

    shading(new_shading) {
        if (typeof(new_shading) === "string")
            this.shading_ = new_shading;
        return this.shading_;
    }

    period() {
        return null;
    }

    title_box() {
        const box_name = chart => chart.i.N ? `<li>${chart.i.N}</li>` : "";
        const box_virus = entry => `<li>${entry.v || ""} ${entry.V || ""} ${entry.A || ""} ${entry.r || ""}</li>`;
        const box_lab = entry => entry.l ? `<li>Lab: ${entry.l}</li>` : "";
        const box_antigens = chart => `<li>Antigens: ${chart.a.length}</li><li>Sera: ${chart.s.length}</li>`;
        const box_date = chart => chart.i.S && chart.i.S.length > 0 ? `<li>Dates: ${chart.i.S[0].D} - ${chart.i.S[chart.i.S.length - 1].D}</li>` : (chart.i.D ? `<li>Date: ${chart.i.D}</li>` : "");

        const box_tables = chart => {
            let result = "";
            if (chart.i.S && chart.i.S.length > 0) {
                const tables = chart.i.S.map(s_entry => s_entry.D || JSON.stringify(s_entry)).join("</li><li>");
                result = `<li>Tables: ${chart.i.S.length}<ol class='a-scrollable a-tables'><li>${tables}</li></ol></li>`;
            }
            else if (chart.t.L && chart.t.L.length > 0) {
                result = `<li>Layers: ${chart.t.L.length}</li>`;
            }
            return result;
        };

        const box_projections = chart => {
            let result = "";
            if (chart.P && chart.P.length > 0) {
                const stresses = chart.P.map(p_entry => p_entry.s ? p_entry.s.toFixed(4) : "<unknown stress>").join("</li><li>");
                result = `<li>Projections: ${chart.P.length}<ol class='a-scrollable a-stresses'><li>${stresses}</li></ol></li>`;
            }
            return result;
        };

        const box_sequenced = chart => {
            let clades = {}, sequenced = 0;
            chart.a.forEach(antigen => {
                if (antigen.c && antigen.c.length) {
                    antigen.c.forEach(clade => { clades[clade] = (clades[clade] || 0) + 1; });
                    ++sequenced;
                }
            });
            let clades_li = "";
            for (let cl in clades)
                clades_li += `<li>${cl}: ${clades[cl]}</li>`;
            return `<li>Sequenced: ${sequenced} <ul class='a-scrollable a-sequenced'>${clades_li}</ul></li>`;
        };

        const chart = this.widget.data.c;
        const projection_no = this.widget.options.projection_no;
        let title_box = $("<ul class='a-title-mouse-popup'></ul>");
        if (chart.i) {
            title_box.append(box_name(chart));
            if (chart.i.S && chart.i.S.length > 0) {
                title_box.append(box_virus(chart.i.S[0]));
                title_box.append(box_lab(chart.i.S[0]));
            }
            else {
                title_box.append(box_virus(chart.i));
                title_box.append(box_lab(chart.i));
            }
            title_box.append(box_antigens(chart));
            title_box.append(box_date(chart));
            title_box.append(box_sequenced(chart));
            title_box.append(box_tables(chart));
            title_box.append(box_projections(chart));
        }
        return title_box;
    }

    draw() {
        const chart = this.widget.data.c;
        const drawing_order_background = this.drawing_order_background();
        if (drawing_order_background && drawing_order_background.length)
            this.widget.surface.points({drawing_order: this.widget.coloring.drawing_order(drawing_order_background, {background: true}),
                                    layout: chart.P[this.projection_no()].l,
                                    transformation: new ace_surface.Transformation(chart.P[this.projection_no()].t),
                                    styles: this.styles(),
                                    point_scale: this.point_scale(),
                                    show_as_background: this.show_as_background()});
        this.widget.surface.points({drawing_order: this.widget.coloring.drawing_order(this.drawing_order()),
                                    layout: chart.P[this.projection_no()].l,
                                    transformation: new ace_surface.Transformation(chart.P[this.projection_no()].t),
                                    styles: this.styles(),
                                    point_scale: this.point_scale()});
    }

    projection_no() {
        return this.projection_no_;
    }

    current_page() {
        return undefined;
    }

    point_scale() {
        return this.widget.parameters.point_scale;
    }

    show_as_background() {
        switch (this.shading()) {
        case "grey":
            return this.widget.options.show_as_background;
        case "hide":
            return null;
        case "shade":
        default:
            return {shade: this.widget.options.show_as_background_shade};
        }
    }

    drawing_order_background() {
        return [];
    }

    styles() {
        return this.widget.coloring.styles();
    }
}

// ----------------------------------------------------------------------

class DrawingMode_Best_Projection extends DrawingMode_Base
{
    mode() {
        return "projection";
    }

    title(args) { // args: {title_fields:}
        const chart = this.widget.data.c;
        const projection_no = this.widget.options.projection_no;
        const makers = this.title_field_makers();
        return acv_utils.join_collapse(args.title_fields.map(field => makers[field](chart, projection_no)));
    }

    title_field_makers() {
        return {
            stress: (chart, projection_no) => { let stress = chart.P[projection_no].s; return stress ? stress.toFixed(4) : ""; },
            min_col_basis: (chart, projection_no) => { let mcb = chart.P[projection_no].m; return mcb ? ">=" + mcb : ">=none"; },
            name: (chart, projection_no) => chart.i.N,
            date: (chart, projection_no) => chart.i.S ? (chart.i.S[0].D + "-" + chart.i.S[chart.i.S.length - 1].D) : (chart.i.D),
            tables: (chart, projection_no) => chart.i.S ? `(${chart.i.S.length} tables)` : "",
            antigens: (chart, projection_no) => "A:" + chart.a.length,
            sera: (chart, projection_no) => "S:" + chart.s.length,
            lab: (chart, projection_no) => chart.i.S ? chart.i.S[0].l : chart.i.l,
            virus_type: (chart, projection_no) => chart.i.S ? chart.i.S[0].V : chart.i.V,
            assay: (chart, projection_no) => chart.i.S ? chart.i.S[0].A : chart.i.A
        };
    }

    drawing_order() {
        return this.widget.data.c.p.d;
    }
}

// ----------------------------------------------------------------------

class DrawingMode_Selection extends DrawingMode_Base
{
    constructor(widget, args={}) {
        super(widget, args);
        this.selected_antigens_ = [];
        this.selected_sera_ = [];
        this.drawing_order_background_ = this.widget.data.c.p.d;
        this.drawing_order_ = [];
    }

    mode() {
        return "selection";
    }

    title(args) {
        if (this.selected_antigens_.length || this.selected_sera_.length) {
            return `${this.selected_antigens_.length} antigens and ${this.selected_sera_.length} sera selected`;
        }
        else
            return "no matches";
    }

    drawing_order() {
        return this.drawing_order_;
    }

    drawing_order_background() {
        return this.drawing_order_background_;
    }

    styles() {
        return this.styles_ || this.widget.coloring.styles();
    }

    // {selection_results: <table>}
    reset(args) {
        args.selection_results.empty();
        this.selected_antigens_ = [];
        this.selected_sera_ = [];
        this._make_drawing_order();
        this.widget.draw();
    }

    // {regex:, subset: "antigens" "sera" "all", selection_results: <table>}
    filter(args) {
        args.selection_results.empty();
        this.selected_antigens_ = [];
        this.selected_sera_ = [];
        if (args.subset.indexOf("antigens") >= 0)
            this._match_antigens(args.regex);
        if (args.subset.indexOf("sera") >= 0)
            this._match_sera(args.regex);
        if (this.selected_antigens_.length || this.selected_sera_.length) {
            if (this.selected_antigens_.length)
                this._add_many(args.selection_results, this.selected_antigens_, "" + this.selected_antigens_.length + " antigens");
            if (this.selected_sera_.length)
                this._add_many(args.selection_results, this.selected_sera_, "" + this.selected_sera_.length + " sera");
            if ((this.selected_antigens_.length + this.selected_sera_.length) < 100) {
                if (this.selected_antigens_.length) {
                    this._add_separator(args.selection_results);
                    this.selected_antigens_.forEach(no => this._add(args.selection_results, no, this.antigens_match_, 0));
                }
                if (this.selected_sera_.length) {
                    this._add_separator(args.selection_results);
                    this.selected_sera_.forEach(no => this._add(args.selection_results, no, this.sera_match_, this.widget.data.c.a.length));
                }
            }
            this._make_drawing_order();
        }
        else {
            args.selection_results.append("<tr><td class='a-message'>nothing matched</td></tr>");
        }
        this.widget.draw();
    }

    _match_antigens(regex) {
        if (!this.antigens_match_)
            this.antigens_match_ = this.widget.data.c.a.map(ag => acv_utils.join_collapse([ag.N, ag.R, acv_utils.join_collapse(ag.a), ag.P, acv_utils.join_collapse(ag.l)]));
        this._match(regex, this.antigens_match_, this.selected_antigens_, 0);
    }

    _match_sera(regex) {
        if (!this.sera_match_)
            this.sera_match_ = this.widget.data.c.s.map(sr => acv_utils.join_collapse([sr.N, sr.R, acv_utils.join_collapse(sr.a), sr.I, sr.s]));
        this._match(regex, this.sera_match_, this.selected_sera_, this.widget.data.c.a.length);
    }

    _match(regex, match_data, result, base) {
        const re = new RegExp(regex, "i");
        // result.splice(0);
        match_data.forEach((ag, no) => {
            if (ag.search(re) >= 0)
                result.push(no + base);
        });
    }

    _make_drawing_order() {
        this.drawing_order_ = this.selected_sera_.concat(this.selected_antigens_);
        this.drawing_order_background_ = this.widget.data.c.p.d.filter(no => !this.drawing_order_.includes(no));
    }

    _add_many(table, indexes, label) {
        const styles = this._make_styles().styles;
        const attrs = indexes.reduce((attrs, index) => {
            const style = styles[index];
            attrs.shape[style.S || "C"] = true;
            attrs.fill[style.F || "transparent"] = true;
            attrs.outline[style.O || "black"] = true;
            attrs.outline_width[style.o || 1] = true;
            attrs.aspect[style.a || 1] = true;
            attrs.rotation[style.r || 0] = true;
            attrs.size[style.s || 0] = true;
            return attrs;
        }, {shape: {}, fill: {}, outline: {}, outline_width: {}, aspect: {}, rotation: {}, size: {}});
        const canvas = $("<canvas></canvas>");
        Object.entries(attrs).forEach(entry => {
            const keys = Object.keys(entry[1]);
            if (keys.length === 1)
                canvas.attr("acv_" + entry[0], keys[0]);
        });
        const tr = $(`<tr class='a-many'><td class='a-plot-spec'></td><td class='a-label'>${label}</td></tr>`).appendTo(table);
        tr.find("td.a-plot-spec").append(canvas);
        acv_point_style.point_style_modifier({canvas: canvas, onchange: data => this._style_modified(data, indexes)});
    }

    _add(table, index, collection, base) {
        const style = this._make_styles().styles[index];
        const canvas = `<canvas acv_shape="${style.S || 'C'}" acv_size="${style.s || 1}" acv_fill="${style.F || 'transparent'}" acv_outline="${style.O || 'black'}" acv_outline_width="${style.o || 1}" acv_aspect="${style.a || 1}" acv_rotation="${style.r || 0}"></canvas>`;
        const tr = $(`<tr class='a-many'><td class='a-plot-spec'>${canvas}</td><td class='a-label'>${collection[index - base]}</td></tr>`).appendTo(table);
        acv_point_style.point_style_modifier({canvas: tr.find("canvas"), onchange: data => this._style_modified(data, [index])});
    }

    _add_separator(table) {
        table.append("<tr class='a-separator'><td colspan='2'></td></tr>");
    }

    _make_styles() {
        if (!this.styles_) {
            const as = this.widget.coloring.all_styles();
            this.styles_ = {index: as.index, styles: as.styles.map(st => Object.assign({}, st))};
        }
        return this.styles_;
    }

    _style_modified(data, indexes) {
        const key_mapping = {fill: "F", outline: "O", outline_width: "o", aspect: "a", rotation: "r", shape: "S", size: "s"};
        // console.log("_style_modified", data, indexes, this.styles_.styles[indexes[0]]);
        indexes.forEach(index => this.styles_.styles[index][key_mapping[data.name]] = data.value);
        this.widget.draw();
    }
}

// ----------------------------------------------------------------------

class DrawingMode_Series extends DrawingMode_Base
{
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

    drawing_order() {
        return this.drawing_order_;
    }

    drawing_order_background() {
        return this.drawing_order_background_;
    }

    current_page() {
        return this.page_no;
    }
}

// ----------------------------------------------------------------------

class DrawingMode_TimeSeries extends DrawingMode_Series
{
    constructor(widget, args={}) {
        super(widget, args);
        this.period_ = (args && args.period) || "month";
        this.make_pages();
        this.set_page(args.page === undefined ? this.pages.length - 1 : args.page);
    }

    mode() {
        return "time-series";
    }

    period() {
        return this.period_;
    }

    make_pages() {
        let periods = new Set();
        for (let antigen of this.widget.data.c.a) {
            const period_name = this.antigen_period_name(antigen);
            if (period_name)
                periods.add(period_name);
        }
        this.pages = [...periods].sort();
    }

    make_drawing_order() {
        const page_period_name = this.pages[this.page_no];
        const in_page = antigen => this.antigen_period_name(antigen) === page_period_name;
        const antigens = this.widget.data.c.a;
        this.drawing_order_ = [];
        if (this.shading() === "hide") {
            this.drawing_order_background_ = undefined;
            for (let point_no of this.widget.data.c.p.d) {
                if (point_no >= antigens.length || (antigens[point_no].S && antigens[point_no].S.indexOf("R") >= 0 && !in_page(antigens[point_no])))
                    this.drawing_order_.push(point_no);
            }
            for (let point_no of this.widget.data.c.p.d) {
                if (point_no < antigens.length && in_page(antigens[point_no]))
                    this.drawing_order_.push(point_no);
            }
        }
        else {
            this.drawing_order_background_ = [];
            for (let point_no of this.widget.data.c.p.d) {
                if (point_no < antigens.length && in_page(antigens[point_no]))
                    this.drawing_order_.push(point_no);
                else
                    this.drawing_order_background_.push(point_no);
            }
        }
    }

    antigen_period_name(antigen) {
        switch (this.period_) {
        case "year":
            return antigen.D && antigen.D.substr(0, 4);
        case "season":
            const season = (year, month) => {
                if (month <= 4)
                    return `${year - 1} Nov - ${year} Apr`;
                else if (month <= 10)
                    return `${year} May - Oct`;
                else
                    return `${year} Nov - ${year + 1} Apr`;
            };
            return antigen.D && season(parseInt(antigen.D.substr(0, 4)), parseInt(antigen.D.substr(5, 2)));
        case "month":
        default:
            return antigen.D && antigen.D.substr(0, 7);
        }
    }
}

// ----------------------------------------------------------------------

class DrawingMode_TableSeries extends DrawingMode_Series
{
    constructor(widget, args={}) {
        super(widget, args);
        this.make_pages();
        this.set_page(args.page === undefined ? this.pages.length - 1 : args.page);
    }

    mode() {
        return "table-series";
    }

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
        this.drawing_order_ = this.widget.data.c.p.d.filter(point_in_layer);
        if (this.shading() !== "hide")
            this.drawing_order_background_ = this.widget.data.c.p.d.filter(point_no => !point_in_layer(point_no));
    }
}

// ----------------------------------------------------------------------

class DrawingMode_GroupSeries extends DrawingMode_Series
{
    constructor(widget, args={}) {
        super(widget, args);
        this.pages_exclusive_ = ["*no-groups*"];
        this.groups_combined_ = [];
        this.combined_mode("exclusive");
        this.set_page(args.page === undefined ? 0 : args.page);
    }

    mode() {
        return "group-series";
    }

    set(args) {
        if (args.shading)
            this.shading_ = args.shading;
    }

    combined_mode(new_mode) {
        if (typeof(new_mode) === "string") {
            this.combined_mode_ = new_mode;
            if (new_mode === "exclusive") {
                this.pages = this.pages_exclusive_;
            }
            else {
                this.pages = ["Multiple groups"];
            }
        }
        return this.combined_mode_;
    }

    add_combined_group(group) {
        if (!this.groups_combined_.includes(group)) {
            this.groups_combined_.push(group);
            this.set_page(this.page_no, true);
        }
    }

    remove_combined_group(group) {
        const index = this.groups_combined_.indexOf(group);
        if (index >= 0) {
            this.groups_combined_.splice(index, 1);
            this.set_page(this.page_no, true);
        }
    }

    set_page(page_no, redraw) {
        if (this.combined_mode() === "exclusive")
            super.set_page(page_no, redraw);
        else
            super.set_page(0, redraw);
    }

    draw() {
        super.draw();
        this._draw_root_connecting_lines();
    }

    _draw_root_connecting_lines() {
        if (this.combined_mode() === "exclusive")
            this._draw_root_connecting_line(this.groups_ && this.groups_[this.page_no]);
        else
            this.groups_combined_.forEach(group => this._draw_root_connecting_line(group));
    }

    _draw_root_connecting_line(group) {
        group = this._find_group(group);
        if (group && group.root !== undefined) {
            const line_color = group.line_color || this.gs_line_color_;
            const line_width = group.line_width || this.gs_line_width_;
            group.members.filter(point_no => this.drawing_order_.includes(point_no))
                .filter(point_no => point_no !== group.root)
                .forEach(point_no => this.widget.surface.line_connecting_points({p1: point_no, p2: group.root, line_color: line_color, line_width: line_width}));
        }
    }

    make_pages(group_set) {
        this.groups_ = group_set.groups;
        this.gs_line_color_ = group_set.line_color || "black";
        this.gs_line_width_ = group_set.line_width || 1;
        this.pages_exclusive_ = this.groups_.map(gr => gr.N);
        this.combined_mode(this.combined_mode());
        this.set_page(this.current_page() || 0, true);
    }

    make_drawing_order() {
        if (this.combined_mode() === "exclusive")
            this._make_drawing_order_exclusive();
        else
            this._make_drawing_order_combined();
        if (this.shading() !== "hide") {
            const chart = this.widget.data.c;
            this.drawing_order_background_ = acv_utils.array_of_indexes(chart.a.length + chart.s.length).filter(index => !this.drawing_order_.includes(index));
        }
        else
            this.drawing_order_background_ = undefined;
    }

    _make_drawing_order_exclusive() {
        this.drawing_order_ = [];
        if (this.groups_ && this.groups_[this.page_no])
            this._update_drawing_order_(this.groups_[this.page_no]);
    }

    _make_drawing_order_combined() {
        this.drawing_order_ = [];
        this.groups_combined_.forEach(group => this._update_drawing_order_(group));
    }

    _update_drawing_order_(group) {
        const add_member = point_no => {
            const index = this.drawing_order_.indexOf(point_no);
            if (index >= 0)
                this.drawing_order_.splice(index, 1);
            this.drawing_order_.push(point_no);
        };
        group = this._find_group(group);
        group.members.forEach(add_member);
        if (group.root !== undefined)
            add_member(group.root);
    }

    _find_group(group) {
        if (typeof(group) === "string")
            group = this.groups_.find(grp => grp.N === group);
        return group;
    }
}

// ----------------------------------------------------------------------

class Coloring_Base
{
    constructor(widget) {
        this.widget = widget;
    }

    drawing_order(original_drawing_order, options) {
        if (!Array.isArray(original_drawing_order)) {
            const chart = this.widget.data.c;
            original_drawing_order = acv_utils.array_of_indexes(chart.a.length + chart.s.length);
        }
        return original_drawing_order;
    }

    legend() {
        return null;
    }

    // {reset_sera: false}
    all_styles(args={}) {
        const chart = this.widget.data.c;
        const egg_passage = (style, index) => {
            if (index < chart.a.length && (!style.a || style.a === 1) && chart.a[index].S && chart.a[index].S.indexOf("E") >= 0)
                style.a = 0.75;
            if (index < chart.a.length && (!style.r || style.r === 0) && chart.a[index].R && chart.a[index].R.length)
                style.r = Math.PI / 6;
            return style;
        };
        const all_styles_1 = chart.p.p.map(style_no => Object.assign({}, chart.p.P[style_no])).map(egg_passage);
        let all_styles = {index: acv_utils.array_of_indexes(all_styles_1.length), styles: all_styles_1};
        if (args.reset_sera) {
            chart.s.forEach((serum, serum_no) => {
                delete all_styles.styles[serum_no + chart.a.length].F;
                all_styles.styles[serum_no + chart.a.length].O = acv_toolkit.sGREY;
            });
        }
        return all_styles;
    }
}

// ----------------------------------------------------------------------

class Coloring_Default extends Coloring_Base
{
    coloring() {
        return "default";
    }

    styles() {
        const chart = this.widget.data.c;
        return {index: chart.p.p, styles: chart.p.P};
    }
}

// ----------------------------------------------------------------------

class Coloring_WithAllStyles extends Coloring_Base
{
    constructor(widget) {
        super(widget);
        this.styles_ = super.all_styles({reset_sera: true});
    }

    styles() {
        return this.styles_;
    }

    all_styles() {
        return this.styles_;
    }
}

// ----------------------------------------------------------------------

const continent_colors = {
    "EUROPE":            "#00ff00",
    "CENTRAL-AMERICA":   "#aaf9ff",
    "MIDDLE-EAST":       "#8000ff",
    "NORTH-AMERICA":     "#00008b",
    "AFRICA":            "#ff7f00",
    "ASIA":              "#ff0000",
    "RUSSIA":            "#b03060",
    "AUSTRALIA-OCEANIA": "#ff69b4",
    "SOUTH-AMERICA":     "#40e0d0",
    "ANTARCTICA":        "#7f7f7f",
    "UNKNOWN":           "#7f7f7f",
    "":                  "#7f7f7f",
    null:                "#7f7f7f",
    undefined:           "#7f7f7f"
};

const continent_name_for_legend = {
    "EUROPE":            "Europe",
    "CENTRAL-AMERICA":   "C-America",
    "MIDDLE-EAST":       "MiddleEast",
    "NORTH-AMERICA":     "N-America",
    "AFRICA":            "Africa",
    "ASIA":              "Asia",
    "RUSSIA":            "Russia",
    "AUSTRALIA-OCEANIA": "Australia",
    "SOUTH-AMERICA":     "S-America",
    "ANTARCTICA":        "Antarctica",
    "UNKNOWN":           "unknown",
    "":                  "unknown",
    null:                "unknown",
    undefined:           "unknown"
};

class Coloring_Continent extends Coloring_WithAllStyles
{
    constructor(widget) {
        super(widget);
        this._make_continent_count({set_styles: true});
    }

    coloring() {
        return "continent";
    }

    drawing_order(original_drawing_order, options) {
        // order: sera, most popular continent, ..., lest popular continent
        const continent_order = this.continent_count.map(entry => entry.name);
        const chart = this.widget.data.c;
        const ranks = Array.apply(null, {length: chart.a.length}).map((_, ag_no) => continent_order.indexOf(chart.a[ag_no].C) + 10).concat(Array.apply(null, {length: chart.s.length}).map(_ => 0));
        this.drawing_order_ = super.drawing_order(original_drawing_order).slice(0).sort((p1, p2) => ranks[p1] - ranks[p2]);
        if (!options || !options.background)
            this._make_continent_count();
        return this.drawing_order_;
    }

    legend() {
        return this.continent_count.map(entry => Object.assign({}, entry, {name: continent_name_for_legend[entry.name] || entry.name}));
    }

    _make_continent_count(args) {
        const drawing_order = this.drawing_order_ || super.drawing_order();
        const chart = this.widget.data.c;
        let continent_count = {};
        drawing_order.filter(no => no < chart.a.length).forEach(antigen_no => {
            const continent = chart.a[antigen_no].C;
            if (args && args.set_styles)
                this.styles_.styles[antigen_no].F = continent_colors[continent];
            continent_count[continent] = (continent_count[continent] || 0) + 1;
        });
        this.continent_count = Object.keys(continent_count)
            .map(continent => { return {name: continent, count: continent_count[continent], color: continent_colors[continent]}; })
            .sort((e1, e2) => e2.count - e1.count);
    }
}

// ----------------------------------------------------------------------

const sCladeColors = {
    "3C.3": "#6495ed",
    "3C.3A": "#00ff00",
    "3C.3B": "#0000ff",
    "3C.2A": "#ff0000",
    "3C.2A1": "#8b0000",
    "3C.2A1A": "#8b0000",
    "3C.2A1B": "#8b0000",
    "3C.2A2": "#8b4040",
    "3C.2A3": "#8b4000",
    "3C.2A4": "#8b0040",
    "6B1": "#0000ff",
    "6B2": "#ff0000",
    "V1": "#0000ff",
    "V1A": "#6495ed",
    "V1B": "#ff0000",
    "3DEL2017": "#de8244",
    "3DEL2017": "#bf3eff",
    "Y2": "#6495ed",
    "Y3": "#ff0000",
    "SEQUENCED": "#ffa500",
    // "NO-GLY": "#ffa500",
    // "GLY": "#ff00a5",
    // "GLY": "#ffa500",
    "": acv_toolkit.sGREY,
    undefined: acv_toolkit.sGREY,
    null: acv_toolkit.sGREY
};

class Coloring_Clade extends Coloring_WithAllStyles
{
    constructor(widget) {
        super(widget);
        this._make_antigens_by_clade({set_clade_for_antigen: true});
        this._make_styles();
    }

    coloring() {
        return "clade";
    }

    _make_antigens_by_clade(args) {
        const drawing_order = this.drawing_order_ || super.drawing_order();
        const chart = this.widget.data.c;
        const clade_sorting_key = clade => (clade === "GLY" || clade === "NO-GLY" || clade === "SEQUENCED") ? 0 : clade.length;
        this.clade_to_number_of_antigens = {};
        if (args && args.set_clade_for_antigen)
            this.clade_for_antigen = Array.apply(null, {length: chart.a.length}).map(() => "");
        drawing_order.filter(no => no < chart.a.length).forEach(antigen_no => {
            const clades = (chart.a[antigen_no].c || []).sort((a, b) => clade_sorting_key(b) - clade_sorting_key(a));
            let clade = clades.length > 0 ? clades[0] : "";
            if (clade === "GLY" || clade === "NO-GLY")
                clade = "SEQUENCED";
            this.clade_to_number_of_antigens[clade] = (this.clade_to_number_of_antigens[clade] || 0) + 1;
            if (args && args.set_clade_for_antigen)
                this.clade_for_antigen[antigen_no] = clade;
        });
        this.clade_order = Object.keys(this.clade_to_number_of_antigens).sort((a, b) => this._clade_rank(a) - this._clade_rank(b));
        if (args && args.set_clade_for_antigen)
            this.point_rank_ = this.clade_for_antigen.map(clade => this.clade_order.indexOf(clade)).concat(Array.apply(null, {length: this.widget.data.c.s.length}).map(() => -2));
    }

    _clade_rank(clade) {
        // order: not sequenced, sequenced without clade, clade with max number of antigens, ..., clade with fewer antigens
        if (clade === "")
            return -1e7;
        if (clade === "GLY" || clade === "NO-GLY" || clade === "SEQUENCED")
            return -1e6;
        return - this.clade_to_number_of_antigens[clade];
    }

    _make_styles() {
        this.clade_for_antigen.forEach((clade, antigen_no) => {
            this.styles_.styles[antigen_no].F = sCladeColors[clade];
            // this.styles_.styles[antigen_no].O = "white";
        });
    }

    drawing_order(original_drawing_order, options) {
        // order: sera, not sequenced, sequenced without clade, clade with max number of antigens, ..., clade with fewer antigens
        this.drawing_order_ = super.drawing_order(original_drawing_order).slice(0).sort((p1, p2) => this.point_rank_[p1] - this.point_rank_[p2]);
        if (!options || !options.background)
            this._make_antigens_by_clade();
        return this.drawing_order_;
    }

    legend() {
        return this.clade_order.filter(clade => !!clade).map(clade => { return {name: clade, count: this.clade_to_number_of_antigens[clade], color: sCladeColors[clade]}; });
    }
}

// ----------------------------------------------------------------------


class Coloring_AAPos extends Coloring_WithAllStyles
{
    constructor(widget) {
        super(widget);
        this._make_styles({set_point_rank: true});
        widget.sequences().then(data => this._sequences_received(data)).catch(error => console.log("Coloring_AAPos::constructor sequences error", error));
    }

    coloring() {
        return "aa_pos";
    }

    _make_styles(args) {
        this._reset_styles();
        this.legend_ = null;
        if (this.sequences_ && this.positions_ && this.positions_.length) {
            this.antigen_aa_  = Object.entries(this.sequences_.antigens).map(entry => { return {no: parseInt(entry[0]), aa: this.positions_.map(pos => entry[1][pos - 1]).join("")}; });
            const aa_count = this.antigen_aa_.reduce((count, entry) => { count[entry.aa] = (count[entry.aa] || 0) + 1; return count; }, {});
            this.aa_order_ = Object.keys(aa_count).sort((e1, e2) => aa_count[e2] - aa_count[e1]);
            if (args && args.set_point_rank)
                this.point_rank_ = Array.apply(null, {length: this.widget.data.c.a.length}).map(() => -1).concat(Array.apply(null, {length: this.widget.data.c.s.length}).map(() => -2));
            this.antigen_aa_.forEach(entry => {
                const aa_index = this.aa_order_.indexOf(entry.aa);
                this.styles_.styles[entry.no].F = acv_toolkit.ana_colors(aa_index);
                this.styles_.styles[entry.no].O = "black";
                if (args && args.set_point_rank)
                    this.point_rank_[entry.no] = aa_index;
            });
        }
    }

    drawing_order(original_drawing_order, options) {
        // order: sera, not sequenced, "clade" with max number of antigens, ..., "clade" with fewer antigens
        original_drawing_order = super.drawing_order(original_drawing_order);
        if (!this.point_rank_)
            return original_drawing_order;
        this.drawing_order_ = original_drawing_order.slice(0).sort((p1, p2) => this.point_rank_[p1] - this.point_rank_[p2]);
        if (!options || !options.background)
            this._make_legend();
        return this.drawing_order_;
    }

    _make_legend() {
        const aa_count = this.antigen_aa_.reduce((count, entry) => {
            if (this.drawing_order_.includes(entry.no))
                count[entry.aa] = (count[entry.aa] || 0) + 1;
            return count;
        }, {});
        this.legend_ = this.aa_order_.map((aa, index) => aa_count[aa] ? {name: aa, count: aa_count[aa], color: acv_toolkit.ana_colors(index)} : null).filter(elt => !!elt);
    }

    legend() {
        if (this.sequences_) {
            if (this.legend_)
                return this.legend_;
            else
                return [{name: "type space separated positions and press Enter"}];
        }
        else
            return [{name: "loading, please wait"}];
    }

    positions() {
        return this.positions_;
    }

    set_positions(positions) {
        const update = positions !== this.positions_;
        if (update) {
            this.positions_ = positions;
            this._make_styles({set_point_rank: true});
        }
        return update;
    }

    _reset_styles() {
        this.widget.data.c.a.forEach((antigen, antigen_no) => {
            this.styles_.styles[antigen_no].F = this.styles_.styles[antigen_no].O = acv_toolkit.sLIGHTGREY;
        });
    }

    _sequences_received(data) {
        this.sequences_ = data;
        this.widget.update_view_dialog();
    }
}

// ----------------------------------------------------------------------

const coloring_selector_data = {
    "clade": Coloring_Clade,
    "aa_pos": Coloring_AAPos,
    "continent": Coloring_Continent,
    "default": Coloring_Default,
    null: Coloring_Default
};

function select_coloring(coloring, widget) {
    return new (coloring_selector_data[coloring] || coloring_selector_data[null])(widget);
}

// ----------------------------------------------------------------------

const AntigenicTable_serum_rows_html = "\
<tr class='a-serum-nos'>\
 <td></td>\
 <td></td>\
 ${nos}\
</tr>\
<tr class='a-serum-names'>\
 <td></td>\
 <td></td>\
 ${names}\
</tr>\
";

const AntigenicTable_antigen_row_html = "\
<tr class='a-antigen' antigen_no='${no0}'>\
 <td class='a-antigen-no'>${no1}</td>\
 <td class='a-antigen-name'>${name}</td>\
 ${titers}\
</tr>\
";

class AntigenicTable_populate
{
    constructor(args) {
        this.widget = args.widget;
        this.chart = args.chart;
        if (this.chart.a.length < 200000) {
            this.div = $("<table class='antigenic-table'></table>").appendTo(args.parent);
            this.make_sera();
            this.make_antigens();
            this.set_size(args.parent);
        }
        else {
            this.div = $(`<p class='a-error-message'>Table is too big: ${this.chart.a.length} antigens</table>`).appendTo(args.parent);
        }
    }

    make_sera() {
        this.div.append(acv_utils.format(AntigenicTable_serum_rows_html, {
            nos: acv_utils.array_of_indexes(this.chart.s.length, 1).map(no => `<td class='a-serum-no' serum_no='${no-1}'>${no}</td>`).join(""),
            names: this.chart.s.map((serum, serum_no) => this.make_serum_name(serum, serum_no)).map((text, serum_no) => `<td class='a-serum-name' serum_no='${serum_no}'>${text}</td>`).join("")
        }));
    }

    make_antigens() {
        const chunk_size = 50;
        let antigen_no = 0;
        const populate = () => {
            const last = Math.min(antigen_no + 50, this.chart.a.length);
            for (; antigen_no < last; ++antigen_no) {
                const antigen = this.chart.a[antigen_no];
                this.div.append(acv_utils.format(AntigenicTable_antigen_row_html, {no0: antigen_no, no1: antigen_no + 1, name: this.make_antigen_name(antigen, antigen_no), titers: this.make_titers_for_antigen(antigen_no)}));
            }
            this.show_antigen_serum_info();
            if (last < this.chart.a.length)
                window.setTimeout(populate, 0);
        };
        populate();
    }

    make_antigen_name(antigen, antigen_no) {
        const title = "title='" + acv_utils.ace_antigen_full_name(antigen, {escape: true}) + "'";
        const abbr_name = acv_utils.antigen_serum_abbreviated_name(antigen.N);
        if (this.widget.options.point_on_click)
            return `<a ${title} point_no='${antigen_no}' href='#antigen-info-from-hidb'>${abbr_name}</a>`;
        else
            return `<span ${title}>${abbr_name}</span>`;
    }

    make_serum_name(serum, serum_no) {
        const title = "title='" + acv_utils.ace_serum_full_name(serum, {escape: true}) + "'";
        const abbr_name = acv_utils.antigen_serum_abbreviated_name(serum.N, {exclude_year: true});
        if (this.widget.options.point_on_click)
            return `<a ${title} point_no='${serum_no + this.chart.a.length}' href='#serum-info-from-hidb'>${abbr_name}</a>`;
        else
            return `<span ${title}>${abbr_name}</span>`;
    }

    make_titers_for_antigen(antigen_no) {
        return this.chart.s.map((serum, serum_no) => {
            const tt = acv_utils.ace_titer(this.chart, antigen_no, serum_no);
            let cls = "";
            switch (tt[0]) {
            case "*":
                cls = "a-titer-dontcare";
                break;
            case "<":
                cls = "a-titer-thresholded a-titer-less";
                break;
            case ">":
                cls = "a-titer-thresholded a-titer-more";
                break;
            case "~":
                cls = "a-titer-dodgy";
                break;
            default:
                cls = "a-titer-numeric";
                break;
            }
            const ag_name = acv_utils.ace_antigen_full_name(this.chart.a[antigen_no]);
            const sr_name = acv_utils.ace_serum_full_name(this.chart.s[serum_no]);
            return `<td class='a-titer ${cls}' serum_no='${serum_no}' title='AG: ${ag_name}\nSR: ${sr_name}'>${tt}</td>`;
        }).join("");
    }

    set_size(parent) {
        if (parent.width() > 600) {
            parent.css("width", "600px");
            if (parent.parent().width() > 600)
                parent.css("width", parent.parent().width());
        }
    }

    show_antigen_serum_info() {
        // called many times while table is populated, first remove old callbacks
        this.div.find("a[title]").off("click");
        this.div.find("a[title]").on("click", evt => acv_utils.forward_event(evt, () => show_antigen_serum_info_from_hidb($(evt.target), this.chart, this.widget.canvas, this.widget.options.point_on_click)));
    }
}

// ----------------------------------------------------------------------

class AntigenicTable
{
    constructor(args) {
        if (args.populate)
            this.populate(args);
        else if (args.find)
            this.find(args);
    }

    populate(args) {
        const title_fields = ["name", "lab", "virus_type", "assay", "date"];
        const makers = new DrawingMode_Best_Projection().title_field_makers();
        const win_space = $(window).height() - (args.parent.offset().top - $(window).scrollTop());
        const max_height = Math.max(win_space - args.parent.height() / 2 - 20, 400);
        const movable_window = new acv_toolkit.MovableWindow({
            title: acv_utils.join_collapse(title_fields.map(field => makers[field](args.chart))),
            parent: args.widget.div.find("canvas"), // args.parent,
            classes: "antigenic-table-movable",
            content_css: {width: "auto", height: "auto", "max-height": max_height},
            id: args.id
        });
        new AntigenicTable_populate({widget: args.widget, parent: movable_window.content(), chart: args.chart});

        const movable_window_content = movable_window.content();
        const target_height = $(window).height() - (movable_window_content.parent().outerHeight() - movable_window_content.height()) * 1.2;
        const target_width = $(window).width() - (movable_window_content.parent().outerWidth() - movable_window_content.width()) * 1.2;
        movable_window_content.css({"max-height": "", "max-width": ""});
        movable_window_content.css({"height": Math.min(movable_window_content.height(), target_height), "width": Math.min(movable_window_content.width(), target_width)});
    }

    find(args) {
        const table = $("#" + args.id).find(".a-content > .antigenic-table");
        if (table.length > 0) {
            this.table = table;
            this.chart = args.chart;
        }
    }

    show_points(points) {
        if (this.table) {
            this.not_show_points();
            points.forEach((point_no, index) => {
                if (point_no < this.chart.a.length) {
                    this.show_antigen(point_no);
                    if (index === 0)
                        this.scroll_to_antigen(point_no);
                }
                else {
                    const serum_no = point_no - this.chart.a.length;
                    this.show_serum(serum_no);
                    if (index === 0)
                        this.scroll_to_serum(serum_no);
                }
            });
        }
    }

    not_show_points() {
        this.table.find("tr.a-antigen").removeClass("a-highlight");
        this.table.find("td[serum_no]").removeClass("a-highlight");
    }

    show_antigen(antigen_no) {
        this.table.find(`tr.a-antigen[antigen_no='${antigen_no}']`).addClass("a-highlight");
    }

    show_serum(serum_no) {
        this.table.find(`td[serum_no='${serum_no}']`).addClass("a-highlight");
    }

    scroll_to_antigen(antigen_no) {
        const row = this.table.find(`tr.a-antigen[antigen_no='${antigen_no}']`);
        if (row.length > 0) {
            const row_top = row.position().top;
            const scrollable = this.table.parent();
            if (row_top < scrollable.scrollTop())
                scrollable.animate({scrollTop: row_top, scrollLeft: 0}, 500);
            else if (row_top > (scrollable.scrollTop() + scrollable.height()))
                scrollable.animate({scrollTop: row_top - scrollable.height() * 0.9, scrollLeft: 0}, 500);
            else if (scrollable.scrollLeft() > 0)
                scrollable.animate({scrollLeft: 0}, 500);
        }
    }

    scroll_to_serum(serum_no) {
        const column = this.table.find(`td.a-serum-name[serum_no='${serum_no}']`);
        if (column.length > 0) {
            const column_left = column.position().left;
            const scrollable = this.table.parent();
            if (column_left < scrollable.scrollLeft())
                scrollable.animate({scrollTop: 0, scrollLeft: column_left}, 500);
            else if (column_left > (scrollable.scrollLeft() + scrollable.width()))
                scrollable.animate({scrollTop: 0, scrollLeft: column_left - scrollable.width() * 0.9}, 500);
            else if (scrollable.scrollTop() > 0)
                scrollable.animate({scrollTop: 0}, 500);
        }
    }
}

// ----------------------------------------------------------------------

const ViewDialog_html = "\
<table class='a-view-dialog'>\
  <tr>\
    <td class='a-label'>Projection</td>\
    <td class='projection-chooser'></td>\
  </tr>\
  <tr>\
    <td class='a-label'>Coloring</td>\
    <td class='coloring'></td>\
  </tr>\
  <tr class='coloring-aa-pos'>\
    <td class='a-label'>Positions</td>\
    <td class='coloring-aa-pos'><input type='text'></input><a href='coloring-aa-pos-hint'>hint</a></td>\
  </tr>\
  <tr class='coloring-legend'>\
    <td class='a-label'>Legend</td>\
    <td class='coloring-legend'></td>\
  </tr>\
  <tr>\
    <td class='a-label'>Mode</td>\
    <td class='mode'></td>\
  </tr>\
  <tr class='time-series-period'>\
    <td class='a-label'>Period</td>\
    <td class='period'>\
      <a href='month'>month</a>\
      <a href='season'>winter/summer</a>\
      <a href='year'>year</a>\
    </td>\
  </tr>\
  <tr class='group-series'>\
    <td class='a-label'>Group Sets</td>\
    <td class='group-series'>\
      <div class='a-sets'>\
        <p class='a-hint'>please drop here group description file or click below to upload</p>\
      </div>\
      <div class='a-buttons'>\
        <a href='upload' title='upload and apply group definition'>upload</a>\
        <a href='download' title='download sample group definition for this chart'>download sample</a>\
        <a href='download-chart' title='download chart in the .ace format with the embedded group data'>download chart</a>\
      </div>\
    </td>\
  </tr>\
  <tr class='group-series-combined'>\
    <td class='a-label'>Groups</td>\
    <td class='group-series-combined'>\
      <div class='a-buttons'>\
        <a href='exclusive'>exclusive</a>\
        <a href='combined'>combined</a>\
      </div>\
      <table class='a-groups'>\
      </table>\
    </td>\
  </tr>\
  <tr class='shading'>\
    <td class='a-label'>Shading</td>\
    <td class='shading'>\
      <a href='hide'>legacy</a>\
      <a href='shade'>shade</a>\
      <a href='grey'>grey</a>\
    </td>\
  </tr>\
  <tr class='selection'>\
    <td class='a-label'>RegEx</td>\
    <td class='regex'>\
      <input type='text'></input>\
      <select name='selection-mode'>\
        <option value='antigens'>antigens only</option>\
        <option value='sera'>sera only</option>\
        <option value='antigens+sera'>antigens and sera</option>\
      </select>\
    </td>\
  <tr class='selection-results'>\
    <td class='a-label'></td>\
    <td class='selection-results'>\
      <p class='a-message'>please enter regular expression above and press enter</p>\
      <table></table>\
    </td>\
  </tr>\
</table>\
";

class ViewDialog
{
    constructor(args) {
        this.widget = args.widget;
        this.id = args.id;
        const movable_window = new acv_toolkit.MovableWindow({
            title: "View",
            parent: args.widget.div.find("canvas"),
            classes: "view-dialog-movable",
            content_css: {width: "auto", height: "auto"},
            id: args.id,
            on_destroy: () => this.destroy()
        });
        this.content = movable_window.content();
        if (movable_window.content().find("table.view-dialog").length === 0)
            this.populate({content: this.content, chart: args.chart});
        this.on_destroy = args.on_destroy;
    }

    destroy() {
        if (this.on_destroy)
            this.on_destroy();
    }

    find_me() {
        new acv_toolkit.MovableWindow({parent: this.widget.div.find("canvas"), id: this.id});
    }

    populate(args) {
        const table = $(ViewDialog_html).appendTo(args.content);
        if (args.chart.P.length === 0)
            table.find("td.projection-chooser").append("<div class='a-error'>None</div>");
        else if (args.chart.P.length === 1)
            table.find("td.projection-chooser").append(`<div>${this.projection_title(args.chart.P[0])}</div>`);
        else {
            const entries = args.chart.P.map((prj, index) => `<option value="${index}">${this.projection_title(prj, index)}</option>`).join();
            const select = $(`<select>${entries}</select>`).appendTo(table.find("td.projection-chooser"));
            select.on("change", evt => acv_utils.forward_event(evt, evt => {
                this.widget.set_view_mode({projection_no: this.projection_no()});
            }));
        }

        const td_coloring = table.find("td.coloring");
        td_coloring.append("<a href='default'>original</a>");
        if (this.widget.features["clades"]) {
            td_coloring.append("<a href='clade'>by clade</a>");
            if (this.widget.options.api.get_sequences)
                td_coloring.append("<a href='aa_pos'>by AA at pos</a>");
        }
        if (this.widget.features["continents"])
            td_coloring.append("<a href='continent'>by geography</a>");
        td_coloring.find("a").on("click", evt => acv_utils.forward_event(evt, evt => {
            this.widget.set_coloring(evt.currentTarget.getAttribute("href"));
            this.set_current_mode();
        }));

        const td_mode = table.find("td.mode");
        td_mode.append("<a href='projection'>all</a>");
        td_mode.append("<a href='selection'>search</a>");
        if (this.widget.features["time-series"])
            td_mode.append("<a href='time-series'>time series</a>");
        if (this.widget.features["table-series"])
            td_mode.append("<a href='table-series'>table series</a>");
        td_mode.append("<a href='group-series'>group series</a>");
        td_mode.find("a").on("click", evt => acv_utils.forward_event(evt, evt => {
            this.widget.set_view_mode({mode: evt.currentTarget.getAttribute("href"), projection_no: this.projection_no()});
            this.set_current_mode();
        }));
        table.find("td.period > a").on("click", evt => acv_utils.forward_event(evt, evt => {
            this.widget.set_view_mode({period: evt.currentTarget.getAttribute("href"), projection_no: this.projection_no()});
            this.set_current_mode();
        }));
        table.find("td.shading > a").on("click", evt => acv_utils.forward_event(evt, evt => {
            this.widget.set_view_mode({shading: evt.currentTarget.getAttribute("href"), projection_no: this.projection_no()});
            this.set_current_mode();
        }));

        this.set_current_mode();
    }

    projection_title(projection, index) {
        return acv_utils.join_collapse([index === undefined ? null : "" + (index + 1) + ".", projection.s.toFixed(4), "&ge;" + (projection.m || "none"), projection.C ? "forced-col-bases" : null, projection.c]);
    }

    projection_no() {
        const select = this.content.find("table.a-view-dialog td.projection-chooser select");
        if (select.length > 0)
            return parseInt(select.val());
        else
            return 0;
    }

    set_current_mode() {
        const td_coloring = this.content.find("table.a-view-dialog td.coloring");
        td_coloring.find("a").removeClass("a-current");
        const coloring = this.widget.coloring.coloring();
        td_coloring.find(`a[href="${coloring}"]`).addClass("a-current");

        const td_mode = this.content.find("table.a-view-dialog td.mode");
        td_mode.find("a").removeClass("a-current");
        const mode = this.widget.view_mode.mode();
        td_mode.find(`a[href="${mode}"]`).addClass("a-current");

        const tr_period = this.content.find("table.a-view-dialog tr.time-series-period").hide();
        const tr_shading = this.content.find("table.a-view-dialog tr.shading").hide();
        const tr_selection = this.content.find("table.a-view-dialog tr.selection").hide();
        const tr_selection_results = this.content.find("table.a-view-dialog tr.selection-results").hide();
        tr_period.find("a").removeClass("a-current");
        tr_shading.find("a").removeClass("a-current");
        switch (mode) {
        case "time-series":
            tr_shading.show();
            tr_period.show();
            break;
        case "table-series":
            tr_shading.show();
            break;
        case "group-series":
            tr_shading.show();
            break;
        case "selection":
            tr_selection.show();
            tr_selection_results.show();
            this.handle_selection();
            break;
        case "projection":
        default:
            break;
        }
        tr_period.find(`a[href="${this.widget.view_mode.period()}"]`).addClass("a-current");
        tr_shading.find(`a[href="${this.widget.view_mode.shading()}"]`).addClass("a-current");

        this.handle_aa_position_entry();
        this.handle_group_series();
        this.show_legend();
    }

    show_legend() {
        const tr_legend = this.content.find("table.a-view-dialog tr.coloring-legend");
        const legend = this.widget.coloring.legend();
        if (legend) {
            const td_legend = tr_legend.find("td.coloring-legend").empty();
            td_legend.append("<table class='a-legend'><tr class='a-names'></tr><tr class='a-colors'></tr></table>");
            legend.map(entry => {
                td_legend.find("tr.a-names").append(`<td>${entry.name}</td>`);
                if (entry.color !== undefined && entry.color !== null)
                    td_legend.find("tr.a-colors").append(`<td><span class="a-color" style="background-color: ${entry.color}">__</span>${entry.count || ""}</td>`);
            });
            tr_legend.show();
        }
        else {
            tr_legend.hide();
        }
    }

    handle_aa_position_entry() {
        const coloring = this.widget.coloring.coloring();
        const tr_coloring_aa_pos = this.content.find("table.a-view-dialog tr.coloring-aa-pos");
        const input = tr_coloring_aa_pos.find("input");
        const hint = tr_coloring_aa_pos.find("a");
        input.off("keypress");
        hint.off("click");
        if (coloring === "aa_pos") {
            input.on("keypress", evt => {
                if (evt.charCode === 13) {
                    if (this.widget.coloring.set_positions(evt.currentTarget.value.split(/[^0-9]/).filter(entry => !!entry))) {
                        this.widget.set_view_mode({projection_no: this.projection_no()});
                        this.set_current_mode();
                    }
                }
            });
            hint.on("click", evt => acv_utils.forward_event(evt, evt => this._aa_positions_hint($(evt.currentTarget))));
            const positions = this.widget.coloring.positions();
            if (positions && positions.length)
                input.val(positions.join(" "));
            tr_coloring_aa_pos.show();
            window.setTimeout(() => input.focus(), 10);
        }
        else {
            tr_coloring_aa_pos.hide();
        }
    }

    _aa_positions_hint(parent) {
        const movable_window = new acv_toolkit.MovableWindow({
            title: "AA positions", parent: parent,
            classes: "coloring-aa-pos-hint",
            content_css: {width: "auto", height: "auto"}
        });
        const content = movable_window.content().empty();
        const compare_shannon = (e1, e2) => e2[1].shannon - e1[1].shannon;
        const compare_position = (e1, e2) => e1[0] - e2[0];
        let sort_by = "shannon";
        const make_table = tbl => {
            tbl.empty();
            Object.entries(this.widget.sequences_.per_pos).sort(sort_by === "shannon" ? compare_shannon : compare_position).forEach(entry => {
                let [pos, sh_count] = entry;
                if (Object.keys(sh_count.aa_count).length > 1) {
                    const row = $(`<tr><td class='a-pos'>${pos}</td></tr>`).appendTo(tbl);
                    const aa_order = Object.keys(sh_count.aa_count).sort((aa1, aa2) => sh_count.aa_count[aa2] - sh_count.aa_count[aa1]);
                    aa_order.forEach(aa => {
                        row.append(`<td class='a-aa'>${aa}</td><td class='a-count'>${sh_count.aa_count[aa]}</td>`);
                    });
                }
            });
        };
        const fill = () => {
            const sort_by_button = $("<a href='sort-by'></a>").appendTo(content);
            const sort_by_text = () => { sort_by_button.empty().append(sort_by === "shannon" ? "re-sort by position" : "re-sort by shannon index"); };
            const tbl = $("<table class='a-position-hint''></table>").appendTo(content);
            sort_by_button.on("click", evt => acv_utils.forward_event(evt, evt => {
                sort_by = sort_by === "shannon" ? "position" : "shannon";
                make_table(tbl);
                sort_by_text();
            }));
            console.log("per_pos", this.widget.sequences_.per_pos);
            sort_by_text();
            make_table(tbl);
            window.setTimeout(() => {
                if (content.height() > 300)
                    content.css("height", "300px");
            }, 10);
        };
        const wait_fill = () => {
            if (this.widget.sequences_ && typeof(this.widget.sequences_) !== "string")
                fill();
            else
                window.setTimeout(wait_fill, 100);
        };
        wait_fill();
    }

    handle_group_series() {
        const tr_group_series = this.content.find("table.a-view-dialog tr.group-series");
        tr_group_series.find("a").off("click");
        if (this.widget.view_mode.mode() === "group-series") {
            tr_group_series.show();
            this.show_group_series_data();
            this._make_uploader({button: tr_group_series.find("a[href='upload']"), drop_area: this.content.find("table.a-view-dialog")});
            this._make_downloader();
        }
        else {
            tr_group_series.hide();
        }
        this._make_exclusive_combined();
    }

    _make_uploader(args) {
        acv_utils.upload_json(args)
            .then(data => { this.show_group_series_uploaded_data(data); this._make_uploader(args); })
            .catch(err => { acv_toolkit.movable_window_with_error(err, args.button); this._make_uploader(args); });
    }

    _make_downloader() {
        const tr_group_series = this.content.find("table.a-view-dialog tr.group-series");
        const button_download_sample = tr_group_series.find("a[href='download']");
        button_download_sample.off("click");
        button_download_sample.on("click", evt => acv_utils.forward_event(evt, evt => {
            const chart = this.widget.data.c;
            const data = {
                "  version": "group-series-set-v1",
                "a": chart.a.map((antigen, ag_no) => Object.assign({"?no": ag_no}, antigen)),
                "s": chart.s.map((serum, sr_no) => Object.assign({"?no": sr_no + chart.a.length}, serum)),
                "group_sets": [{N: "set-1", line_color: "black", line_width: 1, groups: [{"N": "gr-1", line_color: "black", line_width: 1, root: 0, members: [0, 1, 2]}, {"N": "gr-2", root: 3, members: [3, 4, 5]}]}]
            };
            acv_utils.download_blob({data: data, blob_type: "application/json", filename: "group-series-sets.json"});
        }));

        const button_download_chart = tr_group_series.find("a[href='download-chart']");
        button_download_chart.off("click");
        if (this.widget.group_sets_) {
            button_download_chart.show().on("click", evt => acv_utils.forward_event(evt, evt => {
                const data = {"  version": "acmacs-ace-v1", "?created": `ace-view/201805 GroupSeries on ${new Date()}`, c: Object.assign({}, this.widget.data.c, {group_sets: this.widget.group_sets_})};
                acv_utils.download_blob({data: data, blob_type: "application/json", filename: "chart-with-group-series-sets.ace"});
            }));
        }
        else
            button_download_chart.hide();
    }

    _make_exclusive_combined() {
        const tr_group_series_combined = this.content.find("table.a-view-dialog tr.group-series-combined");
        tr_group_series_combined.find(".a-buttons a").off("click");
        if (this.widget.view_mode.mode() === "group-series" && this.widget.group_sets_) {
            tr_group_series_combined.show();
            const button_exclusive = tr_group_series_combined.find(".a-buttons a[href='exclusive']");
            const button_combined = tr_group_series_combined.find(".a-buttons a[href='combined']");
            const table_groups = tr_group_series_combined.find("table.a-groups");
            button_exclusive.on("click", evt => acv_utils.forward_event(evt, evt => {
                this.widget.view_mode.combined_mode("exclusive");
                this.widget.view_mode.set_page(0, true);
                this._make_exclusive_combined();
            }));
            button_combined.on("click", evt => acv_utils.forward_event(evt, evt => {
                this.widget.view_mode.combined_mode("combined");
                this.widget.view_mode.set_page(0, true);
                this._make_exclusive_combined();
            }));
            if (this.widget.view_mode.combined_mode() === "exclusive") {
                button_exclusive.addClass("a-current");
                button_combined.removeClass("a-current");
                table_groups.hide();
            }
            else {
                button_exclusive.removeClass("a-current");
                button_combined.addClass("a-current");
                table_groups.show();
            }
        }
        else {
            tr_group_series_combined.hide();
        }
    }

    _populate_table_groups(group_set) {
        const group_html = group_set.groups.map(group => {
            return `<tr><td class="a-checkbox"><input type="checkbox" name="${group.N}"></input></td><td class="a-name">${group.N}</td></tr>`;
        }).join("");
        const tbl = this.content.find("table.a-view-dialog tr.group-series-combined table.a-groups").empty().append(group_html);
        tbl.find("input").on("change", evt => acv_utils.forward_event(evt, evt => {
            if (evt.currentTarget.checked)
                this.widget.view_mode.add_combined_group(evt.currentTarget.name);
            else
                this.widget.view_mode.remove_combined_group(evt.currentTarget.name);
        }));
        this.widget.view_mode.groups_combined_.forEach(group_name => tbl.find(`input[name="${group_name}"]`).prop("checked", true));
    }

    show_group_series_data() {
        this._make_exclusive_combined();
        const chart = this.widget.data.c;
        if (!this.widget.group_sets_ && chart.group_sets)
            this.widget.group_sets_ = chart.group_sets;
        if (this.widget.group_sets_) {
            const group_sets = this.content.find("table.a-view-dialog tr.group-series .a-sets").empty();
            if (this.widget.group_sets_.length === 1) {
                const gs = this.widget.group_sets_[0];
                group_sets.append(`<a class='a-current' href='${gs.N}'>${gs.N}</a>`);
                group_sets.find("a").on("click", evt => acv_utils.forward_event(evt));
                this._populate_table_groups(gs);
                this.widget.view_mode.make_pages(gs);
            }
            else {
                for (let gs of this.widget.group_sets_) {
                    group_sets.append(`<a href='${gs.N}'>${gs.N}</a>`);
                }
                group_sets.find("a").on("click", evt => acv_utils.forward_event(evt, evt => {
                    const target = $(evt.currentTarget);
                    if (!target.hasClass("a-current")) {
                        group_sets.find("a").removeClass("a-current");
                        target.addClass("a-current");
                        const gs = this.widget.group_sets_[this.widget.group_sets_.findIndex(gs => gs.N === evt.currentTarget.getAttribute("href"))];
                        this._populate_table_groups(gs);
                        this.widget.view_mode.make_pages(gs);
                    }
                }));
                $(group_sets.find("a")[0]).addClass("a-current");
                this._populate_table_groups(this.widget.group_sets_[0]);
                this.widget.view_mode.make_pages(this.widget.group_sets_[0]);
            }
        }
    }

    show_group_series_uploaded_data(data) {
        try {
            this._check_group_sets(data);
            this._match_groups(data);
            this.show_group_series_data();
            this._make_downloader();
        }
        catch (err) {
            acv_toolkit.movable_window_with_error(err, this.content.find("table.a-view-dialog tr.group-series .a-label"));
        }
    }

    _match_groups(data) {
        const chart = this.widget.data.c;
        const point_to_point_ar = data.a.map((elt, no) => [no, chart.a.findIndex(antigen => acv_utils.objects_equal(elt, antigen, ["?no", "no", "C", "S", "c"]))])
              .concat(data.s.map((elt, no) => [no + data.a.length, chart.a.length + chart.s.findIndex(antigen => acv_utils.objects_equal(elt, antigen, ["?no", "no", "S"]))]));
        const point_to_point = point_to_point_ar.reduce((obj, entry) => { obj[entry[0]] = entry[1]; return obj; }, {});
        for (let gs of data.group_sets) {
            for (let grp of gs.groups) {
                if (grp.root !== undefined)
                    grp.root = point_to_point[grp.root];
                grp.members = grp.members.map(no => point_to_point[no]);
            }
        }
        this.widget.group_sets_ = data.group_sets;
    }

    _check_group_sets(data) {
        if (data["  version"] !== "group-series-set-v1")
            throw "Ivalid \"  version\" of the uploaded data";
        if (!data.a || !Array.isArray(data.a) || data.a.length === 0 || !data.s || !Array.isArray(data.s) || data.s.length === 0)
            throw "Invalid or empty \"a\" or \"s\" in the uploaded data";
        const number_of_points = data.a.length + data.s.length;
        if (!data.group_sets || !Array.isArray(data.group_sets) || data.group_sets.length === 0)
            throw "Invalid or empty \"group_sets\" in the uploaded data";
        data.group_sets.forEach((group_set, group_set_no) => {
            if (!group_set.N)
                throw "invalid \"N\" in \"group_set\" " + group_set_no;
            if (group_set.line_color !== undefined && typeof(group_set.line_color) !== "string")
                throw "invalid \"line_color\" in group_set " + group_set.N;
            if (group_set.line_width !== undefined && (typeof(group_set.line_width) !== "number" || group_set.line_width < 0))
                throw "invalid \"line_width\" in group_set " + group_set.N;
            if (!group_set.groups || !Array.isArray(group_set.groups) || group_set.groups.length === 0)
                throw "invalid or empty \"groups\" in \"group_set\" " + group_set_no + " \"" + group_set.N + "\"";
            let present_groups = {};
            group_set.groups.forEach((group, group_no) => {
                if (!group.N)
                    throw `invalid "N" in "group" ${group_no} of "group_set" "${group_set.N}"`;
                const orig_group_name = group.N;
                for (let copy_no = 2; present_groups[group.N] !== undefined; ++copy_no)
                    group.N = `${orig_group_name} (${copy_no})`;
                present_groups[group.N] = true;
                if (group.root !== undefined && (typeof(group.root) !== "number" || group.root < 0 || group.root >= number_of_points))
                    throw `invalid "root" in group "${group.N}" of "group_set" "${group_set.N}"`;
                if (!group.members || !Array.isArray(group.members) || group.members.length === 0)
                    throw `invalid "members" in group "${group.N}" of "group_set" "${group_set.N}"`;
                group.members.forEach(no => {
                    if (typeof(no) !== "number" || no < 0 || no >= number_of_points)
                        throw `invalid "members" element ${no} in "group" ${group_no} of "group_set" "${group_set.N}"`;
                });
                if (group.line_color !== undefined && typeof(group.line_color) !== "string")
                    throw "invalid \"line_color\" in group " + group.N;
                if (group.line_width !== undefined && (typeof(group.line_width) !== "number" || group.line_width < 0))
                    throw "invalid \"line_width\" in group " + group.N;
            });
        });
    }

    handle_selection() {
        const tr_selection = this.content.find("table.a-view-dialog tr.selection");
        const select = tr_selection.find("td.regex select");
        const input = tr_selection.find("td.regex input").val("");
        const selection_results = this.content.find("table.a-view-dialog tr.selection-results td.selection-results");
        const filter = () => {
            if (input.val() === "") {
                selection_results.find(".a-message").show();
                this.widget.view_mode.reset({selection_results: selection_results.find("table")});
            }
            else {
                selection_results.find(".a-message").hide();
                this.widget.view_mode.filter({regex: input.val(), subset: select.val(), selection_results: selection_results.find("table")});
            }
        };
        input.off("keypress").on("keypress", evt => { if (evt.charCode === 13) filter(); }).focus();
        select.off("change").on("change", filter);
        filter();
    }
}

// ----------------------------------------------------------------------

function show_antigen_serum_info_from_hidb(target, chart, invoking_node, shower) {
    if (shower) {
        const point_no = parseInt(target.attr("point_no"));
        const point_data = {virus_type: chart.i.V || (chart.i.S && chart.i.S.length > 0 && chart.i.S[0].V)};
        if (point_no < chart.a.length)
            point_data.antigen = chart.a[point_no];
        else
            point_data.serum = chart.s[point_no - chart.a.length];
        shower(point_data, invoking_node);
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
