import * as av_toolkit from "./toolkit.js";
import * as av_surface from "./surface.js";
import * as av_utils from "./utils.js";

av_utils.load_css("/js/ad/map-draw/ace-view/201807/point-style.css");

// ----------------------------------------------------------------------

// canvas attributes:
// aw_s - Shape
// aw_f - Fill
// aw_o - Outline
// av_o - outline width
// av_a - aspect
// av_r - rotation
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
        this.canvas_.on("click", evt => av_utils.forward_event(evt, () => this.show_dialog()));
    }

    show_dialog() {
        if (!point_style_modifier_dialog_singleton)
            point_style_modifier_dialog_singleton = new PointStyleModifierDialog();
        point_style_modifier_dialog_singleton.show(this.canvas_);
    }
}

// ----------------------------------------------------------------------

const PointStyleModifierDialog_html = "\
<div class='av201807-point-style-modifier-dialog av201807-window-shadow'>\
  <table>\
    <tr class='title'>\
      <td>Fill</td>\
      <td>Outline</td>\
      <td title='Size'>S</td>\
      <td title='Outline width'>W</td>\
      <td title='Aspect'>A</td>\
      <td title='Rotation'>R</td>\
    </tr>\
    <tr>\
      <td name='F'></td>\
      <td name='O'></td>\
      <td><div class='av-point-style-slider-vertical'><input type='range' name='s' value='0' min='-3' max='9' list='point-style-input-tickmarks'></div></td>\
      <td><div class='av-point-style-slider-vertical'><input type='range' name='o' value='0' min='-3' max='19' list='point-style-input-tickmarks'></div></td>\
      <td><div class='av-point-style-slider-vertical'><input type='range' name='a' value='1' min='0.1' max='1' step='0.1', list='point-style-input-tickmarks-aspect'></div></td>\
      <td><div class='av-point-style-slider-vertical'><input type='range' name='r' value='0' min='-180' max='180' step='15' list='point-style-input-tickmarks-angle'></div></td>\
    </tr>\
    <tr>\
      <td colspan='2' class='shape'>\
        <div class='label'>Shape:</div>\
        <div class='shape' name='circle' title='circle'></div>\
        <div class='shape' name='box' title='box'></div>\
        <div class='shape' name='triangle' title='triangle'></div>\
      </td>\
      <td class='av-point-style-slider-value'><span class='av-point-style-slider-value' name='s'></span></td>\
      <td class='av-point-style-slider-value'><span class='av-point-style-slider-value' name='o'></span></td>\
      <td class='av-point-style-slider-value'><span class='av-point-style-slider-value' name='a'></span></td>\
      <td class='av-point-style-slider-value'><span class='av-point-style-slider-value' name='r'></span></td>\
    </tr>\
  </table>\
  <div class='reset-button' title='undo all changes'><div></div></div>\
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
        this.modal_ = new av_toolkit.Modal({element: this.div_, z_index: 900, dismiss: () => this.hide()});
        this.div_.show();
    }

    hide() {
        this.modifier_canvas_ = null;
        this.div_.hide();
    }

    _make() {
        this.div_ = $(PointStyleModifierDialog_html).appendTo("body").hide().css({position: "absolute"});

        const td_fill = this.div_.find('td[name="F"]');
        const td_outline = this.div_.find('td[name="O"]');
        const colors = ["#000000", "white", "transparent", "#ff0000", "#00ff00", "#0000ff", "#ffa500", "#6495ed"].concat(av_toolkit.sAnaColors);
        colors.forEach(color => {
            if (color === "white") {
                td_fill.append(`<div class="fill-color white" name="${color}" title="fill ${color}"></div>`);
                td_outline.append(`<div class="outline-color white" name="${color}" style="background-color: #E0E0E0; border: 3px solid ${color}" title="${color} outline"></div>`);
            }
            else if (color === "transparent") {
                td_fill.append(`<div class="fill-color transparent" name="${color}" title="fill ${color}"></div>`);
                td_outline.append(`<div class="outline-color transparent" name="${color}" title="${color} outline"></div>`);
            }
            else {
                td_fill.append(`<div class="fill-color" name="${color}" style="background-color: ${color}" title="fill ${color}"></div>`);
                td_outline.append(`<div class="outline-color" name="${color}" style="border: 3px solid ${color}" title="outline ${color}"></div>`);
            }
        });
        this.div_.find("div.fill-color").on("click", evt => av_utils.forward_event(evt, () => this._fill(evt.currentTarget.getAttribute("name"))));
        this.div_.find("div.outline-color").on("click", evt => av_utils.forward_event(evt, evt => this._outline(evt.currentTarget.getAttribute("name"))));
        this.div_.find("div.shape").on("click", evt => av_utils.forward_event(evt, evt => this._shape(evt.currentTarget.getAttribute("name"))));

        const tickmarks = this.div_.find("datalist#point-style-input-tickmarks").empty();
        for (let i = -20; i <= 20; i += 2)
            tickmarks.append(`<option value='${i}'>`);
        const tickmarks_angle = this.div_.find("datalist#point-style-input-tickmarks-angle").empty();
        for (let angle = -180; angle <= 180; angle += 30)
            tickmarks_angle.append(`<option value='${angle}'>`);
        const tickmarks_aspect = this.div_.find("datalist#point-style-input-tickmarks-aspect").empty();
        for (let aspect = 0; aspect <= 1; aspect += 0.2)
            tickmarks_aspect.append(`<option value='${aspect}'>`);

        this.div_.find("input[name='s']").on("input", evt => this._size_from_slider(parseFloat(evt.currentTarget.value)));
        this.div_.find("input[name='o']").on("input", evt => this._outline_width_from_slider(parseFloat(evt.currentTarget.value)));
        this.div_.find("input[name='r']").on("input", evt => this._rotation_from_slider(parseFloat(evt.currentTarget.value)));
        this.div_.find("input[name='a']").on("input", evt => this._aspect_from_slider(parseFloat(evt.currentTarget.value)));

        this.div_.find("div.reset-button").on("click", evt => av_utils.forward_event(evt, () => this._undo()));
    }

    _setup(save=true) {
        if (save)
            this.saved_data_ = ["S", "F", "O", "o", "s", "r", "a"].reduce((data, name) => { data[name] = this.modifier_canvas_.get(name, null); return data; }, {});
        this._size_to_slider(parseFloat(this.modifier_canvas_.get("s", 1)));
        this._outline_width_to_slider(parseFloat(this.modifier_canvas_.get("o", 1)));
        this._rotation_to_slider(parseFloat(this.modifier_canvas_.get("r", 0)));
        this._aspect_to_slider(parseFloat(this.modifier_canvas_.get("a", 1)));
    }

    _fill(color) {
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("F", color, true);
    }

    _outline(color) {
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("O", color, true);
    }

    _shape(shape) {
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("S", shape, true);
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
        this.div_.find("span[name='s']").empty().append(value.toFixed(1));
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("s", value, true);
    }

    _size_to_slider(value) {
        const slider = this.div_.find("input[name='s']");
        if (value >= 1)
            slider.val(value - 1);
        else if (value < 1 && value >= 0.7)
            slider.val(-1);
        else if (value < 0.7 && value >= 0.4)
            slider.val(-2);
        else
            slider.val(-3);
        this.div_.find("span[name='s']").empty().append(value.toFixed(1));
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
        this.div_.find("span[name='o']").empty().append(value);
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("o", value, true);
    }

    _outline_width_to_slider(value) {
        const slider = this.div_.find("input[name='o']");
        if (value >= 1)
            slider.val(value - 1);
        else if (value < 0.01)
            slider.val(-3);
        else
            slider.val(Math.log10(value));
        this.div_.find("span[name='o']").empty().append(value);
    }

    _rotation_from_slider(value) {
        this.div_.find("span[name='r']").empty().append(value);
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("r", value * DegreesToRadians, true);
    }

    _rotation_to_slider(value) {
        const degrees = value / DegreesToRadians;
        this.div_.find("input[name='r']").val(degrees);
        this.div_.find("span[name='r']").empty().append(degrees.toFixed(0));
    }

    _aspect_from_slider(value) {
        this.div_.find("span[name='a']").empty().append(value.toFixed(1));
        if (this.modifier_canvas_)
            this.modifier_canvas_.set("a", value, true);
    }

    _aspect_to_slider(value) {
        this.div_.find("input[name='a']").val(value);
        this.div_.find("span[name='a']").empty().append(value.toFixed(1));
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

export class PointStyleModifierCanvasAccess
{
    constructor(canvas) {
        this.canvas_ = canvas;
    }

    _attr_name(name) {
        return (name[0].toLowerCase() === name[0] ? "av_" : "aw_") + name;
    }

    get_raw(name) {
        return this.canvas_.attr(this._attr_name(name));
    }

    get(name, dflt) {
        return this.get_raw(name) || dflt;
    }

    get_float(name, dflt) {
        const value = parseFloat(this.get_raw(name));
        return isNaN(value) ? dflt : value;
    }

    set(name, value) {
        this.canvas_.attr(this._attr_name(name), value);
    }
}

class PointStyleModifierCanvas extends PointStyleModifierCanvasAccess
{
    // {canvas: $(<canvas>), onchange: callback({canvas:, name:, value:}), point_scale: 5}
    constructor(args) {
        super(args.canvas);
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
        av_surface.draw_point(this.context_,
                              {S: this.get("S", "unknown"), F: this.get("F", "unknown"), O: this.get("O", "unknown"),
                               radius: this._radius(), r: this.get_float("r", 0), a: this.get_float("a", 1), o: this.get_float("o", null), scale_inv: this.scale_inv_});
        this.context_.restore();
    }

    on(event, callback) {
        this.canvas_.on(event, callback);
    }

    bottom_left_absolute() {
        const offs = this.canvas_.offset();
        return {left: offs.left, top: offs.top + this.canvas_.height()};
    }

    _radius() {
        const size = this.get_float("s", 1);
        if (size <= 0)
            return 0;
        const size_scaled = size * this.point_scale_ * this.scale_inv_;
        return size_scaled > 1 ? 0.5 : size_scaled * 0.5;
    }

    set(name, value, draw) {
        super.set(name, value);
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
