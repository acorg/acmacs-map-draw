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
        this.chart_ = data.c;
        this.viewer_.start();
        this.viewer_.draw();
        this.title_.update();
    }

    update_title(left_arrow, right_arrow) {
        this.title_.arrows(left_arrow, right_arrow);
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
                if (this.viewer_.viewing_) {
                    if (!this.view_dialog_) {
                        this.view_dialog_ = new ViewDialog({widget: this, chart: this.viewer_.viewing_.chart_, on_destroy: () => delete this.view_dialog_});
                        this.viewer_.coloring_.view_dialog_shown(this.view_dialog_);
                        this.viewer_.viewing_.view_dialog_shown(this.view_dialog_);
                    }
                    else
                        this.view_dialog_.position();
                }
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
        if (viewing) {
            this._span().empty().append(viewing.title({title_fields: this.widget_.options_.title_fields}));
            this.resize();
            this.popup_on_hovering_title(viewing.title_box());
        }
    }

    resize() {
        const title_td_border = this.div_.outerWidth(true) - this.div_.width();
        const title_element = this.div_.find("> .av-title-title");
        const title_element_border = title_element.outerWidth(true) - title_element.width();
        const title_left = this.div_.find("> .av-left");
        const title_left_width = title_left.is(":visible") ? title_left.outerWidth(true) : 0;
        const title_right = this.div_.find("> .av-right");
        const title_right_width = title_right.is(":visible") ? title_right.outerWidth(true) : 0;
        title_element.css("width", this.widget_.viewer_.surface_.width() - title_left_width - title_right_width - title_element_border - title_td_border);
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

const ViewDialog_html = "\
<table class='av-view-dialog'>\
  <tr>\
    <td class='av-label'>Projection</td><td class='projection-chooser'></td>\
  </tr>\
  <tr>\
    <td class='av-label'>Coloring</td><td class='coloring'></td>\
  </tr>\
  <tr class='coloring-aa-pos av-initially-hidden'>\
    <td class='av-label'>Positions</td><td class='coloring-aa-pos'><input type='text'></input><a href='coloring-aa-pos-hint'>hint</a></td>\
  </tr>\
  <tr class='coloring-legend av-initially-hidden'>\
    <td class='av-label'>Legend</td><td class='coloring-legend'></td>\
  </tr>\
  <tr>\
    <td class='av-label'>View</td><td class='mode'></td>\
  </tr>\
  <tr class='time-series-period av-initially-hidden'>\
    <td class='av-label'>Period</td><td class='time-series-period'></td>\
  </tr>\
  <tr class='group-series av-initially-hidden'>\
    <td class='av-label'>Group Sets</td>\
    <td class='group-series'>\
      <div class='av-sets'><p class='av-hint'>please drop here group description file or click below to upload</p></div>\
      <div class='av-buttons'>\
        <a href='upload' title='upload and apply group definition'>upload</a>\
        <a href='download' title='download sample group definition for this chart'>download sample</a>\
        <a href='download-chart' title='download chart in the .ace format with the embedded group data'>download chart</a>\
      </div>\
    </td>\
  </tr>\
  <tr class='group-series-combined av-initially-hidden'>\
    <td class='av-label'>Groups</td>\
    <td class='group-series-combined'>\
      <div class='av-buttons'>\
        <a href='exclusive'>exclusive</a>\
        <a href='combined'>combined</a>\
      </div>\
      <table class='av-groups'>\
      </table>\
    </td>\
  </tr>\
  <tr class='shading av-initially-hidden'>\
    <td class='av-label'>Shading</td><td class='shading'></td>\
  </tr>\
  <tr class='search av-initially-hidden'>\
    <td class='av-label'>RegEx</td>\
    <td class='regex'>\
      <input type='text'></input>\
      <!-- <select name='search-mode'><option value='antigens'>antigens only</option><option value='sera'>sera only</option><option value='antigens+sera'>antigens and sera</option></select> -->\
    </td>\
  <tr class='search-results av-initially-hidden'>\
    <td class='av-label'></td>\
    <td class='search-results'>\
      <p class='av-message'>please enter regular expression above and press enter</p>\
      <table></table>\
    </td>\
  </tr>\
</table>\
";

class ViewDialog
{
    constructor(args) {
        this.widget_ = args.widget;
        this.chart_ = args.chart;
        this.canvas_ = this.widget_.viewer_.surface_.canvas_;
        this.selector_use_select_ = false;
        this.window_ = new av_toolkit.MovableWindow({
            title: "View",
            parent: this.canvas_,
            classes: "av201807-view-dialog",
            content_css: {width: "auto", height: "auto"},
            on_destroy: () => this.destroy()
        });
        this.content_ = this.window_.content();
        this.populate();
        this.coloring_legend_ = new ViewDialogColoringLegend(this.section("coloring-legend"));
        this.on_destroy = args.on_destroy;
    }

    destroy() {
        if (this.on_destroy)
            this.on_destroy();
    }

    position() {
        this.window_.position(this.canvas_);
    }

    populate() {
        const table = $(ViewDialog_html).appendTo(this.content_);
        table.find(".av-initially-hidden").hide();
        this._projection_chooser(table.find(".projection-chooser"));
        this._selector_toggle(table.find(".projection-chooser"));
        this._repopulate(table);
    }

    section(name) {
        return this.content_.find("table tr." + name);
    }

    _selector_toggle(section) {
        const checkbox = $(`<input type='checkbox' checked='{this.selector_use_select_}'></input>`).appendTo(section);
        checkbox.on("change", evt => av_utils.forward_event(evt, evt => {
            this.selector_use_select_ = evt.currentTarget.checked;
            this._repopulate(this.content_.find("table"));
        }));
    }

    _repopulate(table) {
        this._coloring_chooser(table.find("td.coloring"));
        this._view_mode_chooser(table.find("td.mode"));
    }

    _projection_chooser(section) {

        const projection_title = (projection, index) => {
            return av_utils.join_collapse([
                index === undefined ? null : "" + (index + 1) + ".",
                projection.s.toFixed(4),
                "&ge;" + (projection.m || "none"),
                projection.C ? "forced-col-bases" : null,
                projection.c
            ]);
        }

        if (this.chart_.P.length === 0)
            section.append("<div class='av-error'>None</div>");
        else if (this.chart_.P.length === 1)
            section.append(`<div>${projection_title(this.chart_.P[0])}</div>`);
        else {
            const entries = this.chart_.P.map((prj, index) => `<option value="${index}">${projection_title(prj, index)}</option>`).join();
            const select = $(`<select>${entries}</select>`).appendTo(section);
            select.on("change", evt => av_utils.forward_event(evt, evt => this.widget_.viewer_.projection(parseInt(evt.currentTarget.value), true)));
        }
    }

    _coloring_chooser(section) {
        section.empty();
        const onchange = value => this.widget_.viewer_.coloring(value, true);
        const selector = this.selector_use_select_ ? new SelectorSelect(section, onchange) : new SelectorButtons(section, onchange);
        for (let coloring_mode of this.widget_.viewer_.coloring_modes_)
            selector.add(coloring_mode.name());
        selector.current(this.widget_.viewer_.coloring_.name());
    }

    _view_mode_chooser(section) {
        section.empty();
        const onchange = value => this.widget_.viewer_.viewing(value, true);
        const selector = this.selector_use_select_ ? new SelectorSelect(section, onchange) : new SelectorButtons(section, onchange);
        for (let viewing_mode of this.widget_.viewer_.viewing_modes_)
            selector.add(viewing_mode.name());
        selector.current(this.widget_.viewer_.viewing_.name());
    }

} // class ViewDialog

// ----------------------------------------------------------------------

class ViewDialogColoringLegend
{
    constructor(section) {
        this.section_ = section;
    }

    show() {
        this.section_.show();
    }

    hide() {
        this.section_.hide();
    }

    populate(legend) {
        const td_legend = this.section_.find("td.coloring-legend").empty();
        td_legend.append("<table class='av-legend'><tr class='av-names'></tr><tr class='av-colors'></tr></table>");
        for (let entry of legend) {
            td_legend.find("tr.av-names").append(`<td>${entry.name}</td>`);
            if (entry.color !== undefined && entry.color !== null)
                td_legend.find("tr.av-colors").append(`<td><span class="av-color" style="background-color: ${entry.color}">__</span>${entry.count || ""}</td>`);
        }
    }

}

// ----------------------------------------------------------------------

class SelectorSelect
{
    constructor(section, onchange) {
        this.select_ = $("<select></select>").appendTo(section);
        this.select_.on("change", evt => av_utils.forward_event(evt, evt => onchange(evt.currentTarget.value)));
    }

    add(name) {
        this.select_.append(`<option value="${name}">${name}</option>`);
    }

    current(name) {
        this.select_.val(name);
    }
}

class SelectorButtons
{
    constructor(section, onchange) {
        this.section_ = section;
        this.onchange_ = onchange;
    }

    add(name) {
        $(`<a href="${name}">${name}</a>`).appendTo(this.section_).on("click", evt => av_utils.forward_event(evt, () => {
            if (!evt.currentTarget.matches(".av-current")) {
                this.current(name);
                this.onchange_(name);
            }
        }));
    }

    current(name) {
        this.section_.find("a").removeClass("av-current");
        this.section_.find(`a[href="${name}"]`).addClass("av-current");
    }
}

// ----------------------------------------------------------------------

class MapViewer
{
    constructor(widget, canvas) {
        this.widget_ = widget;
        this.surface_ = new av_surface.Surface(canvas);
    }

    start() {
        this._make_coloring_modes();
        this._make_viewing_modes();
        this.coloring("original");
        this.viewing("all");
        this.projection(0);
        this._bind();
        window.setTimeout(() => this._size_parent(), 10);
    }

    draw() {
        this._hide_point_info();
        this.surface_.reset();
        this.surface_.grid();
        this.surface_.border();
        this.viewing_.draw(this.coloring_);
    }

    resize(width_diff) {
        this.surface_.resize(width_diff);
        this.widget_.title_.resize();
        if (this.viewing_)
            this.draw();
        this._size_parent();
    }

    _size_parent() {
        this.surface_.canvas_.parent().css({width: this.surface_.canvas_.outerWidth(), height: this.surface_.canvas_.outerHeight()});
    }

    coloring(mode_name, redraw=false) {
        if (this.coloring_)
            this.coloring_.on_exit(this.widget_.view_dialog_);
        this.coloring_ = this.coloring_modes_.find(mode => mode.name() === mode_name) || this.coloring_modes_.find(mode => mode.name() === "original");
        this.coloring_.on_entry(this.widget_.view_dialog_);
        if (redraw)
            this.draw();
    }

    viewing(mode_name, redraw=false) {
        if (this.viewing_)
            this.viewing_.on_exit(this.widget_.view_dialog_);
        this.viewing_ = this.viewing_modes_.find(mode => mode.name() === mode_name) || this.viewing_modes_.find(mode => mode.name() === "all");
        if (this.projection_no_ !== undefined)
            this.viewing_.projection(this.projection_no_);
        this.viewing_.on_entry(this.widget_.view_dialog_);
        if (redraw)
            this.draw();
    }

    projection(projection_no, redraw=false) {
        this.projection_no_ = projection_no;
        this.viewing_.projection(this.projection_no_);
        if (redraw)
            this.draw();
    }

    // {name: ""}
    make_coloring_modified(args) {
        if (!this.coloring_modes_.find(mode => mode.name() === args.name)) {
            this.coloring_modes_.push(new ColoringModified(this.widget_, args.name, this.coloring_.styles()));
        }
        this.coloring(args.name);
    }

    _drawing_order() {
        return this.coloring_.drawing_order(this.viewing_.drawing_order_foreground());
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
                        console.log("point info from hidb");
// jopa
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

    _make_coloring_modes() {
        const chart = this.widget_.chart_;
        this.coloring_modes_ = [new ColoringOriginal(this.widget_)];
        if (chart.a.some(antigen => antigen.c && antigen.c.length > 0))
            this.coloring_modes_.push(new ColoringByClade(this.widget_));
        this.coloring_modes_.push(new ColoringByAAatPos(this.widget_));
        if (chart.a.some(antigen => antigen.C))
            this.coloring_modes_.push(new ColoringByGeography(this.widget_));
    }

    _make_viewing_modes() {
        const chart = this.widget_.chart_;
        this.viewing_modes_ = [new ViewAll(this, chart), new ViewSearch(this, chart)];
        if (chart.t.L && chart.t.L.length > 1)
            this.viewing_modes_.push(new ViewTableSeries(this, chart));
        if (chart.a.reduce((with_dates, antigen) => with_dates + (antigen.D ? 1 : 0), 0) > (chart.a.length * 0.25))
            this.viewing_modes_.push(new ViewTimeSeries(this, chart));
        this.viewing_modes_.push(new ViewGroups(this, chart));
    }
}

// ----------------------------------------------------------------------

    // {reset_sera: false}
function all_styles(source, args={})
{
    if (source.a && source.s && source.p) {
        const egg_passage = (style, index) => {
            if (index < source.a.length && (!style.a || style.a === 1) && source.a[index].S && source.a[index].S.indexOf("E") >= 0)
                style.a = 0.75;
            if (index < source.a.length && (!style.r || style.r === 0) && source.a[index].R && source.a[index].R.length)
                style.r = Math.PI / 6;
            return style;
        };
        let all_styles = source.p.p.map(style_no => Object.assign({}, source.p.P[style_no])).map(egg_passage);
        if (args.reset_sera) {
            source.s.forEach((serum, serum_no) => {
                delete all_styles[serum_no + source.a.length].F;
                all_styles[serum_no + source.a.length].O = av_toolkit.sGREY;
            });
        }
        return all_styles;
    }
    else if (source.p && source.P) {
        return source.p.map(style_no => Object.assign({}, source.P[style_no]));
    }
    else
        return source;
}

// ----------------------------------------------------------------------

class ColoringBase
{
    constructor(widget) {
        this.widget_ = widget;
        this.chart_ = widget.chart_;
    }

    drawing_order(drawing_order) {
        return drawing_order || av_utils.array_of_indexes(this.chart_.a.length + this.chart_.s.length);
    }

    update_for_drawing_level(drawing_level) {
    }

    legend() {
        return null;
    }

    on_entry(view_dialog) {
    }

    on_exit(view_dialog) {
    }

    view_dialog_shown(view_dialog) {
    }

    point_style(point_no) {
        return this.styles_[point_no];
    }

    // {reset_sera: false}
    all_styles(args={}) {
        return all_styles(this.chart_, args);
    }
}

// ----------------------------------------------------------------------

class ColoringOriginal extends ColoringBase
{
    name() {
        return "original";
    }

    point_style(point_no) {
        const plot_spec = this.chart_.p;
        return plot_spec.P[plot_spec.p[point_no]];
    }

    styles() {
        return this.chart_.p;
    }
}

// ----------------------------------------------------------------------

class ColoringModified extends ColoringBase
{
    constructor(widget, name, source) {
        super(widget);
        this.name_ = name;
        this.styles_ = all_styles(source);
    }

    name() {
        return this.name_;
    }

    point_style(point_no) {
        return this.styles_[point_no];
    }

    styles() {
        return this.styles_;
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
    "": av_toolkit.sGREY,
    undefined: av_toolkit.sGREY,
    null: av_toolkit.sGREY
};

class ColoringByClade extends ColoringBase
{
    name() {
        return "by clade";
    }

    on_entry(view_dialog) {
        super.on_entry(view_dialog);
        if (!this.styles_) {
            this._make_antigens_by_clade({set_clade_for_antigen: true});
            this._make_styles();
        }
        if (view_dialog) {
            view_dialog.coloring_legend_.show();
        }
    }

    on_exit(view_dialog) {
        super.on_exit(view_dialog);
        if (view_dialog)
            view_dialog.coloring_legend_.hide();
    }

    view_dialog_shown(view_dialog) {
        super.view_dialog_shown(view_dialog);
        view_dialog.coloring_legend_.populate(this.legend());
        view_dialog.coloring_legend_.show();
    }

    drawing_order(drawing_order) {
        // order: sera, not sequenced, sequenced without clade, clade with max number of antigens, ..., clade with fewer antigens
        return super.drawing_order(drawing_order).slice(0).sort((p1, p2) => this.point_rank_[p1] - this.point_rank_[p2]);
    }

    update_for_drawing_level(drawing_level) {
        if (drawing_level.type === "foreground") {
            this._make_antigens_by_clade({drawing_order: drawing_level.drawing_order});
            this.widget_.view_dialog_ && this.widget_.view_dialog_.coloring_legend_.populate(this.legend());
        }
    }

    legend() {
        return this.clade_order_.filter(clade => !!clade).map(clade => { return {name: clade, count: this.clade_to_number_of_antigens_[clade], color: sCladeColors[clade]}; });
    }

    styles() {
        return this._styles_;
    }

    // {set_clade_for_antigen: false, drawing_order: []}
    _make_antigens_by_clade(args={}) {
        const clade_sorting_key = clade => (clade === "GLY" || clade === "NO-GLY" || clade === "SEQUENCED") ? 0 : clade.length;
        this.clade_to_number_of_antigens_ = {};
        if (args.set_clade_for_antigen)
            this.clade_for_antigen_ = Array.apply(null, {length: this.chart_.a.length}).map(() => "");
        const drawing_order = args.drawing_order || super.drawing_order();
        drawing_order.filter(no => no < this.chart_.a.length).forEach(antigen_no => {
            const clades = (this.chart_.a[antigen_no].c || []).sort((a, b) => clade_sorting_key(b) - clade_sorting_key(a));
            let clade = clades.length > 0 ? clades[0] : "";
            if (clade === "GLY" || clade === "NO-GLY")
                clade = "SEQUENCED";
            this.clade_to_number_of_antigens_[clade] = (this.clade_to_number_of_antigens_[clade] || 0) + 1;
            if (args.set_clade_for_antigen)
                this.clade_for_antigen_[antigen_no] = clade;
        });
        this.clade_order_ = Object.keys(this.clade_to_number_of_antigens_).sort((a, b) => this._clade_rank(a) - this._clade_rank(b));
        if (args.set_clade_for_antigen)
            this.point_rank_ = this.clade_for_antigen_.map(clade => this.clade_order_.indexOf(clade)).concat(Array.apply(null, {length: this.chart_.s.length}).map(() => -2));
    }

    _clade_rank(clade) {
        // order: not sequenced, sequenced without clade, clade with max number of antigens, ..., clade with fewer antigens
        if (clade === "")
            return -1e7;
        if (clade === "GLY" || clade === "NO-GLY" || clade === "SEQUENCED")
            return -1e6;
        return - this.clade_to_number_of_antigens_[clade];
    }

    _make_styles() {
        this.styles_ = this.all_styles({reset_sera: true});
        this.clade_for_antigen_.forEach((clade, antigen_no) => {
            this.styles_[antigen_no].F = sCladeColors[clade];
            // this.styles_[antigen_no].O = "white";
        });
    }
}

// ----------------------------------------------------------------------

class ColoringByAAatPos extends ColoringBase
{
    name() {
        return "by AA at pos";
    }

    on_entry(view_dialog) {
        super.on_entry(view_dialog);
        if (!this.styles_)
            this._make_styles({set_point_rank: true});
        this.sequences().then(data => this._sequences_received(data)).catch(error => console.log("Coloring_AAPos::constructor sequences error", error));
        if (view_dialog)
            view_dialog.coloring_legend_.show();
    }

    on_exit(view_dialog) {
        super.on_exit(view_dialog);
        if (view_dialog) {
            view_dialog.coloring_legend_.hide();
            this.section_aa_at_pos_.hide();
        }
    }

    view_dialog_shown(view_dialog) {
        super.view_dialog_shown(view_dialog);
        view_dialog.coloring_legend_.populate(this.legend());
        view_dialog.coloring_legend_.show();
        if (this.sequences_) {
            this.section_aa_at_pos_ = view_dialog.section("coloring-aa-pos").show();
            this._section_aa_at_pos_populate();
        }
    }

    drawing_order(original_drawing_order) {
        // order: sera, not sequenced, "clade" with max number of antigens, ..., "clade" with fewer antigens
        original_drawing_order = super.drawing_order(original_drawing_order);
        if (!this.point_rank_)
            return original_drawing_order;
        const drawing_order = original_drawing_order.slice(0).sort((p1, p2) => this.point_rank_[p1] - this.point_rank_[p2]);
        this._make_legend(drawing_order);
        return drawing_order;
    }

    update_for_drawing_level(drawing_level) {
        if (drawing_level.type === "foreground") {
            this._make_legend(drawing_level.drawing_order);
            this.widget_.view_dialog_ && this.widget_.view_dialog_.coloring_legend_.populate(this.legend());
        }
    }

    legend() {
        if (this.sequences_) {
            if (this.legend_)
                return this.legend_;
            else
                return [{name: "type space separated positions and press Enter"}];
        }
        else
            return [{name: "loading sequences, please wait"}];
    }

    styles() {
        return this._styles_;
    }

    set_positions(positions) {
        const update = positions !== this.positions_;
        if (update) {
            this.positions_ = positions;
            this._make_styles({set_point_rank: true});
        }
        return update;
    }

    async sequences() {
        if (this.sequences_ === undefined) {
            this.sequences_ = "loading, please wait";
            return this.widget_.options_.api.get_sequences().then(
                data => new Promise(resolve => resolve(data.sequences)),
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

    _make_legend(drawing_order) {
        if (this.antigen_aa_) {
            const aa_count = this.antigen_aa_.reduce((count, entry) => {
                if (drawing_order.includes(entry.no))
                    count[entry.aa] = (count[entry.aa] || 0) + 1;
                return count;
            }, {});
            this.legend_ = this.aa_order_.map((aa, index) => aa_count[aa] ? {name: aa, count: aa_count[aa], color: av_toolkit.ana_colors(index)} : null).filter(elt => !!elt);
        }
        else
            this.legend_ = null;
    }

    _make_styles(args={}) {
        this._reset_styles();
        this.legend_ = null;
        if (this.sequences_ && this.positions_ && this.positions_.length) {
            this.antigen_aa_  = Object.entries(this.sequences_.antigens).map(entry => { return {no: parseInt(entry[0]), aa: this.positions_.map(pos => entry[1][pos - 1]).join("")}; });
            const aa_count = this.antigen_aa_.reduce((count, entry) => { count[entry.aa] = (count[entry.aa] || 0) + 1; return count; }, {});
            this.aa_order_ = Object.keys(aa_count).sort((e1, e2) => aa_count[e2] - aa_count[e1]);
            if (args.set_point_rank)
                this.point_rank_ = Array.apply(null, {length: this.chart_.a.length}).map(() => -1).concat(Array.apply(null, {length: this.chart_.s.length}).map(() => -2));
            this.antigen_aa_.forEach(entry => {
                const aa_index = this.aa_order_.indexOf(entry.aa);
                this.styles_[entry.no].F = av_toolkit.ana_colors(aa_index);
                this.styles_[entry.no].O = "black";
                if (args.set_point_rank)
                    this.point_rank_[entry.no] = aa_index;
            });
        }
    }

    _reset_styles() {
        this.styles_ = this.all_styles({reset_sera: true});
        this.chart_.a.forEach((antigen, antigen_no) => {
            this.styles_[antigen_no].F = av_toolkit.sLIGHTGREY;
            this.styles_[antigen_no].O = av_toolkit.sGREY;
        });
    }

    _sequences_received(data) {
        this.sequences_ = data;
        if (this.widget_.view_dialog_) {
            this.section_aa_at_pos_ = this.widget_.view_dialog_.section("coloring-aa-pos").show();
            this._section_aa_at_pos_populate();
        }
    }

    _section_aa_at_pos_populate() {
        const input = this.section_aa_at_pos_.find("input")
              .off("keypress")
              .on("keypress", evt => {
                  if (evt.charCode === 13) {
                      if (this.set_positions(evt.currentTarget.value.split(/[^0-9]/).filter(entry => !!entry)))
                          this.widget_.viewer_.draw();
                  }
              })
              .val(this.positions_ ? this.positions_.join(" ") : "");
        window.setTimeout(() => input.focus(), 10);
        this.section_aa_at_pos_.find("a")
           .off("click")
           .on("click", evt => av_utils.forward_event(evt, evt => this._aa_positions_hint($(evt.currentTarget))));
    }

   _aa_positions_hint(parent) {
       const movable_window = new av_toolkit.MovableWindow({
           title: "AA positions", parent: parent,
           classes: "av201807-coloring-aa-pos-hint",
           content_css: {width: "auto", height: "auto"}
       });
       const content = movable_window.content().empty();
       const compare_shannon = (e1, e2) => e2[1].shannon - e1[1].shannon;
       const compare_position = (e1, e2) => e1[0] - e2[0];
       let sort_by = "shannon";
       const make_table = tbl => {
           tbl.empty();
           Object.entries(this.sequences_.per_pos).sort(sort_by === "shannon" ? compare_shannon : compare_position).forEach(entry => {
               let [pos, sh_count] = entry;
               if (Object.keys(sh_count.aa_count).length > 1) {
                   const row = $(`<tr><td class='av-pos'>${pos}</td></tr>`).appendTo(tbl);
                   const aa_order = Object.keys(sh_count.aa_count).sort((aa1, aa2) => sh_count.aa_count[aa2] - sh_count.aa_count[aa1]);
                   aa_order.forEach(aa => {
                       row.append(`<td class='av-aa'>${aa}</td><td class='av-count'>${sh_count.aa_count[aa]}</td>`);
                   });
               }
           });
       };
       const fill = () => {
           const sort_by_button = $("<a href='sort-by'></a>").appendTo(content);
           const sort_by_text = () => { sort_by_button.empty().append(sort_by === "shannon" ? "re-sort by position" : "re-sort by shannon index"); };
           const tbl = $("<table class='av-position-hint''></table>").appendTo(content);
           sort_by_button.on("click", evt => av_utils.forward_event(evt, evt => {
               sort_by = sort_by === "shannon" ? "position" : "shannon";
               make_table(tbl);
               sort_by_text();
           }));
           sort_by_text();
           make_table(tbl);
           window.setTimeout(() => {
               if (content.height() > 300)
                   content.css("height", "300px");
           }, 10);
       };
       const wait_fill = () => {
           if (this.sequences_ && typeof(this.sequences_) !== "string")
               fill();
           else
               window.setTimeout(wait_fill, 100);
       };
       wait_fill();
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

class ColoringByGeography extends ColoringBase
{
    name() {
        return "by geography";
    }

    on_entry(view_dialog) {
        super.on_entry(view_dialog);
        if (!this.styles_) {
            this.styles_ = this.all_styles({reset_sera: true});
            this._make_continent_count({set_styles: true});
        }
        if (view_dialog) {
            view_dialog.coloring_legend_.show();
        }
    }

    on_exit(view_dialog) {
        super.on_exit(view_dialog);
        if (view_dialog)
            view_dialog.coloring_legend_.hide();
    }

    view_dialog_shown(view_dialog) {
        super.view_dialog_shown(view_dialog);
        view_dialog.coloring_legend_.populate(this.legend());
        view_dialog.coloring_legend_.show();
    }

    drawing_order(original_drawing_order) {
        // order: sera, most popular continent, ..., lest popular continent
        const continent_order = this.continent_count_.map(entry => entry.name);
        const ranks = Array.apply(null, {length: this.chart_.a.length}).map((_, ag_no) => continent_order.indexOf(this.chart_.a[ag_no].C) + 10).concat(Array.apply(null, {length: this.chart_.s.length}).map(_ => 0));
        const drawing_order = super.drawing_order(original_drawing_order).slice(0).sort((p1, p2) => ranks[p1] - ranks[p2]);
        this._make_continent_count({drawing_order: drawing_order});
        return drawing_order;
    }

    update_for_drawing_level(drawing_level) {
        if (drawing_level.type === "foreground")
            this.widget_.view_dialog_ && this.widget_.view_dialog_.coloring_legend_.populate(this.legend());
    }

    legend() {
        return this.continent_count_.map(entry => Object.assign({}, entry, {name: continent_name_for_legend[entry.name] || entry.name}));
    }

    styles() {
        return this._styles_;
    }

    _make_continent_count(args={}) {
        let continent_count = {};
        const drawing_order = args.drawing_order || super.drawing_order();
        drawing_order.filter(no => no < this.chart_.a.length).forEach(antigen_no => {
            const continent = this.chart_.a[antigen_no].C;
            if (args.set_styles)
                this.styles_[antigen_no].F = continent_colors[continent];
            continent_count[continent] = (continent_count[continent] || 0) + 1;
        });
        this.continent_count_ = Object.keys(continent_count)
            .map(continent => { return {name: continent, count: continent_count[continent], color: continent_colors[continent]}; })
            .sort((e1, e2) => e2.count - e1.count);
    }
}

// ----------------------------------------------------------------------

class ViewingBase
{
    constructor(map_viewer, chart) {
        this.map_viewer_ = map_viewer;
        this.chart_ = chart;
    }

    draw(coloring) {
        for (let drawing_level of this.drawing_levels()) {
            for (let point_no of coloring.drawing_order(drawing_level.drawing_order)) {
                this.map_viewer_.surface_.point(this.layout_[point_no], drawing_level.style_modifier(coloring.point_style(point_no)), point_no, true);
            }
            coloring.update_for_drawing_level(drawing_level);
        }
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

    chart_drawing_order() {
        return this.chart_.p.d || av_utils.array_of_indexes(this.layout_.length);
    }

    on_exit(view_dialog) {
    }

    on_entry(view_dialog) {
    }

    view_dialog_shown(view_dialog) {
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

// https://stackoverflow.com/questions/5560248/programmatically-lighten-or-darken-a-hex-color-or-rgb-and-blend-colors
function shadeColor2(color, percent) {
    const f = parseInt(color.slice(1), 16),
          t = percent < 0 ? 0 : 255,
          p = percent < 0 ? percent * -1 : percent,
          R = f >> 16,
          G = f >> 8 & 0x00FF,
          B = f & 0x0000FF;
    return "#"+(0x1000000+(Math.round((t-R)*p)+R)*0x10000+(Math.round((t-G)*p)+G)*0x100+(Math.round((t-B)*p)+B)).toString(16).slice(1);
}

// the same as shadeColor2 but optimized for positive percent
function paleColor2(color, percent) {
    const f = parseInt(color.slice(1), 16),
          R = f >> 16,
          G = f >> 8 & 0x00FF,
          B = f & 0x0000FF;
    return "#"+(0x1000000+(Math.round((255-R)*percent)+R)*0x10000+(Math.round((255-G)*percent)+G)*0x100+(Math.round((255-B)*percent)+B)).toString(16).slice(1);
}

function style_modifier_shade(style) {
    const shade = 0.8;
    return Object.assign({}, style, {
        F: (style.F && paleColor2(style.F, shade)) || "transparent",
        O: (style.O && paleColor2(style.O, shade)) || paleColor2("#000000", shade)
    });
}

function style_modifier_grey(style) {
    const grey = av_toolkit.sLIGHTGREY;
    return Object.assign({}, style, {F: (style.F && grey) || "transparent", O: grey});
}

function style_modifier_legacy(style) {
    console.error("style_modifier_legacy");
    return style;
}

const sStyleModifiers = {shade: style_modifier_shade, grey: style_modifier_grey, legacy: style_modifier_legacy};

// ----------------------------------------------------------------------

class ViewAll extends ViewingBase
{
    name() {
        return "all";
    }

    drawing_levels() {
        return [
            {drawing_order: this.chart_drawing_order(), style_modifier: style => style, type: "foreground"}
        ];
    }

    drawing_order_foreground() {
        return this.chart_drawing_order();
    }

    on_entry(view_dialog) {
        this.map_viewer_.widget_.update_title();
    }

     // {title_fields:}
    title(args) {
        const makers = this.title_field_makers();
        return this.projection_no_ !== undefined ? av_utils.join_collapse(args.title_fields.map(field => makers[field](this.chart_, this.projection_no_))) : "";
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

// ----------------------------------------------------------------------

class ViewSearch extends ViewingBase
{
    constructor(map_viewer, chart) {
        super(map_viewer, chart);
        this.shading_ = new Shading(this);
        this._reset();
    }

    name() {
        return "search";
    }

    on_exit(view_dialog) {
        super.on_exit(view_dialog);
        this.shading_.hide();
        this.search_section_.hide();
        this.search_results_section_.hide();
    }

    on_entry(view_dialog) {
        super.on_entry(view_dialog);
        this.search_section_ = view_dialog.section("search").show();
        this.search_results_section_ = view_dialog.section("search-results").show();
        this._bind();
        this.shading_.show(view_dialog && view_dialog.section("shading"));
        this.map_viewer_.widget_.update_title();
    }

    view_dialog_shown(view_dialog) {
        super.view_dialog_shown(view_dialog);
        this.search_section_ = view_dialog.section("search").show();
        this.search_results_section_ = view_dialog.section("search-results").show();
        this._bind();
        this.shading_.show(view_dialog.section("shading"));
        this.map_viewer_.widget_.update_title();
    }

    _shading_changed(shading) {
    }

    title(args) {
        if (this.selected_antigens_.length || this.selected_sera_.length) {
            return `${this.selected_antigens_.length} antigens and ${this.selected_sera_.length} sera selected`;
        }
        else
            return "no matches";
    }

    drawing_levels() {
        return [
            {drawing_order: this.drawing_order_background_, style_modifier: sStyleModifiers[this.shading_.shading_]},
            {drawing_order: this.drawing_order_foreground_, style_modifier: style => style, type: "foreground"}
        ];
    }

    drawing_order_foreground() {
        return this.drawing_order_foreground_;
    }

    _make_drawing_order() {
        this.drawing_order_foreground_ = this.selected_sera_.concat(this.selected_antigens_);
        this.drawing_order_background_ = this.chart_drawing_order().filter(point_no => !this.drawing_order_foreground_.includes(point_no));
    }

    _bind() {
        const input = this.search_section_.find("td.regex input").val("");
        const search_results = this.search_results_section_.find("td.search-results");
        const filter = () => {
            if (input.val() === "") {
                search_results.find(".av-message").show();
                this._reset(search_results.find("table"));
                this.map_viewer_.draw();
            }
            else {
                search_results.find(".av-message").hide();
                this._filter(input.val());
            }
        };
        input.off("keypress").on("keypress", evt => { if (evt.charCode === 13) filter(); }).focus();
        filter();
    }

    _reset() {
        this.search_results_section_ && this.search_results_section_.find("td.search-results table").empty();
        this.selected_antigens_ = [];
        this.selected_sera_ = [];
        this._make_drawing_order();
    }

    _filter(text) {
        const table = this.search_results_section_.find("td.search-results table").empty();
        this._match_antigens(text);
        this._match_sera(text);
        if (this.selected_antigens_.length || this.selected_sera_.length) {
            if (this.selected_antigens_.length)
                this._add_many(table, this.selected_antigens_, "" + this.selected_antigens_.length + " antigens");
            if (this.selected_sera_.length)
                this._add_many(table, this.selected_sera_, "" + this.selected_sera_.length + " sera");
            if ((this.selected_antigens_.length + this.selected_sera_.length) < 100) {
                if (this.selected_antigens_.length) {
                    this._add_separator(table);
                    this.selected_antigens_.forEach(no => this._add(table, no, this.antigens_match_, 0));
                }
                if (this.selected_sera_.length) {
                    this._add_separator(table);
                    this.selected_sera_.forEach(no => this._add(table, no, this.sera_match_, this.chart_.a.length));
                }
            }
        }
        else {
            table.append("<tr><td class='av-message'>nothing matched</td></tr>");
        }
        this._make_drawing_order();
        this.map_viewer_.widget_.update_title();
        this.map_viewer_.draw();
    }

    _match_antigens(regex) {
        if (!this.antigens_match_)
            this.antigens_match_ = this.chart_.a.map(ag => av_utils.join_collapse([ag.N, ag.R, av_utils.join_collapse(ag.a), ag.P, av_utils.join_collapse(ag.l)]));
        this._match(regex, this.antigens_match_, this.selected_antigens_, 0);
    }

    _match_sera(regex) {
        if (!this.sera_match_)
            this.sera_match_ = this.chart_.s.map(sr => av_utils.join_collapse([sr.N, sr.R, av_utils.join_collapse(sr.a), sr.I, sr.s]));
        this._match(regex, this.sera_match_, this.selected_sera_, this.chart_.a.length);
    }

    _match(regex, match_data, result, base) {
        const re = new RegExp(regex, "i");
        result.splice(0);
        match_data.forEach((ag, no) => {
            if (ag.search(re) >= 0)
                result.push(no + base);
        });
    }

    _add_many(table, indexes, label) {
        const attrs = indexes.reduce((attrs, index) => {
            const style = this._style(index);
            attrs.S[style.S || "C"] = true;
            attrs.F[style.F || "transparent"] = true;
            attrs.O[style.O || "black"] = true;
            attrs.o[style.o || 1] = true;
            attrs.a[style.a || 1] = true;
            attrs.r[style.r || 0] = true;
            attrs.s[style.s || 0] = true;
            return attrs;
        }, {S: {}, F: {}, O: {}, o: {}, a: {}, r: {}, s: {}});
        const canvas = $("<canvas></canvas>");
        const canvas_access = new av_point_style.PointStyleModifierCanvasAccess(canvas);
        Object.entries(attrs).forEach(entry => {
            const keys = Object.keys(entry[1]);
            if (keys.length === 1)
                canvas_access.set(entry[0], keys[0]);
        });
        const tr = $(`<tr class='av-many'><td class='av-plot-spec'></td><td class='av-label'>${label}</td></tr>`).appendTo(table);
        tr.find("td.av-plot-spec").append(canvas);
        av_point_style.point_style_modifier({canvas: canvas, onchange: data => this._style_modified(data, indexes)});
    }

    _add(table, index, collection, base) {
        const style = this._style(index);
        const canvas = $("<canvas></canvas>");
        const canvas_access = new av_point_style.PointStyleModifierCanvasAccess(canvas);
        canvas_access.set("S", style.S || "C");
        canvas_access.set("s", style.s || 1);
        canvas_access.set("F", style.F || "transparent");
        canvas_access.set("O", style.O || "black");
        canvas_access.set("o", style.o || 1);
        canvas_access.set("a", style.a || 1);
        canvas_access.set("r", style.r || 0);
        const tr = $(`<tr class='av-many'><td class='av-plot-spec'></td><td class='av-label'>${collection[index - base]}</td></tr>`).appendTo(table);
        tr.find("td.av-plot-spec").append(canvas);
        av_point_style.point_style_modifier({canvas: canvas, onchange: data => this._style_modified(data, [index])});
    }

    _add_separator(table) {
        table.append("<tr class='av-separator'><td colspan='2'></td></tr>");
    }

    _style(index) {
        const get = styles => (styles.P && styles.p) ?  styles.P[styles.p[index]] : styles[index];
        return get(this.map_viewer_.coloring_.styles());
    }

    _style_modified(data, indexes) {
        if (! (this.map_viewer_.coloring_ instanceof ColoringModified))
            this.map_viewer_.make_coloring_modified({name: new Date().toLocaleString("en-CA", {hour12: false}).replace(",", "").substr(0, 16)});
        const styles = this.map_viewer_.coloring_.styles();
        indexes.forEach(index => styles[index][data.name] = data.value);
        this.map_viewer_.draw();
    }

    // _style_modified(data, indexes) {
    //     if (this.style_set_name_ === undefined)
    //         this._create_style_set();
    //     console.log("_style_modified", data, indexes);
    //     indexes.forEach(index => this._style(index)[data.name] = data.value);
    //     this.map_viewer_.draw();
    // }

    // _create_style_set() {
    //     if (!this.style_set_)
    //         this.style_set_ = {};
    //     this.style_set_name_ = new Date().toLocaleString("en-CA", {hour12: false}).replace(",", "").substr(0, 16);
    //     const styles = this.map_viewer_.coloring_.styles();
    //     if (styles.P && styles.p)
    //         this.style_set_[this.style_set_name_] = this.map_viewer_.coloring_.all_styles();
    //     else
    //         this.style_set_[this.style_set_name_] = styles;
    // }
}

// ----------------------------------------------------------------------

class ViewingSeries extends ViewingBase
{
    constructor(map_viewer, chart) {
        super(map_viewer, chart);
        this.shading_ = new Shading(this);
    }

    title() {
        return this.pages_[this.page_no_];
    }

    on_exit(view_dialog) {
        super.on_exit(view_dialog);
        this.shading_.hide();
    }

    on_entry(view_dialog) {
        super.on_entry(view_dialog);
        if (!this.pages_)
            this._make_pages();
        if (this.page_no_ === undefined)
            this.set_page(this._initial_page_no());
        else
            this._update_title();
        this.shading_.show(view_dialog && view_dialog.section("shading"));
    }

    view_dialog_shown(view_dialog) {
        super.view_dialog_shown(view_dialog);
        this.shading_.show(view_dialog.section("shading"));
    }

    set_page(page_no, redraw) {
        if (page_no >= 0 && page_no < this.pages_.length) {
            this.page_no_ = page_no;
            this._make_drawing_levels(this.shading_.shading_);
            this._update_title();
            if (redraw)
                this.map_viewer_.draw();
        }
    }

    drawing_levels() {
        return this.drawing_levels_;
    }

    drawing_order_foreground() {
        return this.drawing_levels_[this.drawing_levels_.length - 1].drawing_order;
    }

    current_page() {
        return this.page_no_;
    }

    _update_title() {
        this.map_viewer_.widget_.update_title(this.page_no_ > 0 ? () => this.set_page(this.page_no_ - 1, true) : null,
                                              this.page_no_ < (this.pages_.length - 1) ? () => this.set_page(this.page_no_ + 1, true) : null);
    }
}

// ----------------------------------------------------------------------

class ViewTimeSeries extends ViewingSeries
{
    constructor(map_viewer, chart) {
        super(map_viewer, chart);
        this.period_ = "month";
    }

    name() {
        return "time series";
    }

    period(new_period) {
        if (new_period !== undefined) {
            this.period_ = new_period;
            this._make_pages();
            this.set_page(this._initial_page_no(), true);
        }
        return this.period_;
    }

    on_exit(view_dialog) {
        super.on_exit(view_dialog);
        view_dialog && view_dialog.section("time-series-period").hide();
    }

    on_entry(view_dialog) {
        super.on_entry(view_dialog);
        view_dialog && this._period_chooser_populate(view_dialog.section("time-series-period"));
    }

    view_dialog_shown(view_dialog) {
        super.view_dialog_shown(view_dialog);
        this._period_chooser_populate(view_dialog.section("time-series-period"));
    }

    _initial_page_no() {
        return this.pages_.length - 1;
    }

    _shading_changed(shading) {
        this._make_drawing_levels(shading);
    }

    _make_pages() {
        let periods = new Set();
        for (let antigen of this.chart().a) {
            const period_name = this._antigen_period_name(antigen);
            if (period_name)
                periods.add(period_name);
        }
        this.pages_ = [...periods].sort();
    }

    _make_drawing_levels(shading) {
        this.drawing_levels_ = [];
        const page_period_name = this.pages_[this.page_no_];
        const in_page = antigen => this._antigen_period_name(antigen) === page_period_name;
        const antigens = this.chart().a;
        const chart_drawing_order = this.chart_drawing_order();
        if (shading === "legacy") {
            const drawing_order = [];
            for (let point_no of chart_drawing_order) {
                if (point_no >= antigens.length || (antigens[point_no].S && antigens[point_no].S.indexOf("R") >= 0 && !in_page(antigens[point_no])))
                    drawing_order.push(point_no);
            }
            for (let point_no of chart_drawing_order) {
                if (point_no < antigens.length && in_page(antigens[point_no]))
                    drawing_order.push(point_no);
            }
            this.drawing_levels_.push({drawing_order: drawing_order, style_modifier: style => style, type: "foreground"});
        }
        else {
            const drawing_order_foreground = [], drawing_order_background = [];
            for (let point_no of chart_drawing_order) {
                if (point_no < antigens.length && in_page(antigens[point_no]))
                    drawing_order_foreground.push(point_no);
                else
                    drawing_order_background.push(point_no);
            }
            this.drawing_levels_.push({drawing_order: drawing_order_background, style_modifier: sStyleModifiers[shading]});
            this.drawing_levels_.push({drawing_order: drawing_order_foreground, style_modifier: style => style, type: "foreground"});
        }
    }

    _antigen_period_name(antigen) {
        switch (this.period_) {
        case "year":
            return antigen.D && antigen.D.substr(0, 4);
        case "season":
        case "winter/summer":
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

    _period_chooser_populate(section) {
        const td = section.find("td.time-series-period");
        td.empty();
        const onchange = value => this.period(value);
        const selector = this.selector_use_select_ ? new SelectorSelect(td, onchange) : new SelectorButtons(td, onchange);
        selector.add("month");
        selector.add("winter/summer");
        selector.add("year");
        selector.current(this.period_);
        section.show();
    }
}

// ----------------------------------------------------------------------

class ViewTableSeries extends ViewingSeries
{
    name() {
        return "table series";
    }

    _initial_page_no() {
        return this.pages_.length - 1;
    }

    _shading_changed(shading) {
        this._make_drawing_levels(shading);
    }

    _make_pages() {
        const number_of_layers = this.chart_.t.L.length;
        const make_name = (source, index) => {
            if (source && source.D)
                return `${source.D} (${index + 1}/${number_of_layers})`;
            else
                return `Table ${index + 1}/${number_of_layers}`;
        };
        const sources = this.chart_.i.S || this.chart_.t.L;
        this.pages_ = sources.map(make_name);
    }

    _make_drawing_levels(shading) {
        this.drawing_levels_ = [];
        const antigens = this.chart().a;
        const chart_drawing_order = this.chart_drawing_order();
        const layer = this.chart().t.L[this.page_no_];
        const point_in_layer = point_no => {
            if (point_no < antigens.length)
                return Object.keys(layer[point_no]).length > 0;
            else
                layer.some(entry => !!entry["" + (point_no - antigens.length)]);
        };
        if (shading !== "legacy")
            this.drawing_levels_.push({drawing_order: chart_drawing_order.filter(point_no => !point_in_layer(point_no)), style_modifier: sStyleModifiers[shading]});
        this.drawing_levels_.push({drawing_order: chart_drawing_order.filter(point_no => point_in_layer(point_no)), style_modifier: style => style, type: "foreground"});
    }
}

// ----------------------------------------------------------------------

class ViewGroups extends ViewingSeries
{
    constructor(map_viewer, chart) {
        super(map_viewer, chart);
        this.pages_exclusive_ = ["*no-groups*"];
        this.groups_combined_ = [];
        this.combined_mode("exclusive");
    }

    name() {
        return "groups";
    }

    draw(coloring) {
        super.draw(coloring);
        this._draw_root_connecting_lines();
    }

    on_exit(view_dialog) {
        super.on_exit(view_dialog);
        if (view_dialog) {
            view_dialog.section("group-series").hide();
            view_dialog.section("group-series-combined").hide();
        }
    }

    on_entry(view_dialog) {
        super.on_entry(view_dialog);
        if (this.page_no_ === undefined)
            this.set_page(this._initial_page_no());
        else
            this._update_title();
        view_dialog && this._show_groups(view_dialog);
    }

    view_dialog_shown(view_dialog) {
        super.view_dialog_shown(view_dialog);
        this._show_groups(view_dialog);
    }

    set_page(page_no, redraw) {
        if (this.combined_mode() === "exclusive")
            super.set_page(page_no, redraw);
        else
            super.set_page(0, redraw);
    }

    combined_mode(new_mode) {
        if (typeof(new_mode) === "string") {
            this.combined_mode_ = new_mode;
            if (new_mode === "exclusive") {
                this.pages_ = this.pages_exclusive_;
            }
            else {
                this.pages_ = ["Multiple groups"];
            }
        }
        return this.combined_mode_;
    }

    _make_pages(group_set) {
        this.groups_ = group_set.groups;
        this.gs_line_color_ = group_set.line_color || "black";
        this.gs_line_width_ = group_set.line_width || 1;
        this.pages_exclusive_ = this.groups_.map(gr => gr.N);
        this.combined_mode(this.combined_mode());
        this.set_page(this._initial_page_no(), true);
    }

    _initial_page_no() {
        return 0;
    }

    _shading_changed(shading) {
        this._make_drawing_levels(shading);
    }

    _make_drawing_levels(shading) {
        this.drawing_levels_ = [{drawing_order: this.combined_mode() === "exclusive" ? this._make_drawing_order_exclusive() : this._make_drawing_order_combined(), type: "foreground", style_modifier: style => style}];
        if (shading !== "legacy") {
            const drawing_order = this.drawing_levels_[0].drawing_order;
            this.drawing_levels_.unshift({drawing_order: av_utils.array_of_indexes(this.chart_.a.length + this.chart_.s.length).filter(index => !drawing_order.includes(index)),
                                          style_modifier: sStyleModifiers[shading]});
        }
    }

    _make_drawing_order_exclusive() {
        const drawing_order = [];
        if (this.groups_ && this.groups_[this.page_no_])
            this._update_drawing_order(drawing_order, this.groups_[this.page_no_]);
        return drawing_order;
    }

    _make_drawing_order_combined() {
        const drawing_order = [];
        for (let group of this.groups_combined_)
            this._update_drawing_order(drawing_order, group);
        return drawing_order;
    }

    _update_drawing_order(drawing_order, group) {
        const add_member = point_no => {
            const index = drawing_order.indexOf(point_no);
            if (index >= 0)
                drawing_order.splice(index, 1);
            drawing_order.push(point_no);
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

    _draw_root_connecting_lines() {
        if (this.combined_mode() === "exclusive")
            this._draw_root_connecting_line(this.groups_ && this.groups_[this.page_no_]);
        else
            this.groups_combined_.forEach(group => this._draw_root_connecting_line(group));
    }

    _draw_root_connecting_line(group) {
        group = this._find_group(group);
        if (group && group.root !== undefined) {
            const line_color = group.line_color || this.gs_line_color_;
            const line_width = group.line_width || this.gs_line_width_;
            const drawing_order = this.drawing_levels_[this.drawing_levels_.length - 1].drawing_order;
            group.members.filter(point_no => drawing_order.includes(point_no))
                .filter(point_no => point_no !== group.root)
                .forEach(point_no => this.map_viewer_.surface_.line({start: this.layout_[point_no], end: this.layout_[group.root], color: line_color, width: line_width}));
        }
    }

    _show_groups(view_dialog) {
        const tr_groups = view_dialog.section("group-series").show();
        const tr_groups_combined = view_dialog.section("group-series-combined").hide();
        this._show_group_series_data(tr_groups, tr_groups_combined);
        this._make_uploader({button: tr_groups.find("a[href='upload']"), drop_area: view_dialog.content_.find("table.av-view-dialog"), tr_groups: tr_groups, tr_groups_combined: tr_groups_combined});
        this._make_downloader(tr_groups);
    }

    _show_group_series_data(tr_groups, tr_groups_combined) {
        if (!this.group_sets_ && this.chart_.group_sets)
            this.group_sets_ = this.chart_.group_sets;
        this._make_exclusive_combined(tr_groups_combined);
        if (this.group_sets_) {
            const group_sets = tr_groups.find(".av-sets").empty();
            if (this.group_sets_.length === 1) {
                const gs = this.group_sets_[0];
                group_sets.append(`<a class='av-current' href='${gs.N}'>${gs.N}</a>`);
                group_sets.find("a").on("click", evt => av_utils.forward_event(evt));
                this._populate_table_groups(gs, tr_groups_combined);
                this._make_pages(gs);
            }
            else {
                for (let gs of this.group_sets_)
                    group_sets.append(`<a href='${gs.N}'>${gs.N}</a>`);

                const switch_current_set = set_no => {
                    if (this.current_set_no_ !== set_no) {
                        this.current_set_no_ = set_no;
                        this.groups_combined_ = [];
                    }
                    $(group_sets.find("a")[this.current_set_no_]).addClass("av-current");
                    const gs = this.group_sets_[this.current_set_no_];
                    this._populate_table_groups(gs, tr_groups_combined);
                    this._make_pages(gs);
                };

                group_sets.find("a").on("click", evt => av_utils.forward_event(evt, evt => {
                    const target = $(evt.currentTarget);
                    if (!target.hasClass("av-current")) {
                        group_sets.find("a").removeClass("av-current");
                        switch_current_set(this.group_sets_.findIndex(gs => gs.N === evt.currentTarget.getAttribute("href")));
                    }
                }));
                switch_current_set(this.current_set_no_ || 0);
            }
        }
    }

    _make_exclusive_combined(tr_groups_combined) {
        tr_groups_combined.find(".av-buttons a").off("click");
        if (this.group_sets_) {
            tr_groups_combined.show();
            const button_exclusive = tr_groups_combined.find(".av-buttons a[href='exclusive']");
            const button_combined = tr_groups_combined.find(".av-buttons a[href='combined']");
            const table_groups = tr_groups_combined.find("table.av-groups");
            button_exclusive.off("click").on("click", evt => av_utils.forward_event(evt, evt => {
                this.combined_mode("exclusive");
                this.set_page(0, true);
                this._make_exclusive_combined(tr_groups_combined);
            }));
            button_combined.off("click").on("click", evt => av_utils.forward_event(evt, evt => {
                this.combined_mode("combined");
                this.set_page(0, true);
                this._make_exclusive_combined(tr_groups_combined);
            }));
            if (this.combined_mode() === "exclusive") {
                button_exclusive.addClass("av-current");
                button_combined.removeClass("av-current");
                table_groups.hide();
            }
            else {
                button_exclusive.removeClass("av-current");
                button_combined.addClass("av-current");
                table_groups.show();
            }
        }
        else {
            tr_groups_combined.hide();
        }
    }

    _populate_table_groups(group_set, tr_groups_combined) {
        const group_html = group_set.groups.map(group => {
            return `<tr><td class="av-checkbox"><input type="checkbox" name="${group.N}"></input></td><td class="av-name">${group.N}</td></tr>`;
        }).join("");
        const tbl = tr_groups_combined.find("table.av-groups").empty().append(group_html);
        tbl.find("input").on("change", evt => av_utils.forward_event(evt, evt => {
            if (evt.currentTarget.checked)
                this._add_combined_group(evt.currentTarget.name);
            else
                this._remove_combined_group(evt.currentTarget.name);
        }));
        this.groups_combined_.forEach(group_name => tbl.find(`input[name="${group_name}"]`).prop("checked", true));
    }

    _make_uploader(args) {
        av_utils.upload_json(args)
            .then(data => { this._show_group_series_uploaded_data(data, args.tr_groups, args.tr_groups_combined); this._make_uploader(args); })
            .catch(err => { av_toolkit.movable_window_with_error(err, args.button); this._make_uploader(args); });
    }

    _make_downloader(tr_group_series) {
        const button_download_sample = tr_group_series.find("a[href='download']");
        button_download_sample.off("click");
        button_download_sample.on("click", evt => av_utils.forward_event(evt, evt => {
            const data = {
                "  version": "group-series-set-v1",
                "a": this.chart_.a.map((antigen, ag_no) => Object.assign({"?no": ag_no}, antigen)),
                "s": this.chart_.s.map((serum, sr_no) => Object.assign({"?no": sr_no + this.chart_.a.length}, serum)),
                "group_sets": [{N: "set-1", line_color: "black", line_width: 1, groups: [{"N": "gr-1", line_color: "black", line_width: 1, root: 0, members: [0, 1, 2]}, {"N": "gr-2", root: 3, members: [3, 4, 5]}]}]
            };
            av_utils.download_blob({data: data, blob_type: "application/json", filename: "group-series-sets.json"});
        }));

        const button_download_chart = tr_group_series.find("a[href='download-chart']");
        button_download_chart.off("click");
        if (this.group_sets_) {
            button_download_chart.show().on("click", evt => av_utils.forward_event(evt, evt => {
                const data = {"  version": "acmacs-ace-v1", "?created": `ace-view/201807 GroupSeries on ${new Date()}`, c: Object.assign({}, this.chart(), {group_sets: this.group_sets_})};
                av_utils.download_blob({data: data, blob_type: "application/json", filename: "chart-with-group-series-sets.ace"});
            }));
        }
        else
            button_download_chart.hide();
    }

    _show_group_series_uploaded_data(data, tr_groups, tr_groups_combined) {
        try {
            this._check_group_sets(data);
            this._match_groups(data);
            this._show_group_series_data(tr_groups, tr_groups_combined);
            this._make_downloader(tr_groups);
        }
        catch (err) {
            av_toolkit.movable_window_with_error(err, tr_groups.find(".av-label"));
        }
    }

    _match_groups(data) {
        const point_to_point_ar = data.a.map((elt, no) => [no, this.chart_.a.findIndex(antigen => av_utils.objects_equal(elt, antigen, ["?no", "no", "C", "S", "c"]))])
              .concat(data.s.map((elt, no) => [no + data.a.length, this.chart_.a.length + this.chart_.s.findIndex(antigen => av_utils.objects_equal(elt, antigen, ["?no", "no", "S"]))]));
        const point_to_point = point_to_point_ar.reduce((obj, entry) => { obj[entry[0]] = entry[1]; return obj; }, {});
        for (let gs of data.group_sets) {
            for (let grp of gs.groups) {
                if (grp.root !== undefined)
                    grp.root = point_to_point[grp.root];
                grp.members = grp.members.map(no => point_to_point[no]);
            }
        }
        this.group_sets_ = data.group_sets;
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

    _add_combined_group(group) {
        if (!this.groups_combined_.includes(group)) {
            this.groups_combined_.push(group);
            this.set_page(this.page_no_, true);
        }
    }

    _remove_combined_group(group) {
        const index = this.groups_combined_.indexOf(group);
        if (index >= 0) {
            this.groups_combined_.splice(index, 1);
            this.set_page(this.page_no_, true);
        }
    }

} // class ViewGroups

// ----------------------------------------------------------------------

class Shading
{
    constructor(viewing) {
        this.viewing_ = viewing;
        this.shading_ = "shade";
    }

    hide() {
        this.section_ && this.section_.hide();
    }

    show(section) {
        if (section) {
            this.section_ = section;
            this._populate();
            this.section_.show();
        }
    }

    shading(new_shading) {
        if (new_shading !== undefined) {
            this.shading_ = new_shading;
            this.viewing_._shading_changed(new_shading);
            this.viewing_.map_viewer_.draw();
        }
        return this.shading_;
    }

    _populate() {
        const td = this.section_.find("td.shading");
        td.empty();
        const onchange = value => this.shading(value);
        const selector = this.selector_use_select_ ? new SelectorSelect(td, onchange) : new SelectorButtons(td, onchange);
        selector.add("shade");
        selector.add("grey");
        selector.add("legacy");
        selector.current(this.shading_);
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
