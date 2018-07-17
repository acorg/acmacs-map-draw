import * as av_utils from "./utils.js";
import * as av_toolkit from "./toolkit.js";
import * as av_surface from "./surface.js";
import * as av_viewing from "./viewing.js";
import * as av_groups from "./groups.js";
import * as av_antigenic_table from "./antigenic-table.js";

av_utils.load_css('/js/ad/map-draw/ace-view/201807/ace-view.css');

// ----------------------------------------------------------------------

const AntigenicMapWidget_left_arrow = "&#x21E6;"; // "&#x27F8;";
const AntigenicMapWidget_right_arrow = "&#x21E8;"; // "&#x27F9;";
const AntigenicMapWidget_burger = "&#x2630;";

const AntigenicMapWidget_default_options = {
    canvas_size: null,          // number, null: auto-size
    point_on_click: null,       // (point, invoking_node) =>
    api: null,                  // new AntigenicMapApi()
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
          <li menu='download_pdf'>PDF</li>\
          <li class='av-menu-separator'></li>\
          <li menu='download_ace'>ace</li>\
          <li menu='download_save'>Lispmds Save</li>\
          <li class='av-menu-separator'></li>\
          <li menu='download_layout_plain'>Layout (plain text)</li>\
          <li menu='download_layout_csv'>Layout (csv)</li>\
          <li class='av-menu-separator'></li>\
          <li menu='download_table_map_distances_plain'>Table vs. Map Distances (plain text)</li>\
          <li menu='download_table_map_distances_csv'>Table vs. Map Distances (csv)</li>\
          <li menu='download_error_lines'>Error lines (csv)</li>\
          <li menu='download_distances_between_all_points_plain'>Distances Between All Points (plain text)</li>\
          <li menu='download_distances_between_all_points_csv'>Distances Between All Points (csv)</li>\
          <li class='av-menu-separator'></li>\
          <li menu='download_sequences_of_chart_as_fasta'>Sequences of antigens in the chart (nucs, fasta)</li>\
        </ul>\
      </li>\
      <li menu='table'>Table</li>\
      <li menu='raw'>Raw</li>\
      <li class='av-menu-separator'></li>\
      <li menu='help'>Help</li>\
      <!-- <li class='av-menu-disabled' menu='help'>Help</li> -->\
    </ul>\
";

const AntigenicMapWidget_help_html = "\
<div class='av-help'>\
  <h3>Mouse Wheel and Drag</h3>\
  <ul>\
    <li>Change point size - Shift-Wheel</li>\
    <li>Zoom - Alt/Option-Wheel</li>\
    <li>Move map - Alt/Option-Drag</li>\
    <li>Rotate map - Ctrl-Wheel</li>\
  </ul>\
  <h3>Table</h3>\
  <p>Table view can be opened via menu. If table view is on the screen,\
     hovering point(s) on map leads to highlighting corresponding\
     row/column in the table, table view is auto-scrolled.</p>\
</div>\
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
            $.getJSON(data).then(loaded, failed);
        }
        else if (typeof(data) === "function" && data.constructor.name === 'AsyncFunction') {
            this._loading_message();
            data().then(loaded, failed);
        }
        else if (typeof(data) === "object" && (data.version || data["  version"]) === "acmacs-ace-v1") {
            loaded(data);
        }
        else {
            console.error("unrecognized", data);
        }
    }

    _make_burger_menu() {
        const actions = {
            view: () => this._show_view_dialog(),
            download_pdf: item => this._external_api(item),
            download_ace: item => this._external_api(item),
            download_save: item => this._external_api(item),
            download_layout_plain: item => this._external_api(item),
            download_layout_csv: item => this._external_api(item),
            download_table_map_distances_plain: item => this._external_api(item),
            download_table_map_distances_csv: item => this._external_api(item),
            download_error_lines: item => this._external_api(item),
            download_distances_between_all_points_plain: item => this._external_api(item),
            download_distances_between_all_points_csv: item => this._external_api(item),
            download_sequences_of_chart_as_fasta: item => this._external_api(item),
            table: () => this._show_table(),
            raw: () => console.log("chart raw data", this.chart_),
            help: () => this._show_help()
        };
        this.burger_menu_ = new av_toolkit.BurgerMenu({menu: $(AntigenicMapWidget_burger_menu_html).appendTo("body"), trigger: this.div_.find(".av-burger"), callback: item => {
            const action = actions[item];
            if (action)
                action(item);
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

    _show_view_dialog() {
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

    _external_api(api_feature) {
        switch (api_feature) {
        case "download_pdf":
            this.options_.api.download_pdf({
                drawing_levels: this.viewer_.viewing_.drawing_levels(),
                projection_no: this.viewer_.viewing_.projection_no_,
                styles: this.viewer_.coloring_.styles(),
                point_scale: this.viewer_.surface_.point_scale_
            });
            break;
        default:
            if (this.options_.api[api_feature])
                this.options_.api[api_feature]();
            else
                console.warn("unrecognized api_feature: " + api_feature);
            break;
        }
    }

    _show_table() {
        if (!this.table_viewer_) {
            if (this.chart_)
                this.table_viewer_ = new av_antigenic_table.AntigenicTable({widget: this, parent: this.viewer_.surface_.canvas_, chart: this.chart_, on_destroy: () => delete this.table_viewer_});
        }
        else
            this.table_viewer_.position();
    }

    _show_help() {
        new av_toolkit.MovableWindow({title: "Help", content: AntigenicMapWidget_help_html, parent: this.viewer_.surface_.canvas_, id: "AntigenicMapWidget_help", classes: "av-help"});
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
      <span class='av-groups-button'>groups &blacktriangledown;</span>\
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

    coloring_modes_updated() {
        this._coloring_chooser(this.content_.find("table td.coloring"));
    }

    coloring_mode_switched() {
        this.coloring_chooser_selector_ && this.coloring_chooser_selector_.current(this.widget_.viewer_.coloring_.name());
    }

    _selector_toggle(section) {
        const checkbox = $("<input type='checkbox'></input>").appendTo(section);
        checkbox.prop("checked", this.selector_use_select_);
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
            section.append(`<span>${projection_title(this.chart_.P[0])}</span>`);
        else {
            const entries = this.chart_.P.map((prj, index) => `<option value="${index}">${projection_title(prj, index)}</option>`).join();
            const select = $(`<select>${entries}</select>`).appendTo(section);
            select.on("change", evt => av_utils.forward_event(evt, evt => this.widget_.viewer_.projection(parseInt(evt.currentTarget.value), true)));
        }
    }

    _coloring_chooser(section) {
        section.empty();
        const onchange = value => this.widget_.viewer_.coloring(value, true);
        this.coloring_chooser_selector_ = this.selector_use_select_ ? new SelectorSelect(section, onchange) : new SelectorButtons(section, onchange);
        for (let coloring_mode of this.widget_.viewer_.coloring_modes_)
            this.coloring_chooser_selector_.add(coloring_mode.name());
        this.coloring_chooser_selector_.current(this.widget_.viewer_.coloring_.name());
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

export class SelectorSelect
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

export class SelectorButtons
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
        this.viewing_ && this.viewing_.coloring_changed(this.coloring_);
        if (redraw)
            this.draw();
    }

    viewing(mode_name, redraw=false) {
        if (this.viewing_)
            this.viewing_.on_exit(this.widget_.view_dialog_);
        this.viewing_ = this.find_viewing(mode_name) || this.find_viewing("all");
        if (this.projection_no_ !== undefined)
            this.viewing_.projection(this.projection_no_);
        this.viewing_.on_entry(this.widget_.view_dialog_);
        if (redraw)
            this.draw();
    }

    find_viewing(name) {
        return this.viewing_modes_.find(mode => mode.name() === name);
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
            this.widget_.view_dialog_ && this.widget_.view_dialog_.coloring_modes_updated();
        }
        this.coloring(args.name);
        this.widget_.view_dialog_ && this.widget_.view_dialog_.coloring_mode_switched();
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

            const full_name = point_no => point_no < chart.a.length ? av_utils.ace_antigen_full_name(chart.a[point_no], {escape: true}) : av_utils.ace_serum_full_name(chart.s[point_no - chart.a.length], {escape: true});
            const make_point_name_row = point_entry => {
                if (this.widget_.options_.point_on_click)
                    return `<li><a href="show-info-on-this-name" point_no="${point_entry.no}" point_name="${point_entry.name}">${point_entry.name}</a></li>`;
                else
                    return `<li>${point_entry.name}</li>`;
            };

            const point_entries = points.map(point_no => { return {name: full_name(point_no), no: point_no}; });
            const mouse_popup_text = $("<ul class='point-info-on-hover'></ul>").append(point_entries.map(make_point_name_row).join(""));
            const popup = av_toolkit.mouse_popup_show(mouse_popup_text, this.surface_.canvas_, {left: mouse_offset.left + this.widget_.options_.mouse_popup_offset.left, top: mouse_offset.top + this.widget_.options_.mouse_popup_offset.top});
                if (this.widget_.options_.point_on_click) {
                    popup.find("a").on("click", evt => {
                        av_utils.forward_event(evt, evt => show_antigen_serum_info_from_hidb($(evt.target), chart, this.surface_.canvas_, this.widget_.options_.point_on_click));
                        window.setTimeout(av_toolkit.mouse_popup_hide, this.widget_.options_.point_info_on_hover_delay);
                    });
                }
            this.widget_.table_viewer_ && this.widget_.table_viewer_.show_points(points);
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
        this.viewing_modes_ = [new av_viewing.ViewAll(this, chart), new av_viewing.ViewSearch(this, chart)];
        if (chart.t.L && chart.t.L.length > 1)
            this.viewing_modes_.push(new av_viewing.ViewTableSeries(this, chart));
        if (chart.a.reduce((with_dates, antigen) => with_dates + (antigen.D ? 1 : 0), 0) > (chart.a.length * 0.25))
            this.viewing_modes_.push(new av_viewing.ViewTimeSeries(this, chart));
        this.viewing_modes_.push(new av_groups.ViewGroups(this, chart));
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

export class ColoringModified extends ColoringBase
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
        return this.styles_;
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
        if (view_dialog) {
            this.section_aa_at_pos_ = view_dialog.section("coloring-aa-pos").show();
            view_dialog.coloring_legend_.show();
        }
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
        this.section_aa_at_pos_ = view_dialog.section("coloring-aa-pos").show();
        view_dialog.coloring_legend_.populate(this.legend());
        view_dialog.coloring_legend_.show();
        if (this.sequences_) {
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
        return this.styles_;
    }

    set_positions(positions) {
        const update = positions !== this.positions_;
        if (update) {
            this.positions_ = positions;
            this._make_styles({set_point_rank: true});
            this.widget_.viewer_.viewing_ && this.widget_.viewer_.viewing_.coloring_changed(this);
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
            // this.section_aa_at_pos_ = this.widget_.view_dialog_.section("coloring-aa-pos").show();
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
        return this.styles_;
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

let sLastId = 0;

function new_id() {
    return "" + (++sLastId);
}

// ----------------------------------------------------------------------

export function show_antigen_serum_info_from_hidb(target, chart, invoking_node, shower) {
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
