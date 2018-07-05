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

    rotate(angle) {
        const cos = Math.cos(angle);
        const sin = Math.sin(angle);
        const r0 = cos * this.transformation_[0] + -sin * this.transformation_[2];
        const r1 = cos * this.transformation_[1] + -sin * this.transformation_[3];
        this.transformation_[2] = sin * this.transformation_[0] +  cos * this.transformation_[2];
        this.transformation_[3] = sin * this.transformation_[1] +  cos * this.transformation_[3];
        this.transformation_[0] = r0;
        this.transformation_[1] = r1;
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
            this.viewport_ = vp.viewport_.slice(0);
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
        return this;
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

    rotate(angle) {
        this.transformation_.rotate(angle);
    }

    add_resizer(callback) {
        if (this.canvas_.parent().css("position") === "static")
            this.canvas_.parent().css("position", "relative");
        $("<div class='av-window-resizer'></div>").appendTo(this.canvas_.parent())
            .on("mousedown", evt => av_toolkit.drag(evt, pos_diff => callback(this.width() + pos_diff.left)));
    }

    resize(new_size) {
        const viewport = this.viewport_;
        delete this.viewport_;
        if (new_size)
            this.canvas_.prop({width: new_size, height: new_size});
        this.viewport(viewport);
    }

    move_relative(delta) {
        this.viewport(new Viewport(this.viewport_).move_relative(delta));
    }

    // {S: shape, F: fill, O: outline, s: size, r: rotation, a: aspect, o: outline_width}
    point(coord, args, point_no=0, hit_map=true) {
        const transformed = this.transformation_.transform(coord);
        this.context_.save();
        try {
            this.context_.translate.apply(this.context_, transformed);
            const size = (args.s === undefined ? 1 : args.s) * this.point_scale_ * this.scale_inv_;
            draw_point(this.context_, Object.assign({radius: size / 2, scale_inv: this.scale_inv_}, args), false);
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
        let hovered;
        switch (shape) {
        case "b":
        case "r":
            size = size * 0.5 * this.scale_inv_;
            hovered = hit_map_box_hovered;
            break;
        case "t":
            hovered = hit_map_triangle_hovered;
            break;
        case "c":
        default:
            hovered = hit_map_circle_hovered;
            size = size * size * 0.25 * this.scale_inv_2_;
            break;
        }
        this.hit_map_[point_no] = {c: coord, s: size, h: hovered};
    }

    find_points_at_pixel_offset(offset, drawing_order) {
        const scaled_offset = this._translate_pixel_offset(offset);
        const result = drawing_order.filter(point_no => {
            const point_data = this.hit_map_[point_no];
            return point_data.h(scaled_offset, point_data.c, point_data.s);
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

function draw_circle(context, radius)
{
    context.arc(0, 0, radius, 0, 2*Math.PI);
}

function draw_box(context, radius)
{
    context.moveTo(- radius, - radius);
    context.lineTo(  radius, - radius);
    context.lineTo(  radius,   radius);
    context.lineTo(- radius,   radius);
    context.closePath();
}

function draw_triangle(context, radius)
{
    const side = radius * av_utils.sqrt_3;
    context.moveTo(0, -radius);
    context.lineTo(- side / 2, radius / 2);
    context.lineTo(  side / 2, radius / 2);
    context.closePath();
}

function draw_unknown(context, radius)
{
    context.arc(0, 0, radius, 0.5, Math.PI);
    context.lineTo(- radius, - radius);
    context.closePath();
}

function draw_shape(context, shape, radius)
{
    context.beginPath();
    switch (shape[0].toLowerCase()) {
    case "c":
        draw_circle(context, radius);
        break;
    case "b":
    case "r":
        draw_box(context, radius);
        break;
    case "t":
        draw_triangle(context, radius);
        break;
    case "u":
    default:
        draw_unknown(context, radius);
        break;
    }
}

// {S: shape, F: fill, O: outline, radius: , r: rotation, a: aspect, o: outline_width, scale_inv:, style_modifier: true}
export function draw_point(context, args, preserve_context=true)
{
    if (preserve_context)
        context.save();
    try {
        if (args.r)
            context.rotate(args.r);
        if (args.a && args.a > 0 && args.a !== 1)
            context.scale(args.a, 1);
        draw_shape(context, args.S || (args.style_modifier ? "u" : "c"), args.radius);
        const outline = args.style_modifier ? (!args.O || args.O === "unknown" ? null : args.O) : (args.O || "black");
        if (!outline) {
            context.setLineDash([args.scale_inv * 3, args.scale_inv * 6]);
            context.strokeStyle = "pink";
        }
        else
            context.strokeStyle = outline;
        const outline_width = args.style_modifier ? args.o || null : args.o || 1;
        if (outline_width === null) {
            context.lineWidth = args.scale_inv;
            context.setLineDash([args.scale_inv * 5, args.scale_inv * 5]);
        }
        else
            context.lineWidth = (outline_width < 1e-5 ? 1e-5 : outline_width) * args.scale_inv;
        switch (args.F) {
        case null:
        case undefined:
        case "unknown":
            if (args.style_modifier)
                _fill_chess(context, "#A0A0FF", "#E0E0E0", args.radius);
            else
                context.stroke();
            break;
        case "transparent":
            if (args.style_modifier)
                _fill_chess(context, "#F0F0F0", "#E0E0E0", args.radius);
            else
                context.stroke();
            break;
        default:
            context.fillStyle = args.F;
            context.fill();
            context.stroke();
            break;
        }
    }
    catch (err) {
        console.error("av_toolkit::draw_point", e);
    }
    if (preserve_context)
        context.restore();
}

function _fill_chess(context, color1, color2, radius)
{
    context.stroke();
    context.save();
    context.clip();
    context.fillStyle = color1;
    context.fillRect(-radius, -radius, radius * 2, radius * 2);
    const step = 0.1;
    context.strokeStyle = color2;
    context.lineWidth = step;
    context.setLineDash([step, step]);
    context.beginPath();
    for (let y = -radius, z = 0; y < radius; y += step, z = z == 0 ? step : 0) {
        context.moveTo(-radius + z, y);
        context.lineTo(radius, y);
    }
    context.stroke();
    context.restore();
}

// ----------------------------------------------------------------------

function hit_map_circle_hovered(scaled_offset, coord, size) {
    return ((scaled_offset.left - coord[0])**2 + (scaled_offset.top - coord[1])**2) <= size;
}

function hit_map_box_hovered(scaled_offset, coord, size) {
    return Math.abs(scaled_offset.left - coord[0]) <= size && Math.abs(scaled_offset.top - coord[1]) <= size;
}

function hit_map_triangle_hovered(scaled_offset, coord, size) {
    return Math.abs(scaled_offset.left - coord[0]) <= size && Math.abs(scaled_offset.top - coord[1]) <= size;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
