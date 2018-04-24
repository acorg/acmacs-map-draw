import {sval, sval_call, sum_offsets} from "../draw/utils.js";
import {Surface} from "../draw/acmacs-draw-surface.js";

// ----------------------------------------------------------------------

class AMW201804 {

    constructor() {
        this.last_id_ = 0;
        this.options = {
            point_info_on_hover_delay: 500,
            mouse_popup_offset: {left: 10, top: 20},
            canvas_size: {width: 500, height: 500}
        };

        this.mouse_popup = new AMW_MousePopup();
    }

    new_id() {
        return "" + ++this.last_id_;
    }
}

// ----------------------------------------------------------------------

class AMW_MousePopup {

    constructor() {
    }

    show(contents, parent, offsets_to_parent) {
        this.hide();
        if ($("#amw201804-mouse-popup").length === 0)
            $("body").append("<div id='amw201804-mouse-popup' class='amw201804-mouse-popup'></div>");
        let popup = $("#amw201804-mouse-popup");
        popup.empty();
        if (typeof(contents) === "function")
            contents(popup);
        else
            popup.append(contents);
        popup.css(sum_offsets(offsets_to_parent.concat(parent.offset()))).show();
    }

    show_ul(text_rows, parent, offsets_to_parent) {
        this.show($("<ul></ul>").append(text_rows.map(text => "<li>" + text + "</li>").join("")), parent, offsets_to_parent);
    }

    hide() {
        $("#amw201804-mouse-popup").hide();
    }
}

// ----------------------------------------------------------------------

if (window.amw201804 === undefined)
    window.amw201804 = new AMW201804();

// ----------------------------------------------------------------------

// const AntigenicMapWidget_options = {
//     point_info_on_hover_delay: 500,
//     mouse_popup_offset: {left: 10, top: 20}
// };

const AntigenicMapWidget_content_html = "\
<table>\
  <tr>\
    <td class='amw201804-title'>\
      <span class='amw201804-title-left'></span>\
      <span class='amw201804-title-middle'></span>\
      <span class='amw201804-title-right'>\
        <span class='amw201804-title-right-left'></span>\
        <span class='amw201804-title-burger-menu'>&#x2630;</span>\
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

export class AntigenicMapWidget {

    constructor(div, data, options={}) {
        this.div = $(div);
        this.options = Object.assign({}, window.amw201804.options, options);

        $('head').append( $('<link rel="stylesheet" type="text/css" />').attr('href', '/js/ad/map-draw/acmacs-map-widget.css') );
        this.div.addClass("amw201804").attr("amw201804_id", window.amw201804.new_id()).append(AntigenicMapWidget_content_html);
        this.canvas = this.div.find("canvas");

        this.data = Array.isArray(data) ? data : [data];
        this.data.unshift({point_scale: sval("point_scale", this.data, 1)});  // for interactive manipulations

        this.surface = new Surface(this.canvas, {canvas: sval("canvas", data, this.options.canvas_size), viewport: sval("viewport", data, [0, 0, 10, 10])});
        sval_call("border", data, v => this.surface.border(v));
        sval_call("title", data, lines => {
            this.div.find(".amw201804-title-middle").append(lines[0].text);
            //this.div.find(".amw201804-title-left").append("LEFT");
            //this.div.find(".amw201804-title-right").append("\u219D");
            //this.div.find(".amw201804-title-burger-menu").append("\u2630");
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
        this.attach("click", this.div.find(".amw201804-title-burger-menu"), event => {
            make_popup_menu(event, this);
        });
        this.set_point_info_on_hover();
    }

    point_info_on_hover(offset) {
        const start = new Date();
        const points = this.surface.find_points_at_pixel_offset(offset);
        // console.log("hover " + JSON.stringify(points));
        if (points.length) {
            const labels = sval("labels", this.data, []);
            const names = points.map((point_no) => labels[point_no]);
            window.amw201804.mouse_popup.show_ul(names, this.canvas, [offset, this.options.mouse_popup_offset]);
        }
        else {
            window.amw201804.mouse_popup.hide();
            // this.hide_point_info();
        }
    }

    point_info_on_mouse_hover(mouse_event) {
        const border_width = parseFloat(this.canvas.css("border-width"));
        const offset_x = border_width + parseFloat(this.canvas.css("padding-left"));
        const offset_y = border_width + parseFloat(this.canvas.css("padding-top"));
        this.point_info_on_hover({left: mouse_event.offsetX - offset_x, top: mouse_event.offsetY - offset_y});
    }

    set_point_info_on_hover() {
        this.mousemove_timeout_id = undefined;
        this.attach("mousemove", this.canvas, event => {
            window.clearTimeout(this.mousemove_timeout_id);
            this.mousemove_timeout_id = window.setTimeout(me => this.point_info_on_mouse_hover(me), this.options.point_info_on_hover_delay, event);
        });
        this.attach("mouseleave", this.canvas, event => window.clearTimeout(this.mousemove_timeout_id));
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
<div id='amw201804-popup-menu' class='amw201804-popup-menu'>\
  <ul>\
    <li>Not</li>\
    <li>Implemented</li>\
    <li>Yet</li>\
  </ul>\
</div>\
";

const PopupMenu_click_background_html = "\
<div id='amw201804-popup-menu-click-background' class='amw201804-popup-menu-click-background'>\
</div>\
";

function make_popup_menu(event, widget) {
    if (window.amw201804_popup_menu) {
        window.amw201804_popup_menu.destroy();
        window.amw201804_popup_menu = null;
    }
    window.amw201804_popup_menu = new PopupMenu();
    window.amw201804_popup_menu.move_to_element($(event.target));
    window.amw201804_popup_menu.show();
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
        $("body").remove("#amw201804-popup-menu");
        $("body").remove("#amw201804-popup-menu-click-background");
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
