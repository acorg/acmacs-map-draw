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
        return coord.length ? [coord[0] * this.transformation_[0] + coord[1] * this.transformation_[2], coord[0] * this.transformation_[1] + coord[1] * this.transformation_[3]] : [];
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
        this.transformation_ = new Transformation();
        this.viewport(new Viewport());
        this.point_scale_ = 5;
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
                this.context_.translate(this.viewport_.left(), this.viewport_.top());
                this.context_.scale(this.scale_inv_, this.scale_inv_);
            }
            this.viewport_ = new Viewport(new_viewport);
            this.scale_ = this.width() / this.viewport_.width();
            this.scale_inv_ = 1 / this.scale_;
            this.scale_inv_2_ = this.scale_inv_ * this.scale_inv_;
            this.context_.scale(this.scale_, this.scale_);
            this.context_.translate(- this.viewport_.left(), - this.viewport_.top());
        }
        return this.viewport_;
    }

    point_scale(new_scale) {
        if (new_scale !== undefined)
            this.point_scale_ = new_scale;
        return this.point_scale_;
    }

    zoom_with_mouse(evt) {
        const scroll = evt.originalEvent.deltaX != 0 ? evt.originalEvent.deltaX : evt.originalEvent.deltaY; // depends if mouse or touchpad used
        this.zoom(this._translate_pixel_offset(this._mouse_offset(evt)), scroll > 0 ? 1.05 : (1 / 1.05));
    }

    zoom(center, change) {
        try {
            const new_size = Math.max(this.viewport_.width() * change, 1);
            if (center.left)
                center = [center.left, center.top];
            const left = (center[0] - this.viewport_.left()) / this.viewport_.width();
            const top = (center[1] - this.viewport_.top()) / this.viewport_.width();
            this.viewport([center[0] - new_size * left, center[1] - new_size * top, new_size]);
        }
        catch (err) {
            console.error("av_surface::zoom", center, change, err);
        }
    }

    // {S: shape, F: fill, O: outline, s: size, r: rotation, a: aspect, o: outline_width}
    point(coord, args, point_no=0, hit_map=true) {
        const transformed = this.transformation_.transform(coord);
        this.context_.save();
        try {
            this.context_.translate.apply(this.context_, transformed);
            const size = (args.s === undefined ? 1 : args.s) * this.point_scale_ * this.scale_inv_;
            av_toolkit.draw_point(this.context_, Object.assign({radius: size / 2, scale_inv: this.scale_inv_}, args), false);
            if (hit_map)
                this._add_to_hit_map(point_no, coord, size, args.S || "c");
        }
        catch (err) {
            console.error("av_surface::point", err);
        }
        this.context_.restore();
    }

    _add_to_hit_map(point_no, coord, size, shape) {
        if (!this.hit_map_)
            this.hit_map_ = [];
        shape = shape[0].toLowerCase();
        switch (shape) {
        case "b":
        case "r":
        case "t":
            size = size * 0.5 * this.scale_inv_;
            break;
        case "c":
        default:
            size = size * size * 0.25 * this.scale_inv_2_;
            shape = "c";
            break;
        }
        this.hit_map_[point_no] = {c: coord, s: size, S: shape};
    }

    find_points_at_pixel_offset(offset, drawing_order) {
        let result = [];
        const scaled_offset = this._translate_pixel_offset(offset);
        drawing_order.forEach(point_no => {
            const point_data = this.hit_map_[point_no];
            switch (point_data.S) {
            case "c":
                if (((scaled_offset.left - point_data.c[0])**2 + (scaled_offset.top - point_data.c[1])**2) <= point_data.s)
                    result.push(point_no);
                break;
            case "b":
            case "r":
                if (Math.abs(scaled_offset.left - point_data.c[0]) <= point_data.s && Math.abs(scaled_offset.top - point_data.c[1]) <= point_data.s)
                    result.push(point_no);
                break;
            case "t":
                if (Math.abs(scaled_offset.left - point_data.c[0]) <= point_data.s && Math.abs(scaled_offset.top - point_data.c[1]) <= point_data.s)
                    result.push(point_no);
                break;
            }
        });
        result.reverse();
        return result;
    }

    _translate_pixel_offset(offset) {
        return {left: offset.left * this.scale_inv_ + this.viewport_.left(), top: offset.top * this.scale_inv_ + this.viewport.top()};
    }

    _mouse_offset(mouse_event) {
        const border_width = parseFloat(this.canvas_.css("border-width"));
        const offset_x = border_width + parseFloat(this.canvas_.css("padding-left"));
        const offset_y = border_width + parseFloat(this.canvas_.css("padding-top"));
        return {left: mouse_event.offsetX - offset_x, top: mouse_event.offsetY - offset_y};
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
