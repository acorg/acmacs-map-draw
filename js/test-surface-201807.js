import * as mod from "/js/ad/map-draw/ace-view/201807/surface.js";

function main()
{
    new TestSurface();
}

class TestSurface
{
    constructor() {
        this.surface = new mod.Surface($("div.main canvas"));
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
        this._slider_values();
        $("div.main [name='point-scale'] input").on("input", evt => {
            this.surface.point_scale(5 * parseFloat(evt.currentTarget.value));
            this.draw();
            this._slider_values();
        });

        $("div.main [name='zoom0'] input").on("input", evt => {
            this.surface.zoom([0, 0], parseFloat(evt.currentTarget.value));
            this.draw();
            this._slider_values();
        });

        $("div.main [name='zoom2'] input").on("input", evt => {
            this.surface.zoom([2, 3], parseFloat(evt.currentTarget.value));
            this.draw();
            this._slider_values();
        });

        $("div.main [name='rotate'] input").on("input", evt => {
            // this.surface.rotate([0, 0], parseFloat(evt.currentTarget.value));
            this.draw();
            this._slider_values();
        });

        $("div.main [name='pan'] input").on("input", evt => {
            // this.surface.move_to([parseFloat(evt.currentTarget.value), 0]);
            this.draw();
            this._slider_values();
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
    }

    _slider_values() {
        for (let name of ["point-scale", "zoom0", "zoom2", "rotate", "pan"])
            $(`div.main [name="${name}"] .value`).empty().append($(`div.main [name="${name}"] input`).val());
    }
}

// ----------------------------------------------------------------------

$(document).ready(main);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
