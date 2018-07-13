import * as av_utils from "./utils.js";
import * as av_toolkit from "./toolkit.js";
import * as av_viewing from "./viewing.js";

av_utils.load_css('/js/ad/map-draw/ace-view/201807/groups.css');

// ----------------------------------------------------------------------

export class ViewGroups extends av_viewing.ViewingSeries
{
    constructor(map_viewer, chart) {
        super(map_viewer, chart);
        this.group_sets_ = chart.group_sets || [];
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
        this.drawing_levels_ = [{drawing_order: this.combined_mode() === "exclusive" ? this._make_drawing_order_exclusive() : this._make_drawing_order_combined(), type: "foreground"}];
        if (shading !== "legacy") {
            const drawing_order = this.drawing_levels_[0].drawing_order;
            this.drawing_levels_.unshift({drawing_order: av_utils.array_of_indexes(this.chart_.a.length + this.chart_.s.length).filter(index => !drawing_order.includes(index)),
                                          shading: shading});
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
        this._make_exclusive_combined(tr_groups_combined);
        if (this.group_sets_.length) {
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
        if (this.group_sets_.length) {
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
        if (this.group_sets_.length) {
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

var groups_editor_dialog_singleton = null;

// {group_sets: [], chart: , antigens: [], sera: [], parent_element: $(<span>)}
export function groups_editor_add_points(args) {
    if (!groups_editor_dialog_singleton)
        groups_editor_dialog_singleton = new GroupsEditor(args.chart);
    groups_editor_dialog_singleton.group_sets(args.group_sets);
    groups_editor_dialog_singleton.add_antigens_sera(args);
    groups_editor_dialog_singleton.show(args.parent_element);
}

// ----------------------------------------------------------------------

const GroupsEditor_html = "\
<div class='av201807-group-editor av201807-window-shadow'>\
  <table>\
    <tr class='av-sets'>\
      <td class='av-sets-label'><span class='av-sets-label'>Sets</span></td>\
      <td class='av-separator'></td>\
      <td colspan='10' class='av-sets av-buttons'><span class='av-sets'></span><span class='av-new-set-button av-button' make_new='1'>new set</span></td>\
    </tr>\
    <tr class='av-groups'>\
      <td class='av-sets-label'><span class='av-sets-label'>Groups</span></td>\
      <td class='av-separator'></td>\
      <td class='av-groups'><table></table></td>\
      <td class='av-separator'></td>\
      <td class='av-group-members'><div class='av-scrollable-table'><table></table></div></td>\
      <td class='av-separator'></td>\
      <td class='av-selected-to-add'>\
        <div class='av-help'>click names below to (de)select, then click link under the table to add selected to the currently shown group</div>\
        <div class='av-scrollable-table'><table></table></div>\
        <div class='av-buttons'><span class='av-button av-add'>add selected to the group</span></div>\
      </td>\
    </tr>\
  </table>\
</div>\
";

class GroupsEditor
{
    constructor(chart) {
        this.chart_ = chart;
        this.group_sets_ = [];
        this._make();
    }

    show(parent_element) {
        const parent = $(parent_element);
        const parent_offset = parent.offset();
        const left = parent_offset.left > this.div_.width() ? parent_offset.left + parent.outerWidth() - this.div_.width() : 20;
        this.div_.css({left: left, top: parent_offset.top + parent.outerHeight()});
        this.modal_ = new av_toolkit.Modal({element: this.div_, z_index: 900, dismiss: () => this.hide()});
        this.div_.show();
    }

    hide() {
        this.modifier_canvas_ = null;
        this.div_.hide();
    }

    group_sets(group_sets) {
        if (group_sets !== undefined) {
            this.group_sets_ = group_sets;
            this._populate();
        }
        return this.group_sets_;
    }

    // {antigens: [], sera: []}
    add_antigens_sera(args) {
        this.to_add_ = (args.antigens || []).concat(args.sera || []);
        this._populate_to_add();
    }

    _make() {
        this.div_ = $(GroupsEditor_html).appendTo("body").hide().css({position: "absolute"});
        this.div_.find(".av-new-set-button").on("click", evt => av_utils.forward_event(evt, () => this._create_set()));
        this.div_.find(".av-new-group-button").on("click", evt => av_utils.forward_event(evt, () => this._create_group()));
        this.div_.find("td.av-selected-to-add").hide();
        this._populate();
    }

    // --------------------------------------------------

    _populate() {
        this._populate_set_names();
        this._populate_group_names();
        this._populate_current_group();
    }

    _populate_set_names() {
        if (!this.group_sets_.length)
            this._add_new_set();
        this.current_set_no_ = this.current_set_no_ || 0;
        const set_buttons = this.div_.find("tr.av-sets > td.av-sets > span.av-sets").empty();
        this.group_sets_.forEach((set, set_no) => {
            if (set_no === this.current_set_no_)
                set_buttons.append($(`<input type="text" class="av-set-name"></input>`).val(set.N).on("change paste keyup", evt => this._current_set_name_changed(evt.currentTarget.value)));
            else
                set_buttons.append(`<span class="av-set-name av-button" set_no="${set_no}">${set.N}</span>`);
            set_buttons.append(`<span class="av-separator"></span>`);
        });
        set_buttons.find("span.av-set-name[set_no]").on("click", evt => this._switch_set(parseInt(evt.currentTarget.getAttribute("set_no"))));
    }

    _switch_set(set_no) {
        this.current_set_no_ = set_no;
        this.current_group_no_ = 0;
        this._populate();
    }

    _create_set() {
        this._add_new_set();
        this._switch_set(this.group_sets_.length - 1);
    }

    _add_new_set() {
        const name = new Date().toLocaleString("en-CA", {hour12: false}).replace(",", "");
        const set_data = {N: name, groups: [{N: name, members: []}]};
        this.group_sets_.push(set_data);
        return set_data;
    }

    _current_set_name_changed(name) {
        if (name)
            this.group_sets_[this.current_set_no_].N = name;
    }

    // --------------------------------------------------

    _populate_group_names() {
        this.current_group_no_ = this.current_group_no_ || 0;
        const group_table = this.div_.find("tr.av-groups > td.av-groups > table").empty();
        this.group_sets_[this.current_set_no_].groups.forEach((group, group_no) => {
            if (group_no === this.current_group_no_) {
                const input = $(`<input type="text" class="av-group-name"></input>`).val(group.N).on("change paste keyup", evt => this._current_group_name_changed(evt.currentTarget.value));
                group_table.append($("<tr></tr>").append($("<td></td>").append(input)));
            }
            else
                group_table.append(`<tr><td><span class="av-group-name av-button" group_no="${group_no}">${group.N}</span></td></tr>`);
            group_table.append(`<tr><td class="av-separator"><td></tr>`);
        });
        const new_group = $("<span class='av-new-group-button av-button' make_new='1'>new group</span>").on("click", evt => av_utils.forward_event(evt, () => this._create_group()));
        group_table.append($("<tr></tr>").append($("<td></td>").append(new_group)));
        group_table.find("span.av-group-name.av-button[group_no]").on("click", evt => this._switch_group(parseInt(evt.currentTarget.getAttribute("group_no"))));
    }

    _populate_current_group() {
        const table = this.div_.find("tr.av-groups > td.av-group-members table").empty();
        const group = this.group_sets_[this.current_set_no_].groups[this.current_group_no_];
        if (group.members && group.members.length) {
            for (let member of group.members) {
                const no_class = member < this.chart_.a.length ? "av-antigen" : "av-serum";
                const root_class = member === group.root ? "av-group-memeber-root" : "";
                table.append(`<tr><td class='av-group-member-root' title='click to set root' member="${member}"><span class='${root_class}'>root</span></td><td class='av-group-member-no ${no_class}'>${member+1}</td><td class='av-group-member'>${this._member_name(member)}</td></tr>`);
            }
            table.find(".av-group-member-root[member]").on("click", evt => av_utils.forward_event(evt, evt => {
                group.root = parseInt(evt.currentTarget.getAttribute("member"));
                table.find(".av-group-member-root[member] > span").removeClass("av-group-memeber-root");
                $(evt.currentTarget).find("span").addClass("av-group-memeber-root");
            }));
        }
        else {
            table.append("<tr><td class='av-empty-group'>empty</td></tr>");
        }
    }

    _member_name(index) {
        if (index < this.chart_.a.length)
            return "<span class='av-antigen'>" + av_utils.ace_antigen_full_abbreviated_name(this.chart_.a[index], {escape: true}) + "</span>";
        else
            return "<span class='av-serum'>" + av_utils.ace_serum_full_abbreviated_name(this.chart_.s[index - this.chart_.a.length], {escape: true}) + "</span>";
    }

    _switch_group(group_no) {
        this.current_group_no_ = group_no;
        this._populate();
    }

    _create_group() {
        this._add_new_group();
        this._switch_group(this.group_sets_[this.current_set_no_].groups.length - 1);
    }

    _add_new_group() {
        const name = new Date().toLocaleString("en-CA", {hour12: false}).replace(",", "");
        const group_data = {N: name, members: []};
        this.group_sets_[this.current_set_no_].groups.push(group_data);
        return group_data;
    }

    _current_group_name_changed(name) {
        if (name)
            this.group_sets_[this.current_set_no_].groups[this.current_group_no_].N = name;
    }

    // --------------------------------------------------

    _populate_to_add() {
        if (this.to_add_ && this.to_add_.length) {
            const td = this.div_.find("td.av-selected-to-add").show();
            const table = td.find("> div.av-scrollable-table > table").empty();
            for (let no of this.to_add_) {
                const no_class = no < this.chart_.a.length ? "av-antigen" : "av-serum";
                table.append(`<tr no="${no}" title='click to (de)select'><td class='av-group-member-no ${no_class}'>${no+1}</td><td class='av-to-add av-to-add-selected' no="${no}">${this._member_name(no)}</td></tr>`);
            }
            table.find("tr").on("click", evt => av_utils.forward_event(evt, evt => {
                $(evt.currentTarget).find("td.av-to-add").toggleClass("av-to-add-selected");
            }));
            td.find(".av-button.av-add").off("click").on("click", evt => av_utils.forward_event(evt, evt => {
                const nos = table.find(".av-to-add-selected").map(function() { return $(this).attr("no"); }).get().map(val => parseInt(val));
                this._add_to_current_group(nos);
            }));
            window.setTimeout(() => { // set height of the to-add table
                const help = td.find(".av-help");
                const table = td.find("> div.av-scrollable-table");
                const table_padding_height = table.outerHeight() - table.height();
                help.css("width", table.outerWidth());
                table.css("height", this.div_.find("td.av-group-members > div.av-scrollable-table").outerHeight() - help.outerHeight() /* - td.find(".av-buttons").outerHeight() */ - table_padding_height);
            }, 10);
        }
    }

    _add_to_current_group(nos) {
        const group = this.group_sets_[this.current_set_no_].groups[this.current_group_no_];
        nos.filter(no => !group.members.includes(no)).forEach(no => group.members.push(no));
        this._populate_current_group();
    }

}

// ----------------------------------------------------------------------

// const GroupsEditor_html = "\
// <div class='av201807-group-editor av201807-window-shadow'>\
//   <table>\
//     <tr class='av-sets'>\
//       <td class='av-sets-label'><span class='av-sets-label'>Sets</span></td>\
//       <td colspan='4' class='av-sets av-buttons'><span class='av-sets'></span><span class='av-new-set-button av-button' make_new='1'>new set</span></td>\
//     </tr>\
//     <tr class='av-groups'>\
//       <td class='av-sets-label'><span class='av-sets-label'>Groups</span></td>\
//       <td class='av-groups av-buttons'><table></table></td>\
//       <td class='av-separator'></td>\
//       <td class='av-group-members'><div class='av-table'><table></table></div></td>\
//       <td class='av-selected-to-add'>\
//         <div class='av-help'>click names below to (de)select, then click link under the table to add selected to the currently shown group</div>\
//         <div class='av-table'><table></table></div>\
//         <div class='av-buttons'><span class='av-button av-add'>add selected to the group</span></div>\
//       </td>\
//     </tr>\
//   </table>\
// </div>\
// ";

// class GroupsEditor
// {
//     constructor(chart) {
//         this.chart_ = chart;
//         this.group_sets_ = [];
//         this.current_set_ = null;
//         this._make();
//     }

//     group_sets(group_sets) {
//         if (group_sets !== undefined) {
//             this.group_sets_ = group_sets;
//             this._populate_sets();
//         }
//         return this.group_sets_;
//     }

//     // {antigens: [], sera: []}
//     add_antigens_sera(args) {
//         this.to_add_ = (args.antigens || []).concat(args.sera || []);
//         this._populate_to_add();
//     }

//     show(parent_element) {
//         const parent = $(parent_element);
//         const parent_offset = parent.offset();
//         const left = parent_offset.left > this.div_.width() ? parent_offset.left + parent.outerWidth() - this.div_.width() : 20;
//         this.div_.css({left: left, top: parent_offset.top + parent.outerHeight()});
//         this.modal_ = new av_toolkit.Modal({element: this.div_, z_index: 900, dismiss: () => this.hide()});
//         this.div_.show();
//     }

//     hide() {
//         this.modifier_canvas_ = null;
//         this.div_.hide();
//     }

//     _make() {
//         this.div_ = $(GroupsEditor_html).appendTo("body").hide().css({position: "absolute"});
//         this.div_.find("tr.av-sets td.av-sets .av-new-set-button").on("click", evt => av_utils.forward_event(evt, () => this._create_set()));
//         this.div_.find("td.av-selected-to-add").hide();
//     }

//     _populate_sets() {
//         if (this.group_sets_.length) {
//             const sets_span = this.div_.find("tr.av-sets td.av-sets span.av-sets").empty();
//             for (let group_set of this.group_sets_)
//                 sets_span.append(this._set_button(group_set.N));
//             this._bind_set_buttons();
//             this._show_set(this.current_set_ ? this.current_set_.N : this.group_sets_[0].N);
//         }
//         else
//             this._create_set();
//         this._sets_label();
//     }

//     _set_button(name) {
//         return `<span class="av-button" name="${name}">${name}</span>`;
//     }

//     _sets_label() {
//         const sets_label = this.div_.find("tr.av-sets td.av-sets-label");
//         const groups = this.div_.find("tr.av-groups");
//         if (this.group_sets_.length) {
//             sets_label.find(".av-no-sets-label").hide();
//             sets_label.find(".av-sets-label").show();
//             groups.show();
//             this.div_.find("tr.av-groups > td.av-sets-label > .av-sets-label").show();
//             // const groups_label = this.div_.find("tr.av-groups td.av-sets-label");
//             // if (this.current_set_ && this.current_set_.groups.length) {
//             //     groups_label.find(".av-no-sets-label").hide();
//             //     groups_label.find(".av-sets-label").show();
//             // }
//             // else {
//             //     groups_label.find(".av-sets-label").hide();
//             //     groups_label.find(".av-no-sets-label").show();
//             // }
//         }
//         else {
//             sets_label.find(".av-sets-label").hide();
//             sets_label.find(".av-no-sets-label").show();
//             groups.hide();
//         }
//     }

//     _bind_set_buttons() {
//         this.div_.find("tr.av-sets td.av-sets span.av-sets .av-button").off("click").on("click", evt => av_utils.forward_event(evt, evt => this._show_set(evt.currentTarget.getAttribute("name"))));
//     }

//     _show_set(name, convert_created=true) {
//         if (convert_created)
//             this._convert_created_set();
//         this.current_set_ = this.group_sets_.find(set => set.N === name);
//         this.div_.find("tr.av-sets td.av-sets span.av-sets .av-button").removeClass("av-current");
//         this.div_.find(`tr.av-sets td.av-sets span.av-sets .av-button[name="${name}"]`).addClass("av-current");
//         this._populate_groups();
//         this._sets_label();
//         console.log("_show_set", this.current_set_);
//     }

//     _populate_groups() {
//         const groups_table = this.div_.find("tr.av-groups td.av-groups table").empty();
//         if (this.current_set_.groups.length) {
//             for (let group of this.current_set_.groups)
//                 groups_table.append(`<tr><td class="av-button" name="${group.N}">${group.N}</td></tr>`);
//             groups_table.append(`<tr><td class="av-button" make_new="1">new group</td></tr>`);
//             this._bind_group_buttons();
//             this._show_group(this.current_group_ ? this.current_group_.N : (this.current_set_.groups.length ? this.current_set_.groups[0].N : null));
//         }
//         else {
//             groups_table.append(`<tr><td class="av-button" make_new="1">new group</td></tr>`);
//             this._create_group();
//         }
//         groups_table.find(".av-button[make_new]").off("click").on("click", evt => av_utils.forward_event(evt, evt => this._create_group()));
//     }

//     _bind_group_buttons() {
//         this.div_.find("tr.av-groups td.av-groups.av-buttons .av-button[name]").off("click").on("click", evt => av_utils.forward_event(evt, evt => this._show_group(evt.currentTarget.getAttribute("name"))));
//     }

//     _show_group(name, convert_created=true) {
//         if (convert_created)
//             this._convert_created_group();
//         if (name) {
//             this.current_group_ = this.current_set_.groups.find(group => group.N === name);
//             this.div_.find("tr.av-groups td.av-groups table td.av-button[name]").removeClass("av-current");
//             this.div_.find(`tr.av-groups td.av-groups table td.av-button[name="${name}"]`).addClass("av-current");
//             this._populate_group();
//         }
//         else {
//             this.current_group_ = null;
//             this._populate_group();
//         }
//     }

//     _populate_group() {
//         const table = this.div_.find("tr.av-groups > td.av-group-members table").empty();
//         if (this.current_group_) {
//             if (this.current_group_.members && this.current_group_.members.length) {
//                 for (let member of this.current_group_.members) {
//                     const no_class = member < this.chart_.a.length ? "av-antigen" : "av-serum";
//                     const root_class = member === this.current_group_.root ? "av-group-memeber-root" : "";
//                     table.append(`<tr><td class='av-group-member-root' title='click to set root' member="${member}"><span class='${root_class}'>root</span></td><td class='av-group-member-no ${no_class}'>${member+1}</td><td class='av-group-member'>${this._member_name(member)}</td></tr>`);
//                 }
//                 table.find(".av-group-member-root[member]").on("click", evt => av_utils.forward_event(evt, evt => {
//                     this.current_group_.root = parseInt(evt.currentTarget.getAttribute("member"));
//                     table.find(".av-group-member-root[member] > span").removeClass("av-group-memeber-root");
//                     $(evt.currentTarget).find("span").addClass("av-group-memeber-root");
//                 }));
//             }
//             else {
//                 table.append("<tr><td class='av-empty-group'>empty</td></tr>");
//             }
//         }
//     }

//     _populate_to_add() {
//         if (this.to_add_ && this.to_add_.length) {
//             const td = this.div_.find("td.av-selected-to-add").show();
//             const table = td.find("> div.av-table > table");
//             for (let no of this.to_add_) {
//                 const no_class = no < this.chart_.a.length ? "av-antigen" : "av-serum";
//                 table.append(`<tr no="${no}" title='click to (de)select'><td class='av-group-member-no ${no_class}'>${no+1}</td><td class='av-to-add av-to-add-selected' no="${no}">${this._member_name(no)}</td></tr>`);
//             }
//             table.find("tr").on("click", evt => av_utils.forward_event(evt, evt => {
//                 $(evt.currentTarget).find("td.av-to-add").toggleClass("av-to-add-selected");
//             }));
//             td.find(".av-button.av-add").off("click").on("click", evt => av_utils.forward_event(evt, evt => {
//                 const nos = table.find(".av-to-add-selected").map(function() { return $(this).attr("no"); }).get().map(val => parseInt(val));
//                 this._add_to_current_group(nos);
//             }));
//             window.setTimeout(() => { // set height of the to-add table
//                 const help = td.find(".av-help");
//                 const table = td.find("> div.av-table");
//                 const table_padding_height = table.outerHeight() - table.height();
//                 help.css("width", table.outerWidth());
//                 table.css("height", this.div_.find("td.av-group-members > div.av-table").outerHeight() - help.outerHeight() - td.find(".av-buttons").outerHeight() - table_padding_height);
//             }, 10);
//         }
//     }

//     _add_to_current_group(nos) {
//         if (this.current_group_) {
//             nos.filter(no => !this.current_group_.members.includes(no)).forEach(no => this.current_group_.members.push(no));
//             this._populate_group();
//         }
//         else
//             console.error("_add_to_current_group: no current group");
//     }

//     _member_name(index) {
//         if (index < this.chart_.a.length)
//             return "<span class='av-antigen'>" + av_utils.ace_antigen_full_abbreviated_name(this.chart_.a[index], {escape: true}) + "</span>";
//         else
//             return "<span class='av-serum'>" + av_utils.ace_serum_full_abbreviated_name(this.chart_.s[index - this.chart_.a.length], {escape: true}) + "</span>";
//     }

//     _create_set() {
//         this._convert_created_set();
//         const name = new Date().toLocaleString("en-CA", {hour12: false}).replace(",", "");
//         const set_data = {N: name, groups: []};
//         this.group_sets_.push(set_data);
//         const input = $("<input type='text' class='av-set-name'></input>")
//               .appendTo(this.div_.find("tr.av-sets td.av-sets span.av-sets"))
//               .val(name).focus().select();
//         input.on("keypress", evt => {
//             if (evt.charCode === 13) {
//                 let new_name = evt.currentTarget.value || name;
//                 set_data.N = "*no-name*";
//                 if (this.group_sets_.find(set => set.N === new_name)) // avoid having name duplicate
//                     new_name = name;
//                 set_data.N = new_name;
//                 evt.currentTarget.value = set_data.N;
//                 input.select();
//             }
//         });
//         this._show_set(name, false);
//         this._sets_label();
//     }

//     _convert_created_set() {
//         const input = this.div_.find("tr.av-sets td.av-sets span.av-sets input");
//         if (input.length) {
//             input.remove();
//             this.div_.find("tr.av-sets td.av-sets span.av-sets").append(this._set_button(this.group_sets_[this.group_sets_.length - 1].N));
//             this._bind_set_buttons();
//         }
//     }

//     _create_group() {
//         this._convert_created_group();
//         const name = new Date().toLocaleString("en-CA", {hour12: false}).replace(",", "");
//         const group_data = {N: name, members: []};
//         this.current_set_.groups.push(group_data);
//         const tr_input = $("<tr><td><input type='text' class='av-group-name'></input><tr></td>")
//               .insertBefore(this.div_.find("tr.av-groups td.av-groups td[make_new]").parent());
//         const input = tr_input.find("input").val(name).focus().select();
//         input.on("keypress", evt => {
//             if (evt.charCode === 13) {
//                 let new_name = evt.currentTarget.value || name;
//                 group_data.N = "*no-name*";
//                 if (this.current_set_.groups.find(group => group.N === new_name)) // avoid having name duplicate
//                     new_name = name;
//                 group_data.N = new_name;
//                 evt.currentTarget.value = group_data.N;
//                 input.select();
//             }
//         });
//         this._show_group(name, false);
//     }

//     _convert_created_group() {
//         const input = this.div_.find("tr.av-groups td.av-groups input.av-group-name");
//         if (input.length) {
//             const name = this.current_set_.groups[this.current_set_.groups.length - 1].N;
//             input.parent().empty().addClass("av-button").attr("name", name).append(name);
//             this._bind_group_buttons();
//         }
//     }
// }

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
