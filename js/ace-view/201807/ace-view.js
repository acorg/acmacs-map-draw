import * as av_utils from "./utils.js";
import * as av_surface from "./surface.js";
import * as av_toolkit from "./toolkit.js";
import * as av_point_style from "./point-style.js";

av_utils.load_css('/js/ad/map-draw/ace-view/201807/ace-view.css');

// ----------------------------------------------------------------------

const AntigenicMapWidget_left_arrow = "&#x21E6;"; // "&#x27F8;";
const AntigenicMapWidget_right_arrow = "&#x21E8;"; // "&#x27F9;";
const AntigenicMapWidget_burger = "&#x2630;";

const AntigenicMapWidget_default_options = {
    canvas_size: null,          // auto-size

    projection_no: 0,
    view_mode: {mode: "projection"},
    coloring: "default",
    point_scale: 5,
    point_info_on_hover_delay: 500,
    mouse_popup_offset: {left: 10, top: 20},
    min_viewport_size: 1.0,
    show_as_background: {fill: av_toolkit.sLIGHTGREY, outline: av_toolkit.sLIGHTGREY},
    show_as_background_shade: 0.8,
    title_fields: ["stress", "antigens", "sera", "name", "lab", "virus_type", "assay", "date", "min_col_basis", "tables"],
    callbacks: {
        load_failure: null
    }
};

const AntigenicMapWidget_content_html = `\
<table>\
  <tr>\
    <td class='av-title'>\
      <div class='av-left'>\
        <div class='av-arrow av-left-arrow av-unselectable'>${AntigenicMapWidget_left_arrow}</div>\
      </div>\
      <div class='av-title-title'><span></span></div>\
      <div class='av-right'>\
        <div class='av-arrow av-right-arrow av-unselectable'>${AntigenicMapWidget_right_arrow}</div>\
        <div class='av-burger'>${AntigenicMapWidget_burger}</div>\
      </div>\
    </td>\
  </tr>\
  <tr>\
    <td class='av-canvas'>\
      <div><canvas></canvas></div>\
    </td>\
  </tr>\
</table>\
`;

const AntigenicMapWidget_burger_menu_html = "\
    <ul>\
      <li menu='view'>View ...</li>\
      <li>\
        Download\
        <ul>\
          <li class='av-menu-disabled' menu='download_pdf'>PDF</li>\
          <li class='av-menu-separator'></li>\
          <li class='av-menu-disabled' menu='download_ace'>ace</li>\
          <li class='av-menu-disabled' menu='download_save'>Lispmds Save</li>\
        </ul>\
      </li>\
      <li class='av-menu-disabled' menu='table'>Table</li>\
      <li class='av-menu-disabled' menu='raw'>Raw</li>\
      <li class='av-menu-separator'></li>\
      <li class='av-menu-disabled' menu='help'>Help</li>\
    </ul>\
";

export class AntigenicMapWidget
{
    constructor(div, data, options={}) { // options: {view_mode: {mode: "table-series"}, coloring: "original", title_fields: [], api: object_providing_external_api}
        this.div_ = $(div);
        this.options_ = Object.assign({}, AntigenicMapWidget_default_options, options);
        this.div_.addClass("amw201807").attr("amw201805_id", new_id()).append(AntigenicMapWidget_content_html);
        this.title_ = new AntigenicMapTitle(this, this.div_.find(".av-title"));
        this.viewer_ = new MapViewer(this, this.div_.find("canvas"));
        this._viewer_size();
        this._make_burger_menu();
        this._load_and_draw(data);
    }

    draw_data(data=undefined) {
        if (this.loading_message_) {
            this.loading_message_.remove();
            delete this.loading_message_;
        }
        this.viewer_.start(data);
        this.viewer_.draw();
        this.title_.update();
    }

