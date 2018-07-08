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

    async sequences() {
        if (this.sequences_ === undefined) {
            this.sequences_ = "loading, please wait";
            return this.options_.api.get_sequences().then(
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

    view_dialog(func, args) {
        if (this.view_dialog_)
            return this.view_dialog_[func](args);
        else
            return undefined;
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
                if (!this.view_dialog_)
                    this.view_dialog_ = new ViewDialog({widget: this, chart: this.viewer_.viewing_.chart_, on_destroy: () => delete this.view_dialog_});
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

const ViewDialog_html = "\
<table class='av-view-dialog'>\
  <tr>\
    <td class='av-label'>Projection</td><td class='projection-chooser'></td>\
  </tr>\
  <tr>\
    <td class='av-label'>Coloring</td><td class='coloring'></td>\
  </tr>\
  <tr class='coloring-aa-pos'>\
    <td class='av-label'>Positions</td>\
    <td class='coloring-aa-pos'><input type='text'></input><a href='coloring-aa-pos-hint'>hint</a></td>\
  </tr>\
  <tr class='coloring-legend'>\
    <td class='av-label'>Legend</td>\
    <td class='coloring-legend'></td>\
  </tr>\
</table>\
";

//   <tr>\
//     <td class='a-label'>Mode</td>\
//     <td class='mode'></td>\
//   </tr>\
//   <tr class='time-series-period'>\
//     <td class='a-label'>Period</td>\
//     <td class='period'>\
//       <a href='month'>month</a>\
//       <a href='season'>winter/summer</a>\
//       <a href='year'>year</a>\
//     </td>\
//   </tr>\
//   <tr class='group-series'>\
//     <td class='a-label'>Group Sets</td>\
//     <td class='group-series'>\
//       <div class='a-sets'>\
//         <p class='a-hint'>please drop here group description file or click below to upload</p>\
//       </div>\
//       <div class='a-buttons'>\
//         <a href='upload' title='upload and apply group definition'>upload</a>\
//         <a href='download' title='download sample group definition for this chart'>download sample</a>\
//         <a href='download-chart' title='download chart in the .ace format with the embedded group data'>download chart</a>\
//       </div>\
//     </td>\
//   </tr>\
//   <tr class='group-series-combined'>\
//     <td class='a-label'>Groups</td>\
//     <td class='group-series-combined'>\
//       <div class='a-buttons'>\
//         <a href='exclusive'>exclusive</a>\
//         <a href='combined'>combined</a>\
//       </div>\
//       <table class='a-groups'>\
//       </table>\
//     </td>\
//   </tr>\
//   <tr class='shading'>\
//     <td class='a-label'>Shading</td>\
//     <td class='shading'>\
//       <a href='hide'>legacy</a>\
//       <a href='shade'>shade</a>\
//       <a href='grey'>grey</a>\
//     </td>\
//   </tr>\
//   <tr class='selection'>\
//     <td class='a-label'>RegEx</td>\
//     <td class='regex'>\
//       <input type='text'></input>\
//       <select name='selection-mode'>\
//         <option value='antigens'>antigens only</option>\
//         <option value='sera'>sera only</option>\
//         <option value='antigens+sera'>antigens and sera</option>\
//       </select>\
//     </td>\
//   <tr class='selection-results'>\
//     <td class='a-label'></td>\
//     <td class='selection-results'>\
//       <p class='a-message'>please enter regular expression above and press enter</p>\
//       <table></table>\
//     </td>\
//   </tr>\
// </table>\
// ";

class ViewDialog
{
    constructor(args) {
        this.widget_ = args.widget;
        this.chart_ = args.chart;
        this.canvas_ = this.widget_.viewer_.surface_.canvas_;
        this.selector_use_select_ = true;
        this.window_ = new av_toolkit.MovableWindow({
            title: "View",
            parent: this.canvas_,
            classes: "av201807-view-dialog",
            content_css: {width: "auto", height: "auto"},
            on_destroy: () => this.destroy()
        });
        this.content_ = this.window_.content();
        if (this.content_.find("table.view-dialog").length === 0)
            this.populate();
        this.on_destroy = args.on_destroy;
    }

    destroy() {
        if (this.on_destroy)
            this.on_destroy();
    }

    populate() {
        const table = $(ViewDialog_html).appendTo(this.content_);
        this._projection_chooser(table.find(".projection-chooser"));
        this._selector_toggle(table.find(".projection-chooser"));
        this._repopulate(table);
    }

    _repopulate(table) {
        this._coloring_chooser(table.find(".coloring"));
    }

    coloring_changed() {
        this._show_aa_at_pos();
    }

    viewing_changed() {
    }

    aa_at_pos_changed() {
        this._show_aa_at_pos();
        this._show_legend();
    }

    position() {
        this.window_.position(this.canvas_);
    }

    _projection_chooser(section) {
        if (this.chart_.P.length === 0)
            section.append("<div class='av-error'>None</div>");
        else if (this.chart_.P.length === 1)
            section.append(`<div>${this._projection_title(this.chart_.P[0])}</div>`);
        else {
            const entries = this.chart_.P.map((prj, index) => `<option value="${index}">${this._projection_title(prj, index)}</option>`).join();
            const select = $(`<select>${entries}</select>`).appendTo(section);
            select.on("change", evt => av_utils.forward_event(evt, evt => this.widget_.viewer_.projection(parseInt(evt.currentTarget.value), true)));
        }
    }

    _projection_title(projection, index) {
        return av_utils.join_collapse([
            index === undefined ? null : "" + (index + 1) + ".",
            projection.s.toFixed(4),
            "&ge;" + (projection.m || "none"),
            projection.C ? "forced-col-bases" : null,
            projection.c
        ]);
    }

    _selector_toggle(section) {
        const checkbox = $(`<input type='checkbox' checked='{this.selector_use_select_}'></input>`).appendTo(section);
        checkbox.on("change", evt => av_utils.forward_event(evt, evt => {
            this.selector_use_select_ = evt.currentTarget.checked;
            this._repopulate(this.content_.find("table"));
        }));
    }

    _coloring_chooser(section) {
        section.empty();
        const onchange = value => {
            // console.log("_coloring_chooser onchange", value);
            this.widget_.viewer_.coloring(value, true);
            this._show_legend();
        };
        const selector = this.selector_use_select_ ? new SelectorSelect(section, onchange) : new SelectorButtons(section, onchange);
        for (let coloring_mode of this.widget_.viewer_.coloring_modes_)
            selector.add(coloring_mode.name());
        selector.current(this.widget_.viewer_.coloring_.name());
        this._show_aa_at_pos();
        this._show_legend();
    }

    _show_legend() {
        const legend = this.widget_.viewer_.coloring_.legend();
        if (legend) {
            const td_legend = this.content_.find("tr.coloring-legend").show().find("td.coloring-legend").empty();
            td_legend.append("<table class='av-legend'><tr class='av-names'></tr><tr class='av-colors'></tr></table>");
            legend.map(entry => {
                td_legend.find("tr.av-names").append(`<td>${entry.name}</td>`);
                if (entry.color !== undefined && entry.color !== null)
                    td_legend.find("tr.av-colors").append(`<td><span class="av-color" style="background-color: ${entry.color}">__</span>${entry.count || ""}</td>`);
            });
        }
        else {
            this.content_.find("tr.coloring-legend").hide();
        }
    }

    _show_aa_at_pos() {
        const tr = this.content_.find("tr.coloring-aa-pos");
        const input = tr.find("input").off("keypress");
        const hint = tr.find("a").off("click");
        if (this.widget_.viewer_.coloring_ instanceof ColoringByAAatPos) {
            input.on("keypress", evt => {
                if (evt.charCode === 13) {
                    if (this.widget_.viewer_.coloring_.set_positions(evt.currentTarget.value.split(/[^0-9]/).filter(entry => !!entry))) {
                        this.widget_.viewer_.draw();
                        this._show_legend();
                    }
                }
            });
            hint.on("click", evt => av_utils.forward_event(evt, evt => this._aa_positions_hint($(evt.currentTarget))));
            const positions = this.widget_.viewer_.coloring_.positions();
            if (positions && positions.length)
                input.val(positions.join(" "));
            tr.show();
            window.setTimeout(() => input.focus(), 10);
        }
        else
            tr.hide();
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
            Object.entries(this.widget_.sequences_.per_pos).sort(sort_by === "shannon" ? compare_shannon : compare_position).forEach(entry => {
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
            if (this.widget_.sequences_ && typeof(this.widget_.sequences_) !== "string")
                fill();
            else
                window.setTimeout(wait_fill, 100);
        };
        wait_fill();
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
        this.viewing_.draw(this.surface_, this.coloring_);
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
        this.coloring_ = this.coloring_modes_.find(mode => mode.name() === mode_name) || this.coloring_modes_.find(mode => mode.name() === "original");
        this.coloring_.start_using();
        if (redraw)
            this.draw();
        this.widget_.view_dialog("coloring_changed");
    }

    viewing(mode_name, redraw=false) {
        this.viewing_ = this.viewing_modes_.find(mode => mode.name() === mode_name) || this.viewing_modes_.find(mode => mode.name() === "all");
        if (this.projection_no_ !== undefined)
            this.viewing_.projection(this.projection_no_);
        if (redraw)
            this.draw();
        this.widget_.view_dialog("viewing_changed");
    }

    projection(projection_no, redraw=false) {
        this.projection_no_ = projection_no;
        this.viewing_.projection(this.projection_no_);
        if (redraw)
            this.draw();
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

class ColoringBase
{
    constructor(widget) {
        this.widget_ = widget;
        this.chart_ = widget.chart_;
    }

    drawing_order(drawing_order) {
        return drawing_order || av_utils.array_of_indexes(this.chart_.a.length + this.chart_.s.length);
    }

    legend() {
        return null;
    }

    start_using() {
    }

    point_style(point_no) {
        return this.styles_[point_no];
    }

    // {reset_sera: false}
    all_styles(args={}) {
        const egg_passage = (style, index) => {
            if (index < this.chart_.a.length && (!style.a || style.a === 1) && this.chart_.a[index].S && this.chart_.a[index].S.indexOf("E") >= 0)
                style.a = 0.75;
            if (index < this.chart_.a.length && (!style.r || style.r === 0) && this.chart_.a[index].R && this.chart_.a[index].R.length)
                style.r = Math.PI / 6;
            return style;
        };
        let all_styles = this.chart_.p.p.map(style_no => Object.assign({}, this.chart_.p.P[style_no])).map(egg_passage);
        if (args.reset_sera) {
            this.chart_.s.forEach((serum, serum_no) => {
                delete all_styles[serum_no + this.chart_.a.length].F;
                all_styles[serum_no + this.chart_.a.length].O = av_toolkit.sGREY;
            });
        }
        return all_styles;
    }
}

class ColoringOriginal extends ColoringBase
{
    name() {
        return "original";
    }

    point_style(point_no) {
        const plot_spec = this.chart_.p;
        return plot_spec.P[plot_spec.p[point_no]];
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

    start_using() {
        if (!this.styles_) {
            this._make_antigens_by_clade({set_clade_for_antigen: true});
            this._make_styles();
        }
    }

    drawing_order(drawing_order) {
        // order: sera, not sequenced, sequenced without clade, clade with max number of antigens, ..., clade with fewer antigens
        return super.drawing_order(drawing_order).slice(0).sort((p1, p2) => this.point_rank_[p1] - this.point_rank_[p2]);
    }

    legend() {
        return this.clade_order_.filter(clade => !!clade).map(clade => { return {name: clade, count: this.clade_to_number_of_antigens_[clade], color: sCladeColors[clade]}; });
    }

    _make_antigens_by_clade(args) {
        const clade_sorting_key = clade => (clade === "GLY" || clade === "NO-GLY" || clade === "SEQUENCED") ? 0 : clade.length;
        this.clade_to_number_of_antigens_ = {};
        if (args && args.set_clade_for_antigen)
            this.clade_for_antigen_ = Array.apply(null, {length: this.chart_.a.length}).map(() => "");
        super.drawing_order().filter(no => no < this.chart_.a.length).forEach(antigen_no => {
            const clades = (this.chart_.a[antigen_no].c || []).sort((a, b) => clade_sorting_key(b) - clade_sorting_key(a));
            let clade = clades.length > 0 ? clades[0] : "";
            if (clade === "GLY" || clade === "NO-GLY")
                clade = "SEQUENCED";
            this.clade_to_number_of_antigens_[clade] = (this.clade_to_number_of_antigens_[clade] || 0) + 1;
            if (args && args.set_clade_for_antigen)
                this.clade_for_antigen_[antigen_no] = clade;
        });
        this.clade_order_ = Object.keys(this.clade_to_number_of_antigens_).sort((a, b) => this._clade_rank(a) - this._clade_rank(b));
        if (args && args.set_clade_for_antigen)
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

    start_using() {
        if (!this.styles_) {
            this._make_styles({set_point_rank: true});
            this.widget_.sequences().then(data => this._sequences_received(data)).catch(error => console.log("Coloring_AAPos::constructor sequences error", error));
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

    _make_legend(drawing_order) {
        const aa_count = this.antigen_aa_.reduce((count, entry) => {
            if (drawing_order.includes(entry.no))
                count[entry.aa] = (count[entry.aa] || 0) + 1;
            return count;
        }, {});
        this.legend_ = this.aa_order_.map((aa, index) => aa_count[aa] ? {name: aa, count: aa_count[aa], color: av_toolkit.ana_colors(index)} : null).filter(elt => !!elt);
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
            this.styles_[antigen_no].F = this.styles_[antigen_no].O = av_toolkit.sLIGHTGREY;
        });
    }

    _sequences_received(data) {
        this.sequences_ = data;
        this.widget_.view_dialog("aa_at_pos_changed");
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

    start_using() {
        if (!this.styles_) {
            this.styles_ = this.all_styles({reset_sera: true});
            this._make_continent_count({set_styles: true});
        }
    }

    drawing_order(original_drawing_order) {
        // order: sera, most popular continent, ..., lest popular continent
        const continent_order = this.continent_count_.map(entry => entry.name);
        const ranks = Array.apply(null, {length: this.chart_.a.length}).map((_, ag_no) => continent_order.indexOf(this.chart_.a[ag_no].C) + 10).concat(Array.apply(null, {length: this.chart_.s.length}).map(_ => 0));
        const drawing_order = super.drawing_order(original_drawing_order).slice(0).sort((p1, p2) => ranks[p1] - ranks[p2]);
        this._make_continent_count({drawing_order: drawing_order});
        return drawing_order;
    }

    legend() {
        return this.continent_count_.map(entry => Object.assign({}, entry, {name: continent_name_for_legend[entry.name] || entry.name}));
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

    draw(surface, coloring) {
        for (let point_no of coloring.drawing_order(this.drawing_order())) {
            this.map_viewer_.surface_.point(this.layout_[point_no], coloring.point_style(point_no), point_no, true);
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
