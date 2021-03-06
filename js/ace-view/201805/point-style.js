import * as acv_toolkit from "./toolkit.js";
import * as acv_utils from "./utils.js";

acv_utils.load_css("/js/ad/map-draw/ace-view/201805/point-style.css");

// ----------------------------------------------------------------------

// {canvas: <canvas>, onchange: callback}
export function point_style_modifier(args) {
    if (!args.canvas || !args.canvas.is("canvas"))
        throw "point_style_modifier: canvas argument must be canvas element";
    new PointStyleModifier({modifier_canvas: new PointStyleModifierCanvas(args)});
}

// ----------------------------------------------------------------------

var point_style_modifier_dialog_singleton = null;

class PointStyleModifier
{
    // args: {modifier_canvas: PointStyleModifierCanvas }
    constructor(args) {
        this.canvas_ = args.modifier_canvas;
        this.canvas_.draw();
        this.canvas_.on("click", evt => acv_utils.forward_event(evt, () => this.show_dialog()));
    }

    show_dialog() {
        if (!point_style_modifier_dialog_singleton)
            point_style_modifier_dialog_singleton = new PointStyleModifierDialog();
        point_style_modifier_dialog_singleton.show(this.canvas_);
    }
}

// ----------------------------------------------------------------------

const PointStyleModifierDialog_html = "\
<div class='a-point-style-modifier-dialog a-window-shadow'>\
  <table>\
    <tr class='a-title'>\
      <td>Fill</td>\
      <td>Outline</td>\
      <td title='Size'>S</td>\
      <td title='Outline width'>W</td>\
      <td title='Aspect'>A</td>\
      <td title='Rotation'>R</td>\
    </tr>\
    <tr>\
      <td name='fill'></td>\
      <td name='outline'></td>\
      <td><div class='a-point-style-slider-vertical'><input type='range' name='size' value='0' min='-3' max='9' list='point-style-input-tickmarks'></div></td>\
      <td><div class='a-point-style-slider-vertical'><input type='range' name='outline_width' value='0' min='-3' max='19' list='point-style-input-tickmarks'></div></td>\
      <td><div class='a-point-style-slider-vertical'><input type='range' name='aspect' value='1' min='0.1' max='1' step='0.1', list='point-style-input-tickmarks-aspect'></div></td>\
      <td><div class='a-point-style-slider-vertical'><input type='range' name='rotation' value='0' min='-180' max='180' step='15' list='point-style-input-tickmarks-angle'></div></td>\
    </tr>\
    <tr>\
      <td colspan='2' class='a-shape'><div class='a-label'>Shape:</div><div class='a-shape' name='circle' title='circle'></div><div class='a-shape' name='box' title='box'></div><div class='a-shape' name='triangle' title='triangle'></div></td>\
      <td class='a-point-style-slider-value'><span class='a-point-style-slider-value' name='size'></span></td>\
      <td class='a-point-style-slider-value'><span class='a-point-style-slider-value' name='outline_width'></span></td>\
      <td class='a-point-style-slider-value'><span class='a-point-style-slider-value' name='aspect'></span></td>\
      <td class='a-point-style-slider-value'><span class='a-point-style-slider-value' name='rotation'></span></td>\
    </tr>\
  </table>\
  <div class='a-reset-button' title='undo all changes'><div></div></div>\
  <datalist id='point-style-input-tickmarks'></datalist>\
  <datalist id='point-style-input-tickmarks-angle'></datalist>\
  <datalist id='point-style-input-tickmarks-aspect'></datalist>\
</div>\
";

const DegreesToRadians = Math.PI / 180;

class PointStyleModifierDialog
{
    // args: {}
    constructor(args={}) {
        this._make();
    }

    show(modifier_canvas) {
        this.modifier_canvas_ = modifier_canvas;
        modifier_canvas.draw();
        this._setup();
        this.div_.css(modifier_canvas.bottom_left_absolute());
        this.modal_ = new acv_toolkit.Modal({element: this.div_, z_index: 900, dismiss: () => this.hide()});
        this.div_.show();
    }

    hide() {
        this.modifier_canvas_ = null;
        this.div_.hide();
    }

