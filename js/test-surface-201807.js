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
        $("div.main [name='point-scale'] .value").empty().append($("div.main [name='point-scale'] input").val());
        $("div.main [name='point-scale'] input").on("input", evt => {
            this.surface.point_scale(5 * parseFloat(evt.currentTarget.value));
            this.draw();
            $("div.main [name='point-scale'] .value").empty().append(evt.currentTarget.value);
        });
    }

    points() {
        this.surface.point([0, 0], {S: "c", s: 1, F: "red", O: "green", o: 1}, 0, true);
        this.surface.point([0.1, 0.3], {}, 1, true);
        this.surface.point([-1.5, 0.1], {S: "c", s: 1, F: "green", o: 1}, 2, true);
        this.surface.point([2.5, -2.1], {S: "c", s: 2, F: "transparent", O: "black", o: 3, transparent_as_chess: true}, 9, true);
        this.surface.point([1.5, -0.1], {S: "c", s: 1, F: "transparent", O: "black", o: 1}, 3, true);
        this.surface.point([-1.2, 0.3], {S: "b", s: 1, F: "green", o: 1}, 4, true);
        this.surface.point([1.2, -0.3], {S: "b", s: 1, F: "transparent", O: "black", o: 1}, 5, true);
        this.surface.point([-0.8, 1.3], {S: "t", s: 1, F: "magenta", o: 3}, 6, true);
        this.surface.point([0.8, -1.3], {S: "t", s: 1, F: "transparent", O: "black", o: 1}, 7, true);
        this.surface.point([-2.5, 2.1], {S: "c", s: 2, F: "green", o: 1, a: 0.7, r: 0.5}, 8, true);
    }
}

// ----------------------------------------------------------------------

$(document).ready(main);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
