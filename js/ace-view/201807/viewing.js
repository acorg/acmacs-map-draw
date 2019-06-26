import * as av_utils from "./utils.js";
import * as av_toolkit from "./toolkit.js";
import * as av_point_style from "./point-style.js";
import * as av_ace_view from "./ace-view.js";

av_utils.load_css('/js/ad/map-draw/ace-view/201807/viewing.css');

// ----------------------------------------------------------------------

export class ViewingBase
{
    constructor(map_viewer, chart) {
        this.map_viewer_ = map_viewer;
        this.chart_ = chart;
    }

    draw(coloring) {
        for (let drawing_level of this.drawing_levels()) {
            for (let point_no of coloring.drawing_order(drawing_level.drawing_order)) {
                this.map_viewer_.surface_.point(this.layout_[point_no], sStyleModifiers[drawing_level.shading](coloring.point_style(point_no)), point_no, true);
            }
            coloring.update_for_drawing_level(drawing_level);
        }
    }

    projection(projection_no) {
        if (projection_no < this.chart_.P.length) {
            this.projection_no_ = projection_no;
            this.layout_ = this.chart_.P[this.projection_no_].l;
            this.map_viewer_.surface_.transformation(this.chart_.P[this.projection_no_].t);
            let viewport = this.map_viewer_.widget_.options_.viewport;
            try {
                if (viewport.length === 3)
                    viewport = [viewport[0], viewport[1], viewport[2], viewport[2]];
                else
                    viewport = [viewport[0], viewport[1], viewport[2], viewport[3]];
            }
            catch (err) {
                viewport = this._calculate_viewport();
            }
            console.log("viewport=" + viewport.slice(0, 3).join(","));
            this.map_viewer_.surface_.viewport(viewport);
        }
        else {
            console.log("invalid projection_no", projection_no);
        }
    }

    chart() {
        return this.chart_;
    }

    chart_drawing_order() {
        return (this.chart_.p && this.chart_.p.d) || av_utils.array_of_indexes(this.layout_ ? this.layout_.length : 0);
    }

    on_exit(view_dialog) {
    }

    on_entry(view_dialog) {
    }

    view_dialog_shown(view_dialog) {
    }

    coloring_changed(coloring) {
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

function style_modifier_none(style) {
    return style;
}

const sStyleModifiers = {shade: style_modifier_shade, grey: style_modifier_grey, legacy: style_modifier_legacy, null: style_modifier_none, undefined: style_modifier_none};

// ----------------------------------------------------------------------

export function title_field_makers()
{
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

// ----------------------------------------------------------------------

export class ViewAll extends ViewingBase
{
    name() {
        return "all";
    }

    drawing_levels() {
        return [
            {drawing_order: this.chart_drawing_order(), type: "foreground"}
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
        const makers = title_field_makers();
        return this.projection_no_ !== undefined ? av_utils.join_collapse(args.title_fields.map(field => makers[field](this.chart_, this.projection_no_))) : "";
    }
}

// ----------------------------------------------------------------------

export class ViewSearch extends ViewingBase
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
        if (view_dialog) {
            this.search_section_ = view_dialog.section("search").show();
            this.search_results_section_ = view_dialog.section("search-results").show();
            this._bind();
            this.shading_.show(view_dialog && view_dialog.section("shading"));
            this.map_viewer_.widget_.update_title();
        }
    }

    view_dialog_shown(view_dialog) {
        super.view_dialog_shown(view_dialog);
        this.search_section_ = view_dialog.section("search").show();
        this.search_results_section_ = view_dialog.section("search-results").show();
        this._bind();
        this.shading_.show(view_dialog.section("shading"));
        this.map_viewer_.widget_.update_title();
    }

    coloring_changed(coloring) {
        this._filter(this.search_section_.find("td.regex input").val());
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
            {drawing_order: this.drawing_order_background_, shading: this.shading_.shading_},
            {drawing_order: this.drawing_order_foreground_, type: "foreground"}
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
            this._handle_groups_button();
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
        if (! (this.map_viewer_.coloring_ instanceof av_ace_view.ColoringModified))
            this.map_viewer_.make_coloring_modified({name: new Date().toLocaleString("en-CA", {hour12: false}).replace(",", "")});
        const styles = this.map_viewer_.coloring_.styles();
        indexes.forEach(index => styles[index][data.name] = data.value);
        this.map_viewer_.draw();
    }

    _handle_groups_button() {
        const groups_button = this.search_section_.find("td.regex .av-groups-button");
        if (this.selected_antigens_.length || this.selected_sera_.length) {
            groups_button.show().off("click").on("click", evt => av_utils.forward_event(evt, evt => {
                const groups_viewing = this.map_viewer_.find_viewing("groups");
                if (groups_viewing)
                    import("./groups.js").then(av_groups => av_groups.groups_editor_add_points({group_sets: groups_viewing.group_sets_, chart: this.chart_, antigens: this.selected_antigens_, sera: this.selected_sera_, parent_element: evt.currentTarget}));
                else
                    console.error("ViewSearch::_handle_groups_button: no groups viewing");
            }));
        }
        else
            groups_button.hide();
    }

}

// ----------------------------------------------------------------------

export class ViewingSeries extends ViewingBase
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

export class ViewTimeSeries extends ViewingSeries
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
            this.drawing_levels_.push({drawing_order: drawing_order, type: "foreground"});
        }
        else {
            const drawing_order_foreground = [], drawing_order_background = [];
            for (let point_no of chart_drawing_order) {
                if (point_no < antigens.length && in_page(antigens[point_no]))
                    drawing_order_foreground.push(point_no);
                else
                    drawing_order_background.push(point_no);
            }
            this.drawing_levels_.push({drawing_order: drawing_order_background, shading: shading});
            this.drawing_levels_.push({drawing_order: drawing_order_foreground, type: "foreground"});
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
        const selector = this.selector_use_select_ ? new av_ace_view.SelectorSelect(td, onchange) : new av_ace_view.SelectorButtons(td, onchange);
        selector.add("month");
        selector.add("winter/summer");
        selector.add("year");
        selector.current(this.period_);
        section.show();
    }
}

// ----------------------------------------------------------------------

export class ViewTableSeries extends ViewingSeries
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
                return layer.some(entry => !!entry["" + (point_no - antigens.length)]);
        };
        if (shading !== "legacy")
            this.drawing_levels_.push({drawing_order: chart_drawing_order.filter(point_no => !point_in_layer(point_no)), shading: shading});
        this.drawing_levels_.push({drawing_order: chart_drawing_order.filter(point_no => point_in_layer(point_no)), type: "foreground"});
    }
}

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
        const selector = this.selector_use_select_ ? new av_ace_view.SelectorSelect(td, onchange) : new av_ace_view.SelectorButtons(td, onchange);
        selector.add("shade");
        selector.add("grey");
        selector.add("legacy");
        selector.current(this.shading_);
    }
}

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
