import * as acv_toolkit from "./toolkit.js";
import * as acv_utils from "./utils.js";

acv_utils.load_css("/js/ad/map-draw/ace-view-1/point-style.css");

// ----------------------------------------------------------------------

export function point_style_modifier(args={}) {
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
      <td title='Outline width'>W</td>\
      <td title='Aspect'>A</td>\
      <td title='Rotation'>R</td>\
    </tr>\
    <tr>\
      <td name='fill'></td>\
      <td name='outline'></td>\
      <td><div class='a-point-style-slider-vertical'><input type='range' name='outline_width' value='0' min='-3' max='19' list='point-style-input-tickmarks'></div></td>\
      <td><div class='a-point-style-slider-vertical'><input type='range' name='aspect' value='1' min='0.1' max='1' step='0.1', list='point-style-input-tickmarks-aspect'></div></td>\
      <td><div class='a-point-style-slider-vertical'><input type='range' name='rotation' value='0' min='-180' max='180' step='15' list='point-style-input-tickmarks-angle'></div></td>\
    </tr>\
    <tr>\
      <td colspan='2' class='a-shape'><div class='a-label'>Shape:</div><div class='a-shape' name='circle' title='circle'></div><div class='a-shape' name='box' title='box'></div><div class='a-shape' name='triangle' title='triangle'></div></td>\
      <td class='a-point-style-slider-value'><span class='a-point-style-slider-value' name='outline_width'></span></td>\
      <td class='a-point-style-slider-value'><span class='a-point-style-slider-value' name='aspect'></span></td>\
      <td class='a-point-style-slider-value'><span class='a-point-style-slider-value' name='rotation'></span></td>\
    </tr>\
  </table>\
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
        const colors = ["#000000", "#ffffff", "#808080", "#ff0000", "#00ff00", "#0000ff", "#ffa500", "#6495ed"].concat(acv_toolkit.sAnaColors);
        colors.forEach(color => {
            if (color === "#FFFFFF") {
                td_fill.append(`<div class="a-fill-color a-white" name="${color}" title="fill with ${color}"></div>`);
                td_outline.append(`<div class="a-outline-color a-white" name="${color}" style="background-color: #E0E0E0; border: 3px solid ${color}" title="outline ${color}"></div>`);
            }
            else {
                td_fill.append(`<div class="a-fill-color" name="${color}" style="background-color: ${color}" title="fill with ${color}"></div>`);
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

        this.div_.find("input[name='outline_width']").on("change", evt => this._outline_width_from_slider(parseFloat(evt.currentTarget.value)));
        this.div_.find("input[name='rotation']").on("change", evt => this._rotation_from_slider(parseFloat(evt.currentTarget.value)));
        this.div_.find("input[name='aspect']").on("change", evt => this._aspect_from_slider(parseFloat(evt.currentTarget.value)));
    }

    _setup() {
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

    draw() {
        this.context_.save();
        this.context_.fillStyle = this.get("background");
        this.context_.fillRect(-0.5, -0.5, 1, 1);
        this._rotation();
        this._aspect();
        this._shape();
        this._outline();
        this._outline_width();
        this.context_.stroke();
        this._fill();
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
        const aspect = parseFloat(this.get("aspect", "unknown"));
        if (!isNaN(aspect) && aspect > 0)
            this.context_.scale(aspect, 1);
    }

    _rotation() {
        const rotation = parseFloat(this.get("rotation", "unknown"));
        if (!isNaN(rotation))
            this.context_.rotate(rotation);
    }

    _shape() {
        const outline_width = parseFloat(this.get("outline_width", "unknown")) || 1;
        let radius = 0.35 - this.scale_ * outline_width;
        this.context_.beginPath();
        switch (this.get("shape", "unknown").toLowerCase()) {
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
        const outline = this.get("outline", "unknown");
        if (outline === "unknown") {
            this.context_.setLineDash([this.scale_ * 3, this.scale_ * 6]);
            this.context_.strokeStyle = "pink";
        }
        else
            this.context_.strokeStyle = outline;
    }

    _outline_width() {
        let outline_width = parseFloat(this.get("outline_width", "unknown"));
        if (isNaN(outline_width)) {
            outline_width = 1;
            this.context_.setLineDash([this.scale_ * 5, this.scale_ * 5]);
        }
        else if (outline_width < 1e-5)
            outline_width = 1e-5;
        this.context_.lineWidth = this.scale_ * outline_width * 2;
    }

    _fill() {
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

    get(name, dflt) {
        return this.canvas_.attr("acv_" + name) || dflt;
    }

    set(name, value, draw) {
        this.canvas_.attr("acv_" + name, value);
        if (draw)
            this.draw();
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
