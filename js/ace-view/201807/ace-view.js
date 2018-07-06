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

export class AntigenicMapWidget
{
    constructor(div, data, options={}) { // options: {view_mode: {mode: "table-series"}, coloring: "original", title_fields: [], api: object_providing_external_api}
        this.div_ = $(div);
        this.options_ = Object.assign({}, AntigenicMapWidget_default_options, options);
        this.div_.addClass("amw201807").attr("amw201805_id", new_id()).append(AntigenicMapWidget_content_html);
        this.viewer_ = new MapViewer(this.div_.find("canvas"));
        this._viewer_size();
        this._bind();
        this._load_and_draw(data);
    }

    draw_data(data=undefined) {
        if (this.loading_message_) {
            this.loading_message_.remove();
            delete this.loading_message_;
        }
        console.log("draw_data", data);
        this.viewer_.start();
        this.viewer_.draw();
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

    _bind() {
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

class MapViewer
{
    constructor(canvas) {
        this.surface_ = new av_surface.Surface(canvas);
    }

    start() {
        this.surface_.add_resizer(width_diff => { this.surface_.resize(width_diff); this.draw(); });
    }

    draw() {
        this.surface_.reset();
        this.surface_.grid();
        this.surface_.border();

        // this.points();
        // this.lines();
    }

    resize(new_size) {
        this.surface_.resize(new_size);
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