    _load_and_draw(data) {
        const loaded = data => this.draw_data(data);
        const failed = deffered => {
            const args = {source: data, statusText: deffered.statusText};
            console.error("failed to load antigenic map data from ", args);
            if (this.options_.callbacks && this.options_.callbacks.load_failure)
                this.options_.callbacks.load_failure(args);
        };
        if (typeof(data) === "string" && RegExp("(\\.ace|\\?acv=ace)$").test(data)) {
            this._loading_message();
            $.getJSON(data).done(loaded).fail(failed);
        }
        else if (typeof(data) === "function" && data.constructor.name === 'AsyncFunction') {
            this._loading_message();
            data().done(loaded).fail(failed);
        }
        else if (typeof(data) === "object" && (data.version || data["  version"]) === "acmacs-ace-v1") {
            loaded(data);
        }
        else {
            console.error("unrecognized", data);
        }
    }

    _make_burger_menu() {
        this.burger_menu_ = new av_toolkit.BurgerMenu({menu: $(AntigenicMapWidget_burger_menu_html).appendTo("body"), trigger: this.div_.find(".av-burger"), callback: item => {
            if (item === "view") {
            }
            else
                console.log("burger-menu", item);
        }});
    }

    _loading_message() {
        const canvas = this.div_.find("canvas");
        this.loading_message_ = $("<div class='av-loading-message'>Loading data, please wait ...</div>")
            .appendTo(canvas.parent())
            .css({top: parseFloat(canvas.css("padding-top")) + canvas.height() / 2, width: canvas.width()});
    }

    _viewer_size() {
        if (this.options_.canvas_size)
            this.viewer_.resize(this.options_.canvas_size);
        else
            this.viewer_.resize(Math.max(Math.min($(window).width() - 100, $(window).height()) - 60, 200));
    }
}

// ----------------------------------------------------------------------

class AntigenicMapTitle
{
    constructor(widget, element) {
        this.widget_ = widget;
        this.div_ = element;
    }

    update() {
        const viewing = this.widget_.viewer_.viewing_;
        this._span().empty().append(viewing.title({title_fields: this.widget_.options_.title_fields}));
        this.resize();
        this.popup_on_hovering_title(viewing.title_box());
    }

    resize() {
        const title_element = this.div_.find("> .av-title-title");
        const title_left = this.div_.find("> .av-left");
        const title_left_width = title_left.is(":visible") ? title_left.outerWidth(true) : 0;
        const title_right = this.div_.find("> .av-right");
        const title_right_width = title_right.is(":visible") ? title_right.outerWidth(true) : 0;
        title_element.css("width", this.widget_.viewer_.surface_.width() - title_left_width - title_right_width - (title_element.outerWidth(true) - title_element.width()));
    }

    popup_on_hovering_title(content) {
        if (content) {
            const title = this._span().off("mouseenter mouseleave");
            const delay = this.widget_.options_.point_info_on_hover_delay;
            let popup_events = false;
            const hide_popup = () => {
                av_toolkit.mouse_popup_hide().off("mouseenter mouseleave");
                popup_events = false;
            };
            const mouse_leave = () => {
                window.clearTimeout(this.mouse_popup_timeout_id);
                this.mouse_popup_timeout_id = window.setTimeout(hide_popup, delay);
            };
            title.on("mouseenter", evt => {
                window.clearTimeout(this.mouse_popup_timeout_id);
                this.mouse_popup_timeout_id = window.setTimeout(() => {
                    const popup_element = av_toolkit.mouse_popup_show(content, title, {left: 30, top: title.outerHeight()});
                    if (!popup_events) {
                        popup_element.on("mouseenter", () => window.clearTimeout(this.mouse_popup_timeout_id));
                        popup_element.on("mouseleave", mouse_leave);
                        popup_events = true;
                    }
                }, delay);
            });
            title.on("mouseleave", mouse_leave);
        }
    }

    arrows(left_callback, right_callback) {
        const left_arrow = this.div_.find(".av-left-arrow");
        left_arrow.off("click");
        if (left_callback)
            left_arrow.show().on("click", left_callback);
        else
            left_arrow.hide();
        const right_arrow = this.div_.find(".av-right-arrow");
        right_arrow.off("click");
        if (right_callback)
            right_arrow.show().on("click", right_callback);
        else
            right_arrow.hide();
    }

