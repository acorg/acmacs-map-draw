import * as av_toolkit from "./toolkit.js";
import * as av_utils from "./utils.js";

// 3D links
// https://stackoverflow.com/questions/35518381/how-to-shade-the-circle-in-canvas
// http://www.bitstorm.it/blog/en/2011/05/3d-sphere-html5-canvas/
//   see source http://www.bitstorm.it/html5/sphere.html (drawPointWithGradient)

// ----------------------------------------------------------------------

export class Transformation
{
    constructor(transformation) {
        if (Array.isArray(transformation) && transformation.length === 4)
            this.transformation_ = transformation;
        else if (transformation instanceof Transformation)
            this.transformation_ = transformation.transformation_;
        else
            this.transformation_ = [1, 0, 0, 1];
    }

    transform(coord) {
        return coord.length ? [coord[0] * this.transformation[0] + coord[1] * this.transformation[2], coord[0] * this.transformation[1] + coord[1] * this.transformation[3]] : [];
    }
}

// ----------------------------------------------------------------------

export class Viewport
{
    constructor(vp) {
        if (Array.isArray(vp)) {
            if (vp.length === 3)
                this.viewport_ = [vp[0], vp[1], vp[2], vp[2]];
            else if (vp.length === 4)
                this.viewport_ = vp.slice(0);
        }
        else if (vp instanceof Viewport)
            this.viewport_ = vp.viewport_;
        if (!this.viewport_)
            this.viewport_ = [-5, -5, 10, 10];
    }

    move_relative(x, y) {
        if (Array.isArray(x)) {
            this.viewport_[0] += x[0];
            this.viewport_[1] += x[1];
        }
        else {
            this.viewport_[0] += x;
            this.viewport_[1] += y;
        }
    }

    rect() {
        return this.viewport_;
    }

    left() {
        return this.viewport_[0];
    }

    right() {
        return this.viewport_[0] + this.viewport_[2];
    }

    top() {
        return this.viewport_[1];
    }

    bottom() {
        return this.viewport_[1] + this.viewport_[3];
    }

    width() {
        return this.viewport_[2];
    }
}

// ----------------------------------------------------------------------

export class Surface
{
    constructor(canvas) {
        this.canvas_ = canvas;
        this.context_ = this.canvas_[0].getContext('2d', {alpha: false});
        this.viewport(new Viewport());
        this.reset();
    }

    reset() {
        this.context_.fillStyle = "#F8F8F8";
        this.context_.fillRect(0, 0, this.canvas_.width(), this.canvas_.height()); // clear canvas, otherwise chrome makes it black
    }

    width() {
        return this.canvas_.width();
    }

    viewport(new_viewport) {
        if (new_viewport) {
            if (this.viewport_) {
                this.context.translate(this.viewport_.left(), this.viewport_.top());
                this.context.scale(this.scale_inv_, this.scale_inv_);
            }
            this.viewport_ = new Viewport(new_viewport);
            this.scale_ = this.width() / this.viewport_.width();
            this.scale_inv_ = 1 / this.scale_;
            this.context_.scale(this.scale_, this.scale_);
            this.context_.translate(- this.viewport_.left(), - this.viewport_.top());
        }
        return this.viewport_;
    }

    grid(args={line_color: "#CCCCCC", line_width: 1, step: 1}) {
        this.context_.beginPath();
        for (var x = this.viewport_.left() + args.step; x < this.viewport_.right(); x += args.step) {
            this.context_.moveTo(x, this.viewport_.top());
            this.context_.lineTo(x, this.viewport_.bottom());
        }
        for (var y = this.viewport_.top() + args.step; y < this.viewport_.bottom(); y += args.step) {
            this.context_.moveTo(this.viewport_.left(),  y);
            this.context_.lineTo(this.viewport_.right(), y);
        }
        this.context_.strokeStyle = args.line_color;
        this.context_.lineWidth = args.line_width / this.scale_;
        this.context_.stroke();
    }

    background(args={color: "white"}) {
        this.context_.fillStyle = args.color;
        this.context_.fillRect.apply(this.context_, this.viewport_.rect());
    }

    border(args={line_color: "#A0A0A0", line_width: 1}) {
        this.canvas_.css("border", "" + args.line_width + "px solid " + args.line_color);
    }
}


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
