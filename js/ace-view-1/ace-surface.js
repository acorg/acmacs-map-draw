const COS_PI_6 = Math.cos(Math.PI / 6);

// ----------------------------------------------------------------------

export class Transformation
{
    constructor(transformation) {
        if (Array.isArray(transformation) && transformation.length !== 4)
            this.transformation = transformation;
        else
            this.transformation = [1, 0, 0, 1];
    }

    transform_coord(coord) {
        return coord.length ? [coord[0] * this.transformation[0] + coord[1] * this.transformation[2], coord[0] * this.transformation[1] + coord[1] * this.transformation[3]] : [];
    }

    transform_layout(layout) {
        return layout.map(coord => this.transform_coord(coord));
    }
}

// ----------------------------------------------------------------------

export class Surface
{
    constructor(canvas, args={}) {
        this.canvas = canvas;
        this.canvas.prop(args.canvas || {width: 300, height: 300});
        this.context = this.canvas[0].getContext('2d');
        if (args.viewport)
            this.set_viewport(args.viewport);
    }

    set_viewport(viewport) {
        if (this.viewport) {
            this.context.translate(this.viewport[0], this.viewport[1]);
            this.context.scale(this.scale_inv, this.scale_inv);
        }
        if (viewport)
            this.viewport = viewport;
        else
            this.viewport = [0, 0, this.canvas.prop("width"), this.canvas.prop("height")];
        this.scale = this.canvas.prop("width") / viewport[2];
        this.scale_inv = 1 / this.scale;
        this.context.scale(this.scale, this.scale);
        this.context.translate(- this.viewport[0], - this.viewport[1]);
    }

    translate_pixel_offset(offset) {
        return {left: offset.left * this.scale_inv + this.viewport[0], top: offset.top * this.scale_inv + this.viewport[1]};
    }

    // {drawing_order, layout, transformation, style_index, styles, point_scale}
    points(args) {
        if (!Array.isArray(args.drawing_order))
            args.drawing_order = Array.apply(null, {length: args.layout.length}).map(Number.call, Number);
        //console.log("points", args);

        const scale_inv2 = this.scale_inv * this.scale_inv;
        // const transform_coord = coord => coord.length ? [coord[0] * args.transformation[0] + coord[1] * args.transformation[2], coord[0] * args.transformation[1] + coord[1] * args.transformation[3]] : [];
        this.layout_size_shape_ = args.transformation.transform_layout(args.layout); // for circles size**2, for boxes size, shape: 0 - circe, 1 - box, 2 - triangle
        args.drawing_order.forEach(point_no => {
            const coord = this.layout_size_shape_[point_no];
            if (coord.length > 1) {
                const style = args.styles[args.style_index[point_no]];
                const size = (style.s === undefined ? 5 : style.s) * args.point_scale;
                const shape_args = [coord,
                              size,
                              style.F || "transparent",            // fill
                              style.O || "black",                  // ouline
                              style.o === undefined ? 1 : style.o, // outline_width
                              style.r,                             // rotation
                              style.a === undefined ? 1 : style.a  // aspect
                             ];
                switch (style.S) {
                case "C":
                    this.circle_pixels.apply(this, shape_args);
                    coord.push(size * size * 0.25 * scale_inv2, 0);
                    break;
                case "B":
                    this.box_pixels.apply(this, shape_args);
                    coord.push(size * 0.5 * this.scale_inv, 1);
                    break;
                case "T":
                    this.triangle_pixels.apply(this, shape_args);
                    coord.push(size * 0.5 * this.scale_inv, 2);
                    break;
                }
            }
        });
    }

    find_points_at_pixel_offset(offset) {
        const scaled_offset = this.translate_pixel_offset(offset);
        let result = [];
        this.layout_size_shape_.forEach((point_coord_size, point_no) => {
            switch (point_coord_size[3]) {
            case 0:             // circle
                if (((scaled_offset.left - point_coord_size[0])**2 + (scaled_offset.top - point_coord_size[1])**2) <= point_coord_size[2])
                    result.push(point_no);
                break;
            case 1:             // box
                if (Math.abs(scaled_offset.left - point_coord_size[0]) <= point_coord_size[2] && Math.abs(scaled_offset.top - point_coord_size[1]) <= point_coord_size[2])
                    result.push(point_no);
                break;
            case 2:             // triangle
                if (Math.abs(scaled_offset.left - point_coord_size[0]) <= point_coord_size[2] && Math.abs(scaled_offset.top - point_coord_size[1]) <= point_coord_size[2])
                    result.push(point_no);
                break;
            }
        });
        return result;
    }