    _span() {
        return this.div_.find("> .av-title-title > span");
    }
}

// ----------------------------------------------------------------------

class MapViewer
{
    constructor(widget, canvas) {
        this.widget_ = widget;
        this.surface_ = new av_surface.Surface(canvas);
    }

    start(data) {
        this._make_coloring_modes(data);
        this._make_viewing_modes(data);
        this.coloring("original");
        this.viewing("all");
        this.projection(0);
        this._bind();
    }

    draw() {
        this._hide_point_info();
        this.surface_.reset();
        this.surface_.grid();
        this.surface_.border();
        this.viewing_.draw(this.surface_, this.coloring_);
        // this.widget_.update_view_dialog();
    }

    resize(width_diff) {
        this.surface_.resize(width_diff);
        this.widget_.title_.resize();
        if (this.viewing_)
            this.draw();
    }

    coloring(mode_name, redraw=false) {
        this.coloring_ = this.coloring_modes_.find(mode => mode.name() === mode_name) || this.coloring_modes_.find(mode => mode.name() === "original");
        if (redraw)
            this.draw();
    }

    viewing(mode_name, redraw=false) {
        this.viewing_ = this.viewing_modes_.find(mode => mode.name() === mode_name) || this.viewing_modes_.find(mode => mode.name() === "all");
        if (this.projection_no_ !== undefined)
            this.viewing_.projection(this.projection_no_);
        if (redraw)
            this.draw();
    }

    projection(projection_no) {
        this.projection_no_ = projection_no;
        this.viewing_.projection(this.projection_no_);
    }

    _drawing_order() {
        return this.coloring_.drawing_order(this.viewing_.drawing_order());
    }

    _bind() {
        this.surface_.add_resizer(width_diff => this.resize(width_diff));

        let mousemove_timeout_id = undefined;
        this.surface_.canvas_.on("mousemove", evt => {
            window.clearTimeout(mousemove_timeout_id);
            const mouse_offset = this.surface_.mouse_offset(evt);
            mousemove_timeout_id = window.setTimeout(() => this._show_point_info(mouse_offset, this.surface_.find_points_at_pixel_offset(mouse_offset, this._drawing_order())), this.widget_.options_.point_info_on_hover_delay, evt);
        });
        this.surface_.canvas_.on("mouseleave", evt => window.clearTimeout(mousemove_timeout_id));

        this.surface_.canvas_.on("wheel DOMMouseScroll", evt => av_utils.forward_event(evt, evt => {
            if (evt.shiftKey) // Shift-Wheel -> point_scale
                this.surface_.point_scale_with_mouse(evt);
            else if (evt.altKey) // Alt-Wheel -> zoom
                this.surface_.zoom_with_mouse(evt);
            else if (evt.ctrlKey) // Ctrl-Wheel -> rotate
                this.surface_.rotate_with_mouse(evt);
            this.draw();
        }));

        this.surface_.canvas_.on("contextmenu", evt => { if (evt.ctrlKey) av_utils.forward_event(evt); }); // block context menu on ctrl-click (but allow on the right-click)
        this.surface_.canvas_.on("click", evt => av_utils.forward_event(evt, evt => {
            if (evt.ctrlKey) { // Ctrl-click -> flip
                this.surface_.flip_ew(evt);
                this.draw();
            }
        }));

        this.surface_.canvas_.on("mousedown", evt => av_utils.forward_event(evt, evt => {
            if (evt.altKey) {   // Alt-Drag - pan
                let mousedown_pos = {left: evt.clientX, top: evt.clientY};
                document.onmouseup = () => { document.onmouseup = document.onmousemove = null; };
                document.onmousemove = evt => {
                    this.surface_.move_relative(mousedown_pos.left - evt.clientX, mousedown_pos.top - evt.clientY);
                    mousedown_pos = {left: evt.clientX, top: evt.clientY};
                    this.draw();
                };
            }
        }));
    }

