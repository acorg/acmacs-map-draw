import * as av_utils from "./utils.js";
import * as av_toolkit from "./toolkit.js";
import * as av_ace_view from "./ace-view.js";

av_utils.load_css('/js/ad/map-draw/ace-view/201807/antigenic-table.css');

// ----------------------------------------------------------------------

export class AntigenicTable
{
    // {widget: , parent: this.viewer_.surface_.canvas_, chart:, on_destroy: () => }
    constructor(args) {
        this.widget_ = args.widget;
        this.parent_ = args.parent;
        this.chart_ = args.chart;
        this.populate(args.on_destroy);
        this.position();
    }

    position() {
        this.window_.position(this.parent_);
    }

    populate(on_destroy) {
        const title_fields = ["name", "lab", "virus_type", "assay", "date"];
        const makers = av_ace_view.title_field_makers();
        const win_space = $(window).height() - (this.parent_.offset().top - $(window).scrollTop());
        const max_height = Math.max(win_space - this.parent_.height() / 2 - 20, 400);
        this.window_ = new av_toolkit.MovableWindow({
            title: av_utils.join_collapse(title_fields.map(field => makers[field](this.chart_))),
            parent: this.parent_,
            classes: "av-antigenic-table",
            content_css: {width: "auto", height: "auto", "max-height": max_height},
            on_destroy: on_destroy
        });
        new AntigenicTable_populate({widget: this.widget_, content: this.window_.content(), chart: this.chart_, parent: this.parent_});

        const movable_window_content = this.window_.content();
        const target_height = $(window).height() - (movable_window_content.parent().outerHeight() - movable_window_content.height()) * 1.2;
        const target_width = $(window).width() - (movable_window_content.parent().outerWidth() - movable_window_content.width()) * 1.2;
        movable_window_content.css({"max-height": "", "max-width": ""});
        movable_window_content.css({"height": Math.min(movable_window_content.height(), target_height), "width": Math.min(movable_window_content.width(), target_width)});
        this.table_ = this.window_.content().find(".antigenic-table");
    }

    show_points(points) {
        if (this.table_) {
            this.not_show_points();
            points.forEach((point_no, index) => {
                if (point_no < this.chart_.a.length) {
                    this.show_antigen(point_no);
                    if (index === 0)
                        this.scroll_to_antigen(point_no);
                }
                else {
                    const serum_no = point_no - this.chart_.a.length;
                    this.show_serum(serum_no);
                    if (index === 0)
                        this.scroll_to_serum(serum_no);
                }
            });
        }
    }

    not_show_points() {
        this.table_.find("tr.av-antigen").removeClass("av-highlight");
        this.table_.find("td[serum_no]").removeClass("av-highlight");
    }

    show_antigen(antigen_no) {
        this.table_.find(`tr.av-antigen[antigen_no='${antigen_no}']`).addClass("av-highlight");
    }

    show_serum(serum_no) {
        this.table_.find(`td[serum_no='${serum_no}']`).addClass("av-highlight");
    }

    scroll_to_antigen(antigen_no) {
        const row = this.table_.find(`tr.av-antigen[antigen_no='${antigen_no}']`);
        if (row.length > 0) {
            const row_top = row.position().top;
            const scrollable = this.table_.parent();
            if (row_top < scrollable.scrollTop())
                scrollable.animate({scrollTop: row_top, scrollLeft: 0}, 500);
            else if (row_top > (scrollable.scrollTop() + scrollable.height()))
                scrollable.animate({scrollTop: row_top - scrollable.height() * 0.9, scrollLeft: 0}, 500);
            else if (scrollable.scrollLeft() > 0)
                scrollable.animate({scrollLeft: 0}, 500);
        }
    }