    _make() {
        this.div_ = $(PointStyleModifierDialog_html).appendTo("body").hide().css({position: "absolute"});

        const td_fill = this.div_.find('td[name="fill"]');
        const td_outline = this.div_.find('td[name="outline"]');
        const colors = ["#000000", "white", "transparent", "#ff0000", "#00ff00", "#0000ff", "#ffa500", "#6495ed"].concat(acv_toolkit.sAnaColors);
        colors.forEach(color => {
            if (color === "white") {
                td_fill.append(`<div class="a-fill-color a-white" name="${color}" title="fill ${color}"></div>`);
                td_outline.append(`<div class="a-outline-color a-white" name="${color}" style="background-color: #E0E0E0; border: 3px solid ${color}" title="${color} outline"></div>`);
            }
            else if (color === "transparent") {
                td_fill.append(`<div class="a-fill-color a-transparent" name="${color}" title="fill ${color}"></div>`);
                td_outline.append(`<div class="a-outline-color a-transparent" name="${color}" title="${color} outline"></div>`);
            }
            else {
                td_fill.append(`<div class="a-fill-color" name="${color}" style="background-color: ${color}" title="fill ${color}"></div>`);
                td_outline.append(`<div class="a-outline-color" name="${color}" style="border: 3px solid ${color}" title="outline ${color}"></div>`);
            }
        });
        this.div_.find("div.a-fill-color").on("click", evt => acv_utils.forward_event(evt, () => this._fill(evt.currentTarget.getAttribute("name"))));
        this.div_.find("div.a-outline-color").on("click", evt => acv_utils.forward_event(evt, evt => this._outline(evt.currentTarget.getAttribute("name"))));
        this.div_.find("div.a-shape").on("click", evt => acv_utils.forward_event(evt, evt => this._shape(evt.currentTarget.getAttribute("name"))));

        const tickmarks = this.div_.find("datalist#point-style-input-tickmarks").empty();
        for (let i = -20; i <= 20; i += 2)
            tickmarks.append(`<option value='${i}'>`);
        const tickmarks_angle = this.div_.find("datalist#point-style-input-tickmarks-angle").empty();
        for (let angle = -180; angle <= 180; angle += 30)
            tickmarks_angle.append(`<option value='${angle}'>`);
        const tickmarks_aspect = this.div_.find("datalist#point-style-input-tickmarks-aspect").empty();
        for (let aspect = 0; aspect <= 1; aspect += 0.2)
            tickmarks_aspect.append(`<option value='${aspect}'>`);

        this.div_.find("input[name='size']").on("input", evt => this._size_from_slider(parseFloat(evt.currentTarget.value)));
        this.div_.find("input[name='outline_width']").on("input", evt => this._outline_width_from_slider(parseFloat(evt.currentTarget.value)));
        this.div_.find("input[name='rotation']").on("input", evt => this._rotation_from_slider(parseFloat(evt.currentTarget.value)));
        this.div_.find("input[name='aspect']").on("input", evt => this._aspect_from_slider(parseFloat(evt.currentTarget.value)));

        this.div_.find("div.a-reset-button").on("click", evt => acv_utils.forward_event(evt, () => this._undo()));
    }

    _setup(save=true) {
        if (save) {
            this.saved_data_ = {
                fill: this.modifier_canvas_.get("fill", null),
                outline: this.modifier_canvas_.get("outline", null),
                outline_width: this.modifier_canvas_.get("outline_width", null),
                shape: this.modifier_canvas_.get("shape", null),
                rotation: this.modifier_canvas_.get("rotation", null),
                aspect: this.modifier_canvas_.get("aspect", null)
            };
        }
        this._size_to_slider(parseFloat(this.modifier_canvas_.get("size", 1)));
        this._outline_width_to_slider(parseFloat(this.modifier_canvas_.get("outline_width", 1)));
        this._rotation_to_slider(parseFloat(this.modifier_canvas_.get("rotation", 0)));
        this._aspect_to_slider(parseFloat(this.modifier_canvas_.get("aspect", 1)));
    }

    _fill(color) {
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("fill", color, true);
    }

    _outline(color) {
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("outline", color, true);
    }

    _shape(shape) {
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("shape", shape, true);
    }

    _size_from_slider(value) {
        if (value === -1)
            value = 0.7;
        else if (value === -2)
            value = 0.4;
        else if (value < 0)
            value = 0.1;
        else
            ++value;
        this.div_.find("span[name='size']").empty().append(value);
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("size", value, true);
    }

    _size_to_slider(value) {
        const slider = this.div_.find("input[name='size']");
        if (value >= 1)
            slider.val(value - 1);
        else if (value < 1 && value >= 0.7)
            slider.val(-1);
        else if (value < 0.7 && value >= 0.4)
            slider.val(-2);
        else
            slider.val(-3);
        this.div_.find("span[name='size']").empty().append(value);
    }

    _outline_width_from_slider(value) {
        if (value < 0) {
            value = 10**value;
            if (value < 0.01)
                value = 0;
        }
        else {
            ++value;
        }
        this.div_.find("span[name='outline_width']").empty().append(value);
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("outline_width", value, true);
    }

    _outline_width_to_slider(value) {
        const slider = this.div_.find("input[name='outline_width']");
        if (value >= 1)
            slider.val(value - 1);
        else if (value < 0.01)
            slider.val(-3);
        else
            slider.val(Math.log10(value));
        this.div_.find("span[name='outline_width']").empty().append(value);
    }

    _rotation_from_slider(value) {
        this.div_.find("span[name='rotation']").empty().append(value);
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("rotation", value * DegreesToRadians, true);
    }

    _rotation_to_slider(value) {
        const degrees = value / DegreesToRadians;
        this.div_.find("input[name='rotation']").val(degrees);
        this.div_.find("span[name='rotation']").empty().append(degrees.toFixed(0));
    }

    _aspect_from_slider(value) {
        this.div_.find("span[name='aspect']").empty().append(value.toFixed(1));
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("aspect", value, true);
    }

    _aspect_to_slider(value) {
        this.div_.find("input[name='aspect']").val(value);
        this.div_.find("span[name='aspect']").empty().append(value.toFixed(1));
    }

