import {sval, sval_call} from "../draw/utils.js";
import {Surface} from "../draw/acmacs-draw-surface.js";

// ----------------------------------------------------------------------

export class AntigenicMapWidget {
    constructor(div, data, options={}) {
        $('head').append( $('<link rel="stylesheet" type="text/css" />').attr('href', '/js/ad/map-draw/acmacs-map-widget.css') );
        $(div).addClass("amw").append("<table><tr>\
<td class='amw-title'><span class='amw-title-left'></span><span class='amw-title-middle'></span><span class='amw-title-right'><span class='amw-title-burger-menu'></span></td></tr>\
<tr><td><canvas></canvas></td></tr></table>");
        this.canvas = $(div).find("canvas");

        this.data = Array.isArray(data) ? data : [data];
        this.data.unshift({point_scale: sval("point_scale", this.data, 1)});  // for interactive manipulations

        this.surface = new Surface(this.canvas, {canvas: sval("canvas", data, {width: 500, height: 500}), viewport: sval("viewport", data, [0, 0, 10, 10])});
        sval_call("border", data, v => this.surface.border(v));
        sval_call("title", data, lines => {
            $(div).find(".amw-title-middle").append(lines[0].text);
            //$(div).find(".amw-title-left").append("LEFT");
            $(div).find(".amw-title-right").append("\u219D");
            $(div).find(".amw-title-burger-menu").append("\u2630");
        });

        this.events_ = [];
        this.bind();
        this.draw();
        console.log(this.events_);
    }

    destroy() {
        this.detach_all();
    }

    bind() {
        this.attach("wheel", this.canvas, event => {
            if (event.shiftKey) {
                // Shift-Wheel -> point_scale
                event.stopPropagation();
                if (event.originalEvent.wheelDelta > 0)
                    this.data[0].point_scale *= 1.1;
                else
                    this.data[0].point_scale /= 1.1;
                this.draw();
            }
        });

        this.mousemove_timeout_id = undefined;
        this.attach("mousemove", this.canvas, event => {
            window.clearTimeout(this.mousemove_timeout_id);
            this.mousemove_timeout_id = window.setTimeout(mouse_event => this.hover(mouse_event), 100, event);
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

    hover(event) {
        console.log("hover " + JSON.stringify([event.offsetX, event.offsetY]));
        // console.log(event);
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
