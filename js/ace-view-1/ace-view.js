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
            view_mode: {mode: "best-projection"},
            coloring: "default",
            point_scale: 5,
            point_info_on_hover_delay: 500,
            mouse_popup_offset: {left: 10, top: 20},
            canvas_size: {width: 0, height: 0},
            min_viewport_size: 1.0,
            show_as_background: {fill: "#E0E0E0", outline: "#E0E0E0"},
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

// const BurgerMenu_html = "\
// <ul class='a-level-0'>\
//   <!-- <li class='a-disabled'><a href='search'>Search</a></li> -->\
//   <li><a href='#'>Coloring</a><span class='a-right-arrow'>&#9654;</span>\
//     <ul class='a-level-1'>\
//       <li class='a-disabled' name='clades'><a href='clades'>by Clade</a></li>\
//       <li class='a-disabled' name='continents'><a href='continents'>by Geography</a></li>\
//       <li                    name='color-by-default'><a href='color-by-default'>reset to default</a></li>\
//     </ul>\
//   </li>\
//   <li><a href='#'>View</a><span class='a-right-arrow'>&#9654;</span>\
//     <ul class='a-level-1'>\
//       <li class='a-disabled' name='best-projection'><a href='best-projection'>Best projection</a></li>\
//       <li class='a-separator'></li>\
//       <li class='a-disabled' name='time-series'><a href='time-series' args='{\"period\":\"month\", \"shading\": \"shade\"}'>Time series, monthly (shade)</a></li>\
//       <li class='a-disabled' name='time-series'><a href='time-series' args='{\"period\":\"month\", \"shading\": \"hide\"}' >Time series, monthly</a></li>\
//       <li class='a-disabled' name='time-series'><a href='time-series' args='{\"period\":\"month\", \"shading\": \"grey\"}' >Time series, monthly (grey)</a></li>\
//       <li class='a-disabled' name='time-series'><a href='time-series' args='{\"period\":\"year\",  \"shading\": \"shade\"}'>Time series, yearly (shade)</a></li>\
//       <li class='a-disabled' name='time-series'><a href='time-series' args='{\"period\":\"season\",\"shading\": \"shade\"}'>Time series, Winter/Summer (shade)</a></li>\
//       <li class='a-separator'></li>\
//       <li class='a-disabled' name='table-series'><a href='table-series'>Table series</a></li>\
//       <li class='a-disabled' name='table-series-shade'><a href='table-series-shade'>Table series (shade)</a></li>\
//       <!-- <li class='a-disabled' name='clade-series'><a href='clade-series'>Clade series</a></li> -->\
//     </ul>\
//   </li>\
//   <li><a href='#'>Download</a><span class='a-right-arrow'>&#9654;</span>\
//     <ul class='a-level-1'>\
//       <li class='a-disabled' name='download_pdf'><a href='download_pdf'>PDF</a></li>\
//       <li class='a-separator'></li>\
//       <li class='a-disabled' name='download_ace'><a href='download_ace'>ace</a></li>\
//       <li class='a-disabled' name='download_save'><a href='download_save'>Lispmds Save</a></li>\
//       <li class='a-separator'></li>\
//       <li class='a-disabled' name='download_layout_plain'><a href='download_layout_plain'>Layout (plain text)</a></li>\
//       <li class='a-disabled' name='download_layout_csv'><a href='download_layout_csv'>Layout (csv)</a></li>\
//       <li class='a-separator'></li>\
//       <li class='a-disabled' name='download_table_map_distances_plain'><a href='download_table_map_distances_plain'>Table vs. Map Distances (plain text)</a></li>\
//       <li class='a-disabled' name='download_table_map_distances_csv'><a href='download_table_map_distances_csv'>Table vs. Map Distances (csv)</a></li>\
//       <li class='a-disabled' name='download_error_lines'><a href='download_error_lines'>Error lines (csv)</a></li>\
//       <li class='a-disabled' name='download_distances_between_all_points_plain'><a href='download_distances_between_all_points_plain'>Distances Between All Points (plain text)</a></li>\
//       <li class='a-disabled' name='download_distances_between_all_points_csv'><a href='download_distances_between_all_points_csv'>Distances Between All Points (csv)</a></li>\
//     </ul>\
//   </li>\
//   <li><a href='table'>Table</a></li>\
//   <li><a href='raw'>Raw</a></li>\
//   <li class='a-separator'></li>\
//   <li><a href='help'>Help</a></li>\
// </ul>\
// ";