    _show_point_info(mouse_offset, points) {
        if (points.length) {
            const chart = this.viewing_.chart();
            const options = this.widget_.options_;

            const full_name = point_no => point_no < chart.a.length ? av_utils.ace_antigen_full_name(chart.a[point_no], {escape: true}) : av_utils.ace_serum_full_name(chart.s[point_no - chart.a.length], {escape: true});
            const make_point_name_row = point_entry => {
                if (options.point_on_click)
                    return `<li><a href="show-info-on-this-name" point_no="${point_entry.no}" point_name="${point_entry.name}">${point_entry.name}</a></li>`;
                else
                    return `<li>${point_entry.name}</li>`;
            };

            const point_entries = points.map(point_no => { return {name: full_name(point_no), no: point_no}; });
            const mouse_popup_text = $("<ul class='point-info-on-hover'></ul>").append(point_entries.map(make_point_name_row).join(""));
            const popup = av_toolkit.mouse_popup_show(mouse_popup_text, this.surface_.canvas_, {left: mouse_offset.left + options.mouse_popup_offset.left, top: mouse_offset.top + options.mouse_popup_offset.top});
                if (options.point_on_click) {
                    popup.find("a").on("click", evt => {
                        console.log("point infor from hidb");
                        // av_utils.forward_event(evt, evt => show_antigen_serum_info_from_hidb($(evt.target), this.data.c, this.canvas, this.options.point_on_click));
                        // window.setTimeout(av_toolkit.mouse_popup_hide, this.options.point_info_on_hover_delay);
                    });
                }
        }
        else {
            this._hide_point_info();
        }
    }

    _hide_point_info() {
        av_toolkit.mouse_popup_hide();
    }

    _make_coloring_modes(data) {
        const chart = data.c;
        this.coloring_modes_ = [new ColoringOriginal(chart)];
        if (chart.a.some(antigen => antigen.c && antigen.c.length > 0))
            this.coloring_modes_.push(new ColoringByClade(chart));
        this.coloring_modes_.push(new ColoringByAAatPos(chart));
        if (chart.a.some(antigen => antigen.C))
            this.coloring_modes_.push(new ColoringByGeography(chart));
    }

    _make_viewing_modes(data) {
        const chart = data.c;
        this.viewing_modes_ = [new ViewAll(this, chart), new ViewSearch(this, chart)];
        if (chart.t.L && chart.t.L.length > 1)
            this.viewing_modes_.push(new ViewTableSeries(this, chart));
        if (chart.a.reduce((with_dates, antigen) => with_dates + (antigen.D ? 1 : 0), 0) > (chart.a.length * 0.25))
            this.viewing_modes_.push(new ViewTimeSeries(this, chart));
        this.viewing_modes_.push(new ViewGroups(this, chart));
    }
}

// ----------------------------------------------------------------------

class StyleAce
{
    constructor(plot_spec) {
        this.plot_spec_ = plot_spec;
    }

    point(point_no) {
        return this.plot_spec_.P[this.plot_spec_.p[point_no]];
    }
}

class ColoringBase
{
    constructor(chart) {
        this.chart_ = chart;
        this.style = new StyleAce(chart.p);
    }

    drawing_order(drawing_order) {
        return drawing_order;
    }
}

class ColoringOriginal extends ColoringBase
{
    name() {
        return "original";
    }
}

class ColoringByClade extends ColoringBase
{
    name() {
        return "by clade";
    }
}

class ColoringByAAatPos extends ColoringBase
{
    name() {
        return "by AA at pos";
    }
}

class ColoringByGeography extends ColoringBase
{
    name() {
        return "by geography";
    }
}

// ----------------------------------------------------------------------

class ViewingBase
{
    constructor(map_viewer, chart) {
        this.map_viewer_ = map_viewer;
        this.chart_ = chart;
    }

