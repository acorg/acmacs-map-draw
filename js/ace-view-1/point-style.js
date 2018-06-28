// ----------------------------------------------------------------------

export function point_style_modifier(args={}) {
    if (!args.canvas || !args.canvas.is("canvas"))
        throw "point_style_modifier: canvas argument must be canvas element";
    new PointStyleModifierCanvas(args).run();
}

// ----------------------------------------------------------------------

const COS_PI_6 = Math.cos(Math.PI / 6);

class PointStyleModifierCanvas
{
    constructor(args) {
        this.canvas_ = args.canvas;
        this.canvas_.prop({width: this.canvas_.width(), height: this.canvas_.height()});
        this.context_ = this.canvas_[0].getContext('2d', {alpha: false});
        const scale_inv = this.canvas_.prop("width");
        this.scale_ = 1 / scale_inv;
        this.context_.scale(scale_inv, scale_inv);
        this.context_.translate(0.5, 0.5);
    }

    run() {
        this.draw();
    }

    draw() {
        this.context_.save();
        this.context_.fillStyle = this._get("background");
        this.context_.fillRect(-0.5, -0.5, 1, 1);
        this._shape();
        this._outline();
        this._outline_width();
        this.context_.stroke();
        this._fill();

        // aspect
        // rotation

        // this.context_.fill();
        this.context_.restore();
    }

    _shape() {
        const outline_width = parseFloat(this._get("outline_width", "unknown")) || 1;
        let radius = 0.5 - this.scale_ * outline_width;
        this.context_.beginPath();
        switch (this._get("shape", "unknown").toLowerCase()) {
        case "circle":
            this.context_.arc(0, 0, radius, 0, 2*Math.PI);
            break;
        case "box":
        case "rectangle":
            this.context_.moveTo(- radius, - radius);
            this.context_.lineTo(  radius, - radius);
            this.context_.lineTo(  radius,   radius);
            this.context_.lineTo(- radius,   radius);
            this.context_.closePath();
            break;
        case "triangle":
            const aspect = 1;
            radius -= this.scale_ * outline_width;
            this.context_.moveTo(0, -radius);
            this.context_.lineTo(-radius * COS_PI_6 * aspect, radius / 2);
            this.context_.lineTo(radius * COS_PI_6 * aspect, radius / 2);
            this.context_.closePath();
            break;
        case "unknown":
        default:
            this.context_.arc(0, 0, radius, 0.5, Math.PI);
            this.context_.lineTo(- radius, - radius);
            this.context_.closePath();
            break;
        }
    }

    _outline() {
        const outline = this._get("outline", "unknown");
        if (outline === "unknown") {
            this.context_.setLineDash([this.scale_ * 3, this.scale_ * 6]);
            this.context_.strokeStyle = "pink";
        }
        else
            this.context_.strokeStyle = outline;
    }

    _outline_width() {
        let outline_width = parseFloat(this._get("outline_width", "unknown"));
        if (isNaN(outline_width)) {
            outline_width = 1;
            this.context_.setLineDash([this.scale_ * 5, this.scale_ * 5]);
        }
        else if (outline_width < 1e-5)
            outline_width = 1e-5;
        this.context_.lineWidth = this.scale_ * outline_width * 2;
    }

    _fill() {
        const fill = this._get("fill", "unknown");
        if (fill === "unknown") {
            this._fill_chess("#A0A0FF", "#E0E0E0");
        }
        else if (fill === "transparent") {
            this._fill_chess("#F0F0F0", "#E0E0E0");
        }
        else {
            this.context_.fillStyle = fill;
            this.context_.fill();
        }
    }

    _fill_chess(color1, color2) {
        this.context_.save();
        this.context_.clip();
        this.context_.fillStyle = color1;
        this.context_.fillRect(-0.5, -0.5, 1, 1);
        const step = 0.1;
        this.context_.strokeStyle = color2;
        this.context_.lineWidth = step;
        this.context_.setLineDash([step, step]);
        this.context_.beginPath();
        for (let y = -0.5, z = 0; y < 0.5; y += step, z = z == 0 ? step : 0) {
            this.context_.moveTo(-0.5 + z, y);
            this.context_.lineTo(0.5, y);
        }
        this.context_.stroke();
        this.context_.restore();
    }

    _get(name, dflt) {
        return this.canvas_.attr("acv_" + name) || dflt;
    }

    _set(name, value) {
        return this.canvas_.attr("acv_" + name, value);
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