const API_Features = [
    "download_pdf", "download_ace", "download_save",
    "download_layout_plain", "download_layout_csv",
    "download_table_map_distances_plain", "download_table_map_distances_csv", "download_error_lines",
    "download_distances_between_all_points_plain", "download_distances_between_all_points_csv"
];

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
        const destroy = () => this.destroy();

        // this.find("a[href='raw']").on("click", evt => acv_utils.forward_event(evt, () => acv_toolkit.movable_window_with_json(this.parent.data, evt.currentTarget, "map view raw data"), destroy));
        this.find("a[href='raw']").on("click", evt => acv_utils.forward_event(evt, () => {
            console.log("amw raw data", this.parent.data);
            // alert("Please see raw data in the console");
        }, destroy));

        this.find("a[href='search']").on("click", evt => acv_utils.forward_event(evt, () => console.log("search"), destroy));
        this.find("a[href='clades']").on("click", evt => acv_utils.forward_event(evt, () => this.parent.set_coloring("clade"), destroy));
        this.find("a[href='continents']").on("click", evt => acv_utils.forward_event(evt, () => this.parent.set_coloring("continent"), destroy));
        this.find("a[href='color-by-default']").on("click", evt => acv_utils.forward_event(evt, () => this.parent.set_coloring("default"), destroy));
        this.find("a[href='best-projection']").on("click", evt => acv_utils.forward_event(evt, () => this.parent.set_view_mode({mode: "best-projection"}), destroy));
        this.find("a[href='time-series']").on("click", evt => acv_utils.forward_event(evt, () => this.parent.set_view_mode(Object.assign({mode: "time-series"}, JSON.parse(evt.target.getAttribute("args")))), destroy));
        this.find("a[href='table-series']").on("click", evt => acv_utils.forward_event(evt, () => this.parent.set_view_mode({mode: "table-series", shading: "hide"}), destroy));
        this.find("a[href='table-series-shade']").on("click", evt => acv_utils.forward_event(evt, () => this.parent.set_view_mode({mode: "table-series", shading: "shade"}), destroy));
        this.find("a[href='clade-series']").on("click", evt => acv_utils.forward_event(evt, () => this.parent.set_view_mode({mode: "series-clade", shading: "shade"}), destroy));
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
        acv_utils.load_css('/js/ad/map-draw/ace-view-1/ace-view.css');
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
            this.title();
            this.resize_title();
        }
    }

    set_view_mode(args) {
        switch (args.mode) {
        case "time-series":
            switch (args.shading) {
            case "hide":
                this.view_mode = new DrawingMode_TimeSeries(this, args);
                break;
            case "grey":
                this.view_mode = new DrawingMode_TimeSeriesGrey(this, args);
                break;
            case "shade":
            default:
                this.view_mode = new DrawingMode_TimeSeriesShade(this, args);
                break;
            }
            break;
        case "table-series":
            switch (args.shading) {
            case "hide":
                this.view_mode = new DrawingMode_TableSeries(this);
                break;
            case "shade":
            default:
                this.view_mode = new DrawingMode_TableSeriesShade(this);
                break;
            }
            break;
        case "best-projection":
        default:
            this.view_mode = new DrawingMode_Best_Projection(this);
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
        this.title_element().empty().append(this.view_mode.title({title_fields: this.options.title_fields}));
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
                        acv_utils.forward_event(evt, evt => show_antigen_serum_info_from_hidb($(evt.target), this.data.c, this.options.point_on_click));
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
                return `<li><a href="#show-info-on-this-name" point_no="${point_entry.no}" point_name="${point_entry.name}">${point_entry.name}</a></li>`;
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
        if (this.data)
            return new ViewDialog({widget: this, parent: $(parent), chart: this.data.c, id: this.dialog_id("view")});
        else
            return null;
    }

    external_api(api_feature) {
        switch (api_feature) {
        case "download_pdf":
            this.options.api.download_pdf({drawing_order_background: this.view_mode.drawing_order_background(), drawing_order: this.view_mode.drawing_order(), projection_no: this.view_mode.projection_no(), styles: this.view_mode.styles(), point_scale: this.view_mode.point_scale()});
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
        return this.options.api.get_sequences().then(data => console.log("AntigenicMapWidget::sequences", data));
    }
}

// ----------------------------------------------------------------------

class DrawingMode_Base
{
    constructor(widget) {
        this.widget = widget;
        if (widget) {
            this.projection_no_ = widget.options.projection_no;
            widget.show_title_arrows(null, null);
        }
    }

    shading() {
        return null;
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
            this.widget.surface.points({drawing_order: this.widget.coloring.drawing_order(drawing_order_background),
                                    layout: chart.P[this.projection_no()].l,
                                    transformation: new ace_surface.Transformation(chart.P[this.projection_no()].t),
                                    styles: this.widget.coloring.styles(),
                                    point_scale: this.point_scale(),
                                    show_as_background: this.show_as_background()});
        this.widget.surface.points({drawing_order: this.widget.coloring.drawing_order(this.drawing_order()),
                                    layout: chart.P[this.projection_no()].l,
                                    transformation: new ace_surface.Transformation(chart.P[this.projection_no()].t),
                                    styles: this.widget.coloring.styles(),
                                    point_scale: this.point_scale()});
    }

    projection_no() {
        return this.projection_no_;
    }

    point_scale() {
        return this.widget.parameters.point_scale;
    }

    show_as_background() {
        return null;
    }

    drawing_order_background() {
        return [];
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

    show_as_background() {
        return this.widget.options.show_as_background;
    }

    drawing_order() {
        return this.drawing_order_;
    }

    drawing_order_background() {
        return this.drawing_order_background_;
    }

}

// ----------------------------------------------------------------------

class DrawingMode_TimeSeries extends DrawingMode_Series
{
    constructor(widget, args) {
        super(widget);
        this.period_ = (args && args.period) || "month";
        this.make_pages();
        this.set_page(this.pages.length - 1);
    }

    mode() {
        return "time-series";
    }

    shading() {
        return "hide";
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

class DrawingMode_TimeSeriesGrey extends DrawingMode_TimeSeries
{
    shading() {
        return "grey";
    }

    make_drawing_order() {
        const page_period_name = this.pages[this.page_no];
        const in_page = antigen => this.antigen_period_name(antigen) === page_period_name;
        const antigens = this.widget.data.c.a;
        this.drawing_order_ = [];
        this.drawing_order_background_ = [];
        for (let point_no of this.widget.data.c.p.d) {
            if (point_no < antigens.length && in_page(antigens[point_no]))
                this.drawing_order_.push(point_no);
            else
                this.drawing_order_background_.push(point_no);
        }
    }
}

// ----------------------------------------------------------------------

class DrawingMode_TimeSeriesShade extends DrawingMode_TimeSeriesGrey
{
    shading() {
        return "shade";
    }

    show_as_background() {
        return {shade: this.widget.options.show_as_background_shade};
    }
}

// ----------------------------------------------------------------------

class DrawingMode_TableSeries extends DrawingMode_Series
{
    constructor(widget, args) {
        super(widget);
        this.make_pages();
        this.set_page(this.pages.length - 1);
    }

    mode() {
        return "table-series";
    }

    shading() {
        return "hide";
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
    }

}

// ----------------------------------------------------------------------

class DrawingMode_TableSeriesShade extends DrawingMode_TableSeries
{
    shading() {
        return "shade";
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
        this.drawing_order_background_ = this.widget.data.c.p.d.filter(point_no => !point_in_layer(point_no));
    }

    show_as_background() {
        return {shade: this.widget.options.show_as_background_shade};
    }
}

// ----------------------------------------------------------------------

// const view_mode_selector_data = {
//     "best-projection": DrawingMode_Best_Projection,
//     "time-series": DrawingMode_TimeSeries,
//     "time-series-shade": DrawingMode_TimeSeriesShade,
//     "time-series-grey": DrawingMode_TimeSeriesGrey,
//     "table-series": DrawingMode_TableSeries,
//     "table-series-shade": DrawingMode_TableSeriesShade,
//     null: DrawingMode_Best_Projection
// };

// function select_view_mode(mode, widget) {
//     return new (view_mode_selector_data[mode] || view_mode_selector_data[null])(widget);
// }

// ----------------------------------------------------------------------

class Coloring_Base
{
    constructor(widget) {
        this.widget = widget;
    }

    drawing_order(original_drawing_order) {
        return original_drawing_order;
    }

    legend() {
        return null;
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

const sGREY = "#c0c0c0";

class Coloring_WithAllStyles extends Coloring_Base
{
    constructor(widget) {
        super(widget);
        const chart = this.widget.data.c;
        const egg_passage = (style, index) => {
            if (index < chart.a.length && (!style.a || style.a === 1.0) && chart.a[index].S && chart.a[index].S.indexOf("E") >= 0)
                style.a = 0.75;
            return style;
        };
        const all_styles = chart.p.p.map(style_no => Object.assign({}, chart.p.P[style_no])).map(egg_passage);
        this.styles_ = {index: acv_utils.array_of_indexes(all_styles.length), styles: all_styles};
        chart.s.forEach((serum, serum_no) => {
            delete this.styles_.styles[serum_no + chart.a.length].F;
            this.styles_.styles[serum_no + chart.a.length].O = sGREY;
        });
    }

    styles() {
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
        const chart = this.widget.data.c;
        let continent_count = {};
        chart.a.forEach((antigen, antigen_no) => {
            this.styles_.styles[antigen_no].F = continent_colors[antigen.C];
            continent_count[antigen.C] = (continent_count[antigen.C] || 0) + 1;
        });
        this.continent_count = Object.keys(continent_count)
            .map(continent => { return {name: continent, count: continent_count[continent], color: continent_colors[continent]}; })
            .sort((e1, e2) => e2.count - e1.count);
    }

    coloring() {
        return "continent";
    }

    drawing_order(original_drawing_order) {
        // order: sera, most popular continent, ..., lest popular continent
        const continent_order = this.continent_count.map(entry => entry.name);
        const chart = this.widget.data.c;
        const ranks = Array.apply(null, {length: chart.a.length}).map((_, ag_no) => continent_order.indexOf(chart.a[ag_no].C) + 10).concat(Array.apply(null, {length: chart.s.length}).map(_ => 0));
        return original_drawing_order.slice(0).sort((p1, p2) => ranks[p1] - ranks[p2]);
    }

    legend() {
        return this.continent_count.map(entry => Object.assign(entry, {name: continent_name_for_legend[entry.name]}));
    }
}

// ----------------------------------------------------------------------

const sCladeColors = {
    "3C3": "#6495ed",
    "3C3A": "#00ff00",
    "3C3B": "#0000ff",
    "3C2A": "#ff0000",
    "3C2A1": "#8b0000",
    "3C2A1A": "#8b0000",
    "3C2A1B": "#8b0000",
    "3C2A2": "#8b4040",
    "3C2A3": "#8b4000",
    "3C2A4": "#8b0040",
    "6B1": "#0000ff",
    "6B2": "#ff0000",
    "1": "#0000ff",
    "1A": "#6495ed",
    "1B": "#ff0000",
    "DEL2017": "#de8244",
    "TRIPLEDEL2017": "#bf3eff",
    "Y2": "#6495ed",
    "Y3": "#ff0000",
    "SEQUENCED": "#ffa500",
    "NO-GLY": "#ffa500",
    // "GLY": "#ff00a5",
    "GLY": "#ffa500",
    "": sGREY,
    undefined: sGREY,
    null: sGREY
};

class Coloring_Clade extends Coloring_WithAllStyles
{
    constructor(widget) {
        super(widget);
        this._make_antigens_by_clade();
        this._make_styles();
    }

    coloring() {
        return "clade";
    }

    _make_antigens_by_clade() {
        this.clade_to_number_of_antigens = {};
        this.clade_for_antigen = this.widget.data.c.a.map(antigen => {
            const clade_sorting_key = clade => (clade === "GLY" || clade === "NO-GLY" || clade === "SEQUENCED") ? 0 : clade.length;
            const clades = (antigen.c || []).sort((a, b) => clade_sorting_key(b) - clade_sorting_key(a));
            let clade = clades.length > 0 ? clades[0] : "";
            if (clade === "GLY" || clade === "NO-GLY")
                clade = "SEQUENCED";
            this.clade_to_number_of_antigens[clade] = (this.clade_to_number_of_antigens[clade] || 0) + 1;
            return clade;
        });
        this.clade_order = Object.keys(this.clade_to_number_of_antigens).sort((a, b) => this._clade_rank(a) - this._clade_rank(b));
        this.point_rank = this.clade_for_antigen.map(clade => this.clade_order.indexOf(clade)).concat(Array.apply(null, {length: this.widget.data.c.s.length}).map(() => -2));
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
        this.clade_for_antigen.forEach((clade, antigen_no) => this.styles_.styles[antigen_no].F = sCladeColors[clade]);
    }

    drawing_order(original_drawing_order) {
        // order: sera, not sequenced, sequenced without clade, clade with max number of antigens, ..., clade with fewer antigens
        return original_drawing_order.slice(0).sort((p1, p2) => this.point_rank[p1] - this.point_rank[p2]);
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
        this._reset_styles();
        this._make_styles();
        widget.sequences().then(data => console.log("Coloring_AAPos::constructor", data));
    }

    coloring() {
        return "aa_pos";
    }

    _make_styles() {
    }

    drawing_order(original_drawing_order) {
        // order: sera, not sequenced, "clade" with max number of antigens, ..., "clade" with fewer antigens
        return original_drawing_order;
    }

    legend() {
        return [{name: "loading, please wait"}];
    }

    _reset_styles() {
        this.widget.data.c.a.forEach((antigen, antigen_no) => {
            this.styles_.styles[antigen_no].F = this.styles_.styles[antigen_no].O = sGREY;
        });
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
        // console.log("AntigenicTable_populate", args);
        this.widget = args.widget;
        this.chart = args.chart;
        if (this.chart.a.length < 200000) {
            this.div = $("<table class='antigenic-table'></table>").appendTo(args.parent);
            this.make_sera();
            this.make_antigens();
            this.show_antigen_serum_info();
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
        this.div.find("a[title]").on("click", evt => acv_utils.forward_event(evt, () => show_antigen_serum_info_from_hidb($(evt.target), this.chart, this.widget.options.point_on_click)));
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
<table>\
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
    <td class='coloring-aa-pos'><input type='text'></input></td>\
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
  <tr class='shading'>\
    <td class='a-label'>Shading</td>\
    <td class='shading'>\
      <a href='hide'>legacy</a>\
      <a href='shade'>shade</a>\
      <a href='grey'>grey</a>\
    </td>\
  </tr>\
</table>\
";

class ViewDialog
{
    constructor(args) {
        this.widget = args.widget;
        const movable_window = new acv_toolkit.MovableWindow({title: "View", parent: args.widget.div.find("canvas"), classes: "view-dialog-movable", content_css: {width: "auto", height: "auto"}, id: args.id});
        this.content = movable_window.content();
        if (movable_window.content().find("table.view-dialog").length === 0)
            this.populate({content: this.content, chart: args.chart});
    }

    populate(args) {
        //console.log("features", this.widget.features);

        const table = $(ViewDialog_html).appendTo(args.content);
        if (args.chart.P.length === 0)
            table.find("td.projection-chooser").append("<div class='a-error'>None</div>");
        else if (args.chart.P.length === 1)
            table.find("td.projection-chooser").append(`<div>${this.projection_title(args.chart.P[0])}</div>`);
        else {
            const entries = args.chart.P.map((prj, index) => `<option value="${index}">${this.projection_title(prj, index)}</option>`).join();
            const select = $(`<select>${entries}</select>`).appendTo(table.find("td.projection-chooser"));
            select.on("change", evt => acv_utils.forward_event(evt, evt => {
                this.widget.set_view_mode({mode: this.widget.view_mode.mode(), shading: this.widget.view_mode.shading(), period: this.widget.view_mode.period(), projection_no: this.projection_no()});
            }));
        }

        const td_coloring = table.find("td.coloring");
        td_coloring.append("<a href='default'>default</a>");
        if (this.widget.features["clades"])
            td_coloring.append("<a href='clade'>by clade</a><a href='aa_pos'>by AA at pos</a>");
        if (this.widget.features["continents"])
            td_coloring.append("<a href='continent'>by geography</a>");
        td_coloring.find("a").on("click", evt => acv_utils.forward_event(evt, evt => {
            this.widget.set_coloring(evt.currentTarget.getAttribute("href"));
            this.set_current_mode();
        }));

        const td_mode = table.find("td.mode");
        td_mode.append("<a href='projection'>projection</a>");
        if (this.widget.features["time-series"])
            td_mode.append("<a href='time-series'>time series</a>");
        if (this.widget.features["table-series"])
            td_mode.append("<a href='table-series'>table series</a>");
        td_mode.find("a").on("click", evt => acv_utils.forward_event(evt, evt => {
            this.widget.set_view_mode({mode: evt.currentTarget.getAttribute("href"), shading: "shade", projection_no: this.projection_no()});
            this.set_current_mode();
        }));
        table.find("td.period > a").on("click", evt => acv_utils.forward_event(evt, evt => {
            this.widget.set_view_mode({mode: this.widget.view_mode.mode(), shading: this.widget.view_mode.shading(), period: evt.currentTarget.getAttribute("href"), projection_no: this.projection_no()});
            this.set_current_mode();
        }));
        table.find("td.shading > a").on("click", evt => acv_utils.forward_event(evt, evt => {
            this.widget.set_view_mode({mode: this.widget.view_mode.mode(), shading: evt.currentTarget.getAttribute("href"), period: this.widget.view_mode.period(), projection_no: this.projection_no()});
            this.set_current_mode();
        }));

        this.set_current_mode();
    }

    projection_title(projection, index) {
        return acv_utils.join_collapse([index === undefined ? null : "" + (index + 1) + ".", projection.s.toFixed(4), "&ge;" + (projection.m || "none"), projection.C ? "forced-col-bases" : null, projection.c]);
    }

    projection_no() {
        const select = this.content.find("table td.projection-chooser select");
        if (select.length > 0)
            return parseInt(select.val());
        else
            return 0;
    }

    set_current_mode() {
        const td_coloring = this.content.find("table td.coloring");
        td_coloring.find("a").removeClass("a-current");
        const coloring = this.widget.coloring.coloring();
        td_coloring.find(`a[href="${coloring}"]`).addClass("a-current");
        const tr_coloring_aa_pos = this.content.find("table tr.coloring-aa-pos").hide();
        if (coloring === "aa_pos")
            tr_coloring_aa_pos.show();

        const td_mode = this.content.find("table td.mode");
        td_mode.find("a").removeClass("a-current");
        const mode = this.widget.view_mode.mode();
        td_mode.find(`a[href="${mode}"]`).addClass("a-current");

        const tr_period = this.content.find("table tr.time-series-period").hide();
        const tr_shading = this.content.find("table tr.shading").hide();
        tr_period.find("a").removeClass("a-current");
        tr_shading.find("a").removeClass("a-current");
        switch (mode) {
        case "time-series":
            tr_shading.show();
            tr_period.show();
            tr_shading.find("a[href='grey']").show();
            break;
        case "table-series":
            tr_shading.show();
            tr_shading.find("a[href='grey']").hide();
            break;
        case "projection":
        default:
            break;
        }
        tr_period.find(`a[href="${this.widget.view_mode.period()}"]`).addClass("a-current");
        tr_shading.find(`a[href="${this.widget.view_mode.shading()}"]`).addClass("a-current");
        this.show_legend();
    }

    show_legend() {
        const tr_legend = this.content.find("table tr.coloring-legend");
        const legend = this.widget.coloring.legend();
        if (legend) {
            console.log("legend", legend);
            const td_legend = tr_legend.find("td.coloring-legend").empty();
            td_legend.append("<table><tr class='a-names'></tr><tr class='a-colors'></tr></table>");
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
}

// ----------------------------------------------------------------------

function show_antigen_serum_info_from_hidb(target, chart, shower) {
    if (shower) {
        const point_no = parseInt(target.attr("point_no"));
        const point_data = {virus_type: chart.i.V || (chart.i.S && chart.i.S.length > 0 && chart.i.S[0].V)};
        if (point_no < chart.a.length)
            point_data.antigen = chart.a[point_no];
        else
            point_data.serum = chart.s[point_no - chart.a.length];
        shower(point_data, target);
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
