import * as acv_utils from "./utils.js";

// ----------------------------------------------------------------------

class AMW201805
{
    constructor() {
        this.last_id_ = 0;
        this.options = {
            point_info_on_hover_delay: 500,
            mouse_popup_offset: {left: 10, top: 20},
            canvas_size: {width: 500, height: 500},
            min_viewport_size: 1.0
        };

        // this.mouse_popup = new AMW_MousePopup();
    }

    new_id() {
        return "" + (++this.last_id_);
    }
}

if (window.amw201805 === undefined)
    window.amw201805 = new AMW201805();

// ----------------------------------------------------------------------

const AntigenicMapWidget_content_html = "\
<table>\
  <tr>\
    <td class='a-title'>\
      <span class='a-left'></span>\
      <span class='a-middle'></span>\
      <span class='a-right'>\
        <span class='a-right-left'></span>\
        <span class='a-burger'>&#x2630;</span>\
      </span>\
    </td>\
  </tr>\
  <tr>\
    <td>\
      <canvas></canvas>\
    </td>\
  </tr>\
</table>\
";

// ----------------------------------------------------------------------

export class AntigenicMapWidget
{
    constructor(div, data, options={}) {
        this.div = div;
        this.options = Object.assign({}, window.amw201805.options, options);
        acv_utils.load_css('/js/ad/map-draw/acmacs-map-widget.css');

        // $('head').append( $('<link rel="stylesheet" type="text/css" />').attr('href', '/js/ad/map-draw/acmacs-map-widget.css') );
        // this.div.addClass("amw201804").attr("amw201804_id", window.amw201804.new_id()).append(AntigenicMapWidget_content_html);
        // this.canvas = this.div.find("canvas");

        // this.data = Array.isArray(data) ? data : [data];
        // this.data.unshift({point_scale: sval("point_scale", this.data, 1)});  // for interactive manipulations

        // this.surface = new Surface(this.canvas, {canvas: sval("canvas", data, this.options.canvas_size), viewport: sval("viewport", data, [0, 0, 10, 10])});
        // sval_call("border", data, v => this.surface.border(v));
        // sval_call("title", data, lines => {
        //     this.div.find(".amw201804-title-middle").append(lines[0].text);
        //     //this.div.find(".amw201804-title-left").append("LEFT");
        //     //this.div.find(".amw201804-title-right").append("\u219D");
        //     //this.div.find(".amw201804-title-burger-menu").append("\u2630");
        // });

        // this.events_ = [];
        // this.bind();
        // this.draw();
    }

    destroy() {
        this.detach_all();
    }

}

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