    scroll_to_serum(serum_no) {
        const column = this.table_.find(`td.av-serum-name[serum_no='${serum_no}']`);
        if (column.length > 0) {
            const column_left = column.position().left;
            const scrollable = this.table_.parent();
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

const AntigenicTable_serum_rows_html = "\
<tr class='av-serum-nos'>\
 <td></td>\
 <td></td>\
 ${nos}\
</tr>\
<tr class='av-serum-names'>\
 <td></td>\
 <td></td>\
 ${names}\
</tr>\
";

const AntigenicTable_antigen_row_html = "\
<tr class='av-antigen' antigen_no='${no0}'>\
 <td class='av-antigen-no'>${no1}</td>\
 <td class='av-antigen-name'>${name}</td>\
 ${titers}\
</tr>\
";

class AntigenicTable_populate
{
    constructor(args) {
        this.widget_ = args.widget;
        this.chart_ = args.chart;
        this.parent_ = args.parent;
        if (this.chart_.a.length < 200000) {
            this.div = $("<table class='antigenic-table'></table>").appendTo(args.content);
            this.make_sera();
            this.make_antigens();
            this.set_size(args.content);
        }
        else {
            this.div = $(`<p class='av-error-message'>Table is too big: ${this.chart_.a.length} antigens</table>`).appendTo(args.content);
        }
    }

    make_sera() {
        this.div.append(av_utils.format(AntigenicTable_serum_rows_html, {
            nos: av_utils.array_of_indexes(this.chart_.s.length, 1).map(no => `<td class='av-serum-no' serum_no='${no-1}'>${no}</td>`).join(""),
            names: this.chart_.s.map((serum, serum_no) => this.make_serum_name(serum, serum_no)).map((text, serum_no) => `<td class='av-serum-name' serum_no='${serum_no}'>${text}</td>`).join("")
        }));
    }

    make_antigens() {
        const chunk_size = 50;
        let antigen_no = 0;
        const populate = () => {
            const last = Math.min(antigen_no + 50, this.chart_.a.length);
            for (; antigen_no < last; ++antigen_no) {
                const antigen = this.chart_.a[antigen_no];
                this.div.append(av_utils.format(AntigenicTable_antigen_row_html, {no0: antigen_no, no1: antigen_no + 1, name: this.make_antigen_name(antigen, antigen_no), titers: this.make_titers_for_antigen(antigen_no)}));
            }
            this.show_antigen_serum_info();
            if (last < this.chart_.a.length)
                window.setTimeout(populate, 0);
        };
        populate();
    }

    make_antigen_name(antigen, antigen_no) {
        const title = "title='" + av_utils.ace_antigen_full_name(antigen, {escape: true}) + "'";
        const abbr_name = av_utils.antigen_serum_abbreviated_name(antigen.N);
        if (this.widget_.options_.point_on_click)
            return `<a ${title} point_no='${antigen_no}' href='#antigen-info-from-hidb'>${abbr_name}</a>`;
        else
            return `<span ${title}>${abbr_name}</span>`;
    }

    make_serum_name(serum, serum_no) {
        const title = "title='" + av_utils.ace_serum_full_name(serum, {escape: true}) + "'";
        const abbr_name = av_utils.antigen_serum_abbreviated_name(serum.N, {exclude_year: true});
        if (this.widget_.options_.point_on_click)
            return `<a ${title} point_no='${serum_no + this.chart_.a.length}' href='#serum-info-from-hidb'>${abbr_name}</a>`;
        else
            return `<span ${title}>${abbr_name}</span>`;
    }

    make_titers_for_antigen(antigen_no) {
        return this.chart_.s.map((serum, serum_no) => {
            const tt = av_utils.ace_titer(this.chart_, antigen_no, serum_no);
            let cls = "";
            switch (tt[0]) {
            case "*":
                cls = "av-titer-dontcare";
                break;
            case "<":
                cls = "av-titer-thresholded av-titer-less";
                break;
            case ">":
                cls = "av-titer-thresholded av-titer-more";
                break;
            case "~":
                cls = "av-titer-dodgy";
                break;
            default:
                cls = "av-titer-numeric";
                break;
            }
            const ag_name = av_utils.ace_antigen_full_name(this.chart_.a[antigen_no]);
            const sr_name = av_utils.ace_serum_full_name(this.chart_.s[serum_no]);
            return `<td class='av-titer ${cls}' serum_no='${serum_no}' title='AG: ${ag_name}\nSR: ${sr_name}'>${tt}</td>`;
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
        this.div.find("a[title]").on("click", evt => av_utils.forward_event(evt, () => av_ace_view.show_antigen_serum_info_from_hidb($(evt.target), this.chart_, this.parent_, this.widget_.options_.point_on_click)));
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
