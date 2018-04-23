import {sval, sval_call} from "../draw/utils.js";
import {Surface} from "../draw/acmacs-draw-surface.js";

// ----------------------------------------------------------------------

const AntigenicMapWidget_options = {
    point_info_on_hover_delay: 500,
    mouse_popup_offset: {left: 10, top: 20}
};

const AntigenicMapWidget_content_html = "\
<table>\
  <tr>\
    <td class='amw-title'>\
      <span class='amw-title-left'></span>\
      <span class='amw-title-middle'></span>\
      <span class='amw-title-right'>\
        <span class='amw-title-right-left'></span>\
        <span class='amw-title-burger-menu'>&#x2630;</span>\
      </span>\
    </td>\
  </tr>\
  <tr>\
    <td>\
      <canvas></canvas>\
    </td>\
  </tr>\
</table>\
<div class='amw-mouse-popup'><ul></ul></div>\
";

// ----------------------------------------------------------------------

export class AntigenicMapWidget {

    constructor(div, data, options={}) {
        this.div = $(div);
        this.options = Object.assign({}, AntigenicMapWidget_options, options);

        $('head').append( $('<link rel="stylesheet" type="text/css" />').attr('href', '/js/ad/map-draw/acmacs-map-widget.css') );
        this.div.addClass("amw").append(AntigenicMapWidget_content_html);
        this.canvas = this.div.find("canvas");

        this.data = Array.isArray(data) ? data : [data];
        this.data.unshift({point_scale: sval("point_scale", this.data, 1)});  // for interactive manipulations

        this.surface = new Surface(this.canvas, {canvas: sval("canvas", data, {width: 500, height: 500}), viewport: sval("viewport", data, [0, 0, 10, 10])});
        sval_call("border", data, v => this.surface.border(v));
        sval_call("title", data, lines => {
            this.div.find(".amw-title-middle").append(lines[0].text);
            //this.div.find(".amw-title-left").append("LEFT");
            //this.div.find(".amw-title-right").append("\u219D");
            //this.div.find(".amw-title-burger-menu").append("\u2630");
        });

        this.events_ = [];
        this.bind();
        this.draw();
        // console.log(this.events_);
    }

    destroy() {
        this.detach_all();
    }

    bind() {
        this.attach("wheel", this.canvas, event => {
            if (event.shiftKey) {
                // Shift-Wheel -> point_scale
                event.stopPropagation();
                event.preventDefault();
                if (event.originalEvent.wheelDelta > 0)
                    this.data[0].point_scale *= 1.1;
                else
                    this.data[0].point_scale /= 1.1;
                this.draw();
            }
        });
        this.attach("click", this.div.find(".amw-title-burger-menu"), event => {
            make_popup_menu(event, this);
        });
        this.set_point_info_on_hover();
    }

    point_info_on_hover(offset) {
        const start = new Date();
        const points = this.surface.find_points_at_pixel_offset(offset);
        //console.log("hover " + JSON.stringify(points));
        if (points.length) {
            const labels = sval("labels", this.data, []);
            const names = points.map((point_no) => labels[point_no]);
            this.show_point_info(names, offset);
            // console.log("hover " + JSON.stringify(names));
            //console.log("search time: " + (new Date() - start) + "ms");
        }
        else {
            this.hide_point_info();
        }
    }

    point_info_on_mouse_hover(mouse_event) {
        const border_width = parseFloat(this.canvas.css("border-width"));
        const offset_x = border_width + parseFloat(this.canvas.css("padding-left"));
        const offset_y = border_width + parseFloat(this.canvas.css("padding-top"));
        this.point_info_on_hover([mouse_event.offsetX - offset_x, mouse_event.offsetY - offset_y]);
    }

    set_point_info_on_hover() {
        this.mousemove_timeout_id = undefined;
        this.attach("mousemove", this.canvas, event => {
            window.clearTimeout(this.mousemove_timeout_id);
            this.mousemove_timeout_id = window.setTimeout(me => this.point_info_on_mouse_hover(me), this.options.point_info_on_hover_delay, event);
        });
        this.attach("mouseleave", this.canvas, event => window.clearTimeout(this.mousemove_timeout_id));
    }

    show_point_info(text_rows, offset) {
        const w = this.div.find(".amw-mouse-popup");
        const ul = w.find("ul");
        ul.empty();
        text_rows.forEach(text => ul.append("<li>" + text + "</li>"));
        const canvas_pos = this.canvas.parent().position();
        w.css({left: offset[0] + canvas_pos.left + this.options.mouse_popup_offset.left, top: offset[1] + canvas_pos.top + this.options.mouse_popup_offset.top});
        w.show();
    }

    hide_point_info() {
        this.div.find(".amw-mouse-popup").hide();
    }

    attach(event_name, target, handler) {
        this.detach(event_name, target);
        target.on(event_name, handler);
        this.events_.push({name: event_name, target: target});
    }

    detach(event_name, target) {
        let index = this.events_.findIndex(entry => entry.name === event_name && entry.target === target);
        if (index >= 0) {
            target.off(event_name);
            this.events_.splice(index, 1);
        }
    }

    detach_all() {
        for (let entry of this.events_)
            entry.target.off(entry.name);
        this.events_ = [];
    }

    draw() {
        sval_call("background", this.data, v => this.surface.background(v));
        sval_call("grid", this.data, v => this.surface.grid(v));
        this.surface.points(sval("drawing_order", this.data), sval("layout", this.data), sval("transformation", this.data), sval("style_index", this.data), sval("styles", this.data), sval("point_scale", this.data, 1));
    }
}

// ----------------------------------------------------------------------

const PopupMenu_content_html = "\
<div id='amw-popup-menu' class='amw-popup-menu'>\
  <ul>\
    <li>Not</li>\
    <li>Implemented</li>\
    <li>Yet</li>\
  </ul>\
</div>\
";

const PopupMenu_click_background_html = "\
<div id='amw-popup-menu-click-background' class='amw-popup-menu-click-background'>\
</div>\
";

function make_popup_menu(event, widget) {
    if (window.amw_popup_menu) {
        window.amw_popup_menu.destroy();
        window.amw_popup_menu = null;
    }
    window.amw_popup_menu = new PopupMenu();
    window.amw_popup_menu.move_to_element($(event.target));
    window.amw_popup_menu.show();
}

class PopupMenu {
    constructor() {
        this.menu = $(PopupMenu_content_html).appendTo($("body"));
        this.background = $(PopupMenu_click_background_html).appendTo($("body"));
        this.background.on("click", () => this.hide());
        this.menu.find("li").on("click", event => { this.clicked(event); });
    }

    clicked(event) {
        console.log("clicked", event);
        this.hide();
    }

    hide() {
        this.menu.hide();
        this.background.hide();
    }

    show() {
        this.menu.show();
        this.background.css({width: $(document).width(), height: $(document).height()});
        this.background.show();
    }

    move_to_element(element) {
        const offset = element.offset();
        this.menu.css({left: offset.left + element.outerWidth(true) - this.menu.outerWidth(true), top: offset.top + element.outerHeight(true)});
    }

    destroy() {
        this.hide();
        this.menu.find("li").off("click");
        this.background.off("click");
        $("body").remove("#amw-popup-menu");
        $("body").remove("#amw-popup-menu-click-background");
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
