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
    }

    bind() {
    }
}

// ----------------------------------------------------------------------

$(document).ready(main);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