    circle_scaled(center, size, fill, outline, outline_width, rotation, aspect) {
        this.context.save();
        try {
            this.context.lineWidth = outline_width;
            this.context.fillStyle = fill;
            this.context.strokeStyle = outline;
            this.context.translate(center[0], center[1]);
            if (rotation)
                this.context.rotate(rotation);
            if (aspect && aspect !== 1)
                this.context.scale(aspect, 1);
            this.context.beginPath();
            this.context.arc(0, 0, size / 2, 0, 2*Math.PI);
            this.context.fill();
            this.context.stroke();
        }
        catch(err) {
            console.error('Surface::circle_scaled', err);
        }
        this.context.restore();
    }

    circle_pixels(center, size, fill, outline, outline_width, rotation, aspect) {
        this.circle_scaled(center, size * this.scale_inv, fill, outline, outline_width * this.scale_inv, rotation, aspect);
    }

    box_scaled(center, size, fill, outline, outline_width, rotation, aspect) {
        this.context.save();
        try {
            this.context.lineWidth = outline_width;
            this.context.fillStyle = fill;
            this.context.strokeStyle = outline;
            this.context.translate(center[0], center[1]);
            if (rotation)
                this.context.rotate(rotation);
            this.context.fillRect(-size * aspect / 2, -size / 2, size * aspect, size);
            this.context.strokeRect(-size * aspect / 2, -size / 2, size * aspect, size);
        }
        catch(err) {
            console.error('Surface::box_scaled', err);
        }
        this.context.restore();
    }

    box_pixels(center, size, fill, outline, outline_width, rotation, aspect) {
        this.box_scaled(center, size * this.scale_inv, fill, outline, outline_width * this.scale_inv, rotation, aspect);
    }

    triangle_scaled(center, size, fill, outline, outline_width, rotation, aspect) {
        this.context.save();
        try {
            this.context.lineWidth = outline_width;
            this.context.fillStyle = fill;
            this.context.strokeStyle = outline;
            this.context.translate(center[0], center[1]);
            if (rotation)
                this.context.rotate(rotation);
            this.context.beginPath();
            const radius = size / 2;
            this.context.moveTo(0, -radius);
            this.context.lineTo(-radius * COS_PI_6 * aspect, radius / 2);
            this.context.lineTo(radius * COS_PI_6 * aspect, radius / 2);
            this.context.closePath();
            this.context.fill();
            this.context.stroke();
        }
        catch(err) {
            console.error('Surface::triangle_scaled', err);
        }
        this.context.restore();
    }

    triangle_pixels(center, size, fill, outline, outline_width, rotation, aspect) {
        this.triangle_scaled(center, size * this.scale_inv, fill, outline, outline_width * this.scale_inv, rotation, aspect);
    }

    grid(args={line_color: "#cccccc", line_width: 1, step: 1}) {
        this.context.beginPath();
        for (var x = this.viewport[0] + args.step; x < this.viewport[0] + this.viewport[2]; x += args.step) {
            this.context.moveTo(x, this.viewport[1]);
            this.context.lineTo(x, this.viewport[1] + this.viewport[3]);
        }
        for (var y = this.viewport[1] + args.step; y < this.viewport[1] + this.viewport[3]; y += args.step) {
            this.context.moveTo(this.viewport[0], y);
            this.context.lineTo(this.viewport[0] + this.viewport[2], y);
        }
        this.context.strokeStyle = args.line_color;
        this.context.lineWidth = args.line_width / this.scale;
        this.context.stroke();
    }

    background(args={color: "white"}) {
        this.context.fillStyle = args.color;
        this.context.fillRect.apply(this.context, this.viewport);
    }

    border(args={line_color: "black", line_width: 1}) {
        this.canvas.css("border", "" + args.line_width + "px solid " + args.line_color);
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End: