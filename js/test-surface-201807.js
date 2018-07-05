import * as av_surface from "/js/ad/map-draw/ace-view/201807/surface.js";

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
    }

    bind() {
        this.surface.add_resizer(width_diff => {
            this.surface.resize(width_diff);
            this.draw();
            this._slider_values();
        });

        let mousemove_timeout_id = undefined;
        this.surface.canvas_.on("mousemove", evt => {
            window.clearTimeout(mousemove_timeout_id);
            mousemove_timeout_id = window.setTimeout(me => {
                const points = this.surface.find_points_at_pixel_offset(this.surface.mouse_offset(evt), this.drawing_order_);
                console.log("hovered", points);
            }, 500, evt);
        });
        this.surface.canvas_.on("mouseleave", evt => window.clearTimeout(mousemove_timeout_id));

        this._slider_values();
        $("div.main [name='point-scale'] input").on("input", evt => {
            this.surface.point_scale(5 * parseFloat(evt.currentTarget.value));
            this.draw();
            this._slider_values();
        });

        let current_zoom = 10;
        $("div.main [name='zoom0'] input").on("input", evt => {
            const new_zoom = parseFloat(evt.currentTarget.value);
            this.surface.zoom([0, 0], current_zoom / new_zoom);
            this.draw();
            this._slider_values();
            current_zoom = new_zoom;
        });

        $("div.main [name='zoom2'] input").on("input", evt => {
            const new_zoom = parseFloat(evt.currentTarget.value);
            this.surface.zoom([2, 3], current_zoom / new_zoom);
            this.draw();
            this._slider_values();
            current_zoom = new_zoom;
        });

        let current_rotation = 0;
        $("div.main [name='rotate'] input").on("input", evt => {
            const angle = parseFloat(evt.currentTarget.value) - current_rotation;
            this.surface.rotate(angle);
            this.draw();
            this._slider_values();
            current_rotation = parseFloat(evt.currentTarget.value);
        });

        let current_x = 0;
        $("div.main [name='pan'] input").on("input", evt => {
            this.surface.move_relative([current_x - parseFloat(evt.currentTarget.value), 0]);
            this.draw();
            this._slider_values();
            current_x = parseFloat(evt.currentTarget.value);
        });
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

    _slider_values() {
        for (let name of ["point-scale", "zoom0", "zoom2", "rotate", "pan", "size"])
            $(`div.main [name="${name}"] .value`).empty().append($(`div.main [name="${name}"] input`).val());
    }
}

// ----------------------------------------------------------------------

$(document).ready(main);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
