import * as av_surface from "/js/ad/map-draw/ace-view/201807/surface.js";
import * as av_utils from "/js/ad/map-draw/ace-view/201807/utils.js";

function main()
{
    new TestSurface();
}

class TestSurface
{
    constructor() {
        this.surface = new av_surface.Surface($("div.main canvas"));
        this.bind();
        this.draw();
    }

    draw() {
        this.surface.reset();
        this.surface.background();
        this.surface.grid();
        this.surface.border();
        this.points();
        this.lines();
    }

    bind() {
        this.surface.add_resizer(width_diff => {
            this.surface.resize(width_diff);
            this.draw();
            // this._slider_values();
        });

        let mousemove_timeout_id = undefined;
        this.surface.canvas_.on("mousemove", evt => {
            window.clearTimeout(mousemove_timeout_id);
            mousemove_timeout_id = window.setTimeout(me => {
                const points = this.surface.find_points_at_pixel_offset(this.surface.mouse_offset(evt), this.drawing_order_);
                $("div.main div[name='point-list']").empty().append(points.map(pn => "" + pn).join());
            }, 500, evt);
        });
        this.surface.canvas_.on("mouseleave", evt => window.clearTimeout(mousemove_timeout_id));

        this.surface.canvas_.on("wheel DOMMouseScroll", evt => av_utils.forward_event(evt, evt => {
            if (evt.shiftKey) { // Shift-Wheel -> point_scale
                this.surface.point_scale_with_mouse(evt);
                this.draw();
            }
            else if (evt.altKey) { // Alt-Wheel -> zoom
                this.surface.zoom_with_mouse(evt);
                this.draw();
            }
            else if (evt.ctrlKey) { // Ctrl-Wheel -> rotate
                this.surface.rotate_with_mouse(evt);
                this.draw();
            }
        }));

        this.surface.canvas_.on("contextmenu", evt => { if (evt.ctrlKey) av_utils.forward_event(evt); }); // block context menu on ctrl-click (but allow on the right-click)
        this.surface.canvas_.on("click", evt => av_utils.forward_event(evt, evt => {
            if (evt.ctrlKey) { // Ctrl-click -> flip
                this.surface.flip_ew(evt);
                this.draw();
            }
        }));

        this.surface.canvas_.on("mousedown", evt => av_utils.forward_event(evt, evt => {
            if (evt.altKey) {   // Alt-Drag - pan
                let mousedown_pos = {left: evt.clientX, top: evt.clientY};
                document.onmouseup = () => { document.onmouseup = document.onmousemove = null; };
                document.onmousemove = evt => {
                    this.surface.move_relative(mousedown_pos.left - evt.clientX, mousedown_pos.top - evt.clientY);
                    mousedown_pos = {left: evt.clientX, top: evt.clientY};
                    this.draw();
                };
            }
        }));


        // this._slider_values();

        // let current_x = 0;
        // $("div.main [name='pan'] input").on("input", evt => {
        //     this.surface.move_relative([current_x - parseFloat(evt.currentTarget.value), 0]);
        //     this.draw();
        //     this._slider_values();
        //     current_x = parseFloat(evt.currentTarget.value);
        // });
    }

    points() {
        this.surface.point([0, 0], {S: "c", s: 1, F: "red", O: "green", o: 1}, 0, true);
        this.surface.point([0.1, 0.3], {}, 1, true);
        this.surface.point([-1.5, 0.1], {S: "c", s: 1, F: "green", o: 1}, 2, true);
        this.surface.point([2.5, -2.1], {S: "c", s: 2, F: "transparent", O: "black", o: 3, style_modifier: true}, 9, true);
        this.surface.point([1.5, -0.1], {S: "c", s: 1, F: "transparent", O: "black", o: 1}, 3, true);
        this.surface.point([-1.2, 0.3], {S: "b", s: 1, F: "green", o: 1}, 4, true);
        this.surface.point([1.2, -0.3], {S: "b", s: 1, F: "transparent", O: "black", o: 1}, 5, true);
        this.surface.point([-0.8, 1.3], {S: "t", s: 1, F: "magenta", o: 3}, 6, true);
        this.surface.point([0.8, -1.3], {S: "t", s: 1, F: "transparent", O: "black", o: 1}, 7, true);
        this.surface.point([-2.5, 2.1], {S: "c", s: 2, F: "green", o: 1, a: 0.7, r: 0.5}, 8, true);
        this.drawing_order_ = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
    }

    lines() {
        this.surface.line({start: [0, 0], end: [4, -3], color: "cyan"});
        this.surface.line({start: [0, 0.5], end: [-4, 3], color: "rgba(255, 0, 255, 0.5)", width: 10});
        this.surface.arrow({start: [0, 0], end: [-4, -3]});
        this.surface.arrow({start: [-4, -2], end: [-2, 4]});
        this.surface.arrow({start: [-2, -4], end: [-4, 2]});
        this.surface.arrow({start: [-2, 4], end: [4, -2], head_width: 20, head_filled: false, head_color: "magenta"});

        this.surface.circle({center: [-2, -3], radius: 1, fill: "rgba(0, 0, 255, 0.5)", aspect: 1.3, rotation: 0.2});
        this.surface.sector({center: [2, 2], start: 0.2, end: 0.2 + Math.PI / 4, radius: 2, fill: "rgba(255, 0, 255, 0.5)", aspect: 1, rotation: 0, width: 4});

        const text1 = {origin: [-4, 4], text: "Abyrvalg", color: "red", size: 16, rotation: 0};
        this.surface.text(text1);
        const text_width = this.surface.text_width({text: text1.text, size: text1.size});
        this.surface.text(Object.assign({}, text1, {origin: [text1.origin[0] + text_width, text1.origin[1]], color: "#008000"}));
    }

    // _slider_values() {
    //     for (let name of ["point-scale", "zoom0", "zoom2", "rotate", "pan", "size"])
    //         $(`div.main [name="${name}"] .value`).empty().append($(`div.main [name="${name}"] input`).val());
    // }
}

// ----------------------------------------------------------------------

$(document).ready(main);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