    _undo() {
        if (this.saved_data_ && this.modifier_canvas_) {
            Object.entries(this.saved_data_).forEach(entry => this.modifier_canvas_.set(entry[0], entry[1], false));
            this.modifier_canvas_.draw();
            this._setup(false);
        }
    }
}

// ----------------------------------------------------------------------

const COS_PI_6 = Math.cos(Math.PI / 6);

class PointStyleModifierCanvas
{
    // {canvas: $(<canvas>), onchange: callback({canvas:, name:, value:}), point_scale: 5}
    constructor(args) {
        this.canvas_ = args.canvas;
        this.canvas_.prop({width: this.canvas_.width(), height: this.canvas_.height()});
        this.context_ = this.canvas_[0].getContext('2d', {alpha: false});
        this.point_scale_ = args.point_scale || 5;
        this.scale_ = this.canvas_.prop("width");
        this.scale_inv_ = 1 / this.scale_;
        this.context_.scale(this.scale_, this.scale_);
        this.context_.translate(0.5, 0.5);
        this.onchange_ = args.onchange;
    }

    draw() {
        this.context_.save();
        this.context_.clearRect(-0.5, -0.5, 1, 1);
        this.context_.fillStyle = this.get("background", "transparent");
        this.context_.fillRect(-0.5, -0.5, 1, 1);
        this._rotation();
        this._aspect();
        this._shape();
        this._outline();
        this._outline_width();
        this._fill_stroke();
        this.context_.restore();
    }

    on(event, callback) {
        this.canvas_.on(event, callback);
    }

    bottom_left_absolute() {
        const offs = this.canvas_.offset();
        return {left: offs.left, top: offs.top + this.canvas_.height()};
    }

    _aspect() {
        const aspect = this.get_float("aspect", 1);
        if (aspect > 0 && aspect !== 1)
            this.context_.scale(aspect, 1);
    }

    _rotation() {
        const rotation = this.get_float("rotation", 0);
        if (rotation !== 0)
            this.context_.rotate(rotation);
    }

    _shape() {
        const radius = this._radius();
        this.context_.beginPath();
        switch (this.get("shape", "unknown")[0].toLowerCase()) {
        case "c":
            this.context_.arc(0, 0, radius, 0, 2*Math.PI);
            break;
        case "b":
        case "r":
            this.context_.moveTo(- radius, - radius);
            this.context_.lineTo(  radius, - radius);
            this.context_.lineTo(  radius,   radius);
            this.context_.lineTo(- radius,   radius);
            this.context_.closePath();
            break;
        case "t":
            const aspect = 1;
            this.context_.moveTo(0, -radius);
            this.context_.lineTo(-radius * COS_PI_6 * aspect, radius / 2);
            this.context_.lineTo(radius * COS_PI_6 * aspect, radius / 2);
            this.context_.closePath();
            break;
        case "u":
        default:
            this.context_.arc(0, 0, radius, 0.5, Math.PI);
            this.context_.lineTo(- radius, - radius);
            this.context_.closePath();
            break;
        }
    }

    _radius() {
        const size = this.get_float("size", 1);
        if (size <= 0)
            return 0;
        const size_scaled = size * this.point_scale_ * this.scale_inv_;
        return size_scaled > 1 ? 0.5 : size_scaled * 0.5;
    }

    _outline() {
        const outline = this.get("outline", "unknown");
        if (outline === "unknown") {
            this.context_.setLineDash([this.scale_inv_ * 3, this.scale_inv_ * 6]);
            this.context_.strokeStyle = "pink";
        }
        else
            this.context_.strokeStyle = outline;
    }

    _outline_width() {
        if (!this.get_raw("outline_width")) {
            this.context_.lineWidth = this.scale_inv_;
            this.context_.setLineDash([this.scale_inv_ * 5, this.scale_inv_ * 5]);
        }
        else {
            let outline_width = this.get_float("outline_width", 1);
            if (outline_width < 1e-5)
                outline_width = 1e-5;
            this.context_.lineWidth = outline_width * this.scale_inv_;
        }
    }

    _fill_stroke() {
        const fill = this.get("fill", "unknown");
        if (fill === "unknown") {
            this._fill_chess("#A0A0FF", "#E0E0E0");
        }
        else if (fill === "transparent") {
            this._fill_chess("#F0F0F0", "#E0E0E0");
        }
        else {
            this.context_.fillStyle = fill;
            this.context_.fill();
            this.context_.stroke();
        }
    }

    _fill_chess(color1, color2) {
        this.context_.stroke();
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

    get_raw(name) {
        return this.canvas_.attr("acv_" + name);
    }

    get(name, dflt) {
        return this.get_raw(name) || dflt;
    }

    get_float(name, dflt) {
        const value = parseFloat(this.get_raw(name));
        return isNaN(value) ? dflt : value;
    }

    set(name, value, draw) {
        this.canvas_.attr("acv_" + name, value);
        if (draw)
            this.draw();
        if (this.onchange_)
            this.onchange_({canvas: this.canvas_, name: name, value: value});
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