    draw(surface, coloring) {
        for (let point_no of coloring.drawing_order(this.drawing_order())) {
            this.map_viewer_.surface_.point(this.layout_[point_no], coloring.style.point(point_no), point_no, true);
        }

        // const drawing_order_background = this.drawing_order_background();
        // if (drawing_order_background && drawing_order_background.length)
        //     surface.points({drawing_order: this.widget.coloring.drawing_order(drawing_order_background, {background: true}),
        //                             layout: chart.P[this.projection_no()].l,
        //                             transformation: new ace_surface.Transformation(chart.P[this.projection_no()].t),
        //                             styles: this.styles(),
        //                             point_scale: this.point_scale(),
        //                             show_as_background: this.show_as_background()});
        // surface.points({drawing_order: this.widget.coloring.drawing_order(this.drawing_order()),
        //                             layout: chart.P[this.projection_no()].l,
        //                             transformation: new ace_surface.Transformation(chart.P[this.projection_no()].t),
        //                             styles: this.styles(),
        //                             point_scale: this.point_scale()});
    }

    projection(projection_no) {
        if (projection_no < this.chart_.P.length) {
            this.projection_no_ = projection_no;
            this.layout_ = this.chart_.P[this.projection_no_].l;
            this.map_viewer_.surface_.transformation(this.chart_.P[this.projection_no_].t);
            this.map_viewer_.surface_.viewport(this._calculate_viewport());
        }
        else {
            console.log("invalid projection_no", projection_no);
        }
    }

    chart() {
        return this.chart_;
    }

    _calculate_viewport() {
        const transformed_layout = this.map_viewer_.surface_.transformation_.transform_layout(this.layout_);
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

     // {title_fields:}
    title(args) {
        return "$$title";
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
                result = `<li>Tables: ${chart.i.S.length}<ol class='av-scrollable av-tables'><li>${tables}</li></ol></li>`;
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
                result = `<li>Projections: ${chart.P.length}<ol class='av-scrollable av-stresses'><li>${stresses}</li></ol></li>`;
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
            return `<li>Sequenced: ${sequenced} <ul class='av-scrollable av-sequenced'>${clades_li}</ul></li>`;
        };

        let title_box = $("<ul class='av201807-title-mouse-popup'></ul>");
        if (this.chart_.i) {
            title_box.append(box_name(this.chart_));
            if (this.chart_.i.S && this.chart_.i.S.length > 0) {
                title_box.append(box_virus(this.chart_.i.S[0]));
                title_box.append(box_lab(this.chart_.i.S[0]));
            }
            else {
                title_box.append(box_virus(this.chart_.i));
                title_box.append(box_lab(this.chart_.i));
            }
            title_box.append(box_antigens(this.chart_));
            title_box.append(box_date(this.chart_));
            title_box.append(box_sequenced(this.chart_));
            title_box.append(box_tables(this.chart_));
            title_box.append(box_projections(this.chart_));
        }
        return title_box;
    }
}

// ----------------------------------------------------------------------

class ViewAll extends ViewingBase
{
    name() {
        return "all";
    }

    drawing_order(coloring) {
        return this.chart_.p.d || av_utils.array_of_indexes(this.layout_.length);
    }

     // {title_fields:}
    title(args) {
        const makers = this.title_field_makers();
        return av_utils.join_collapse(args.title_fields.map(field => makers[field](this.chart_, this.projection_no_)));
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

}

class ViewSearch extends ViewingBase
{
    name() {
        return "search";
    }
}

class ViewTimeSeries extends ViewingBase
{
    name() {
        return "time series";
    }
}

class ViewTableSeries extends ViewingBase
{
    name() {
        return "table series";
    }
}

class ViewGroups extends ViewingBase
{
    name() {
        return "groups";
    }
}

// ----------------------------------------------------------------------

let sLastId = 0;

function new_id() {
    return "" + (++sLastId);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
