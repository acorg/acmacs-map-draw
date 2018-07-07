import * as av_utils from "./utils.js";

av_utils.load_css("/js/ad/map-draw/ace-view/201807/toolkit.css");

// ----------------------------------------------------------------------

export const sGREY = "#c0c0c0";
export const sLIGHTGREY = "#e0e0e0";
export const sAnaColors = ["#03569b", "#e72f27", "#ffc808", "#a2b324", "#a5b8c7", "#049457", "#f1b066", "#742f32", "#9e806e", "#75ada9", "#675b2c", "#a020f0", "#8b8989", "#e9a390", "#dde8cf", "#00939f"];

// export function ana_colors(index) {
//     if (index < 0 || index >= sAnaColors.length)
//         return sGREY;
//     return sAnaColors[index];
// }

// ----------------------------------------------------------------------

export class Modal
{
    // args: {element:, z_index:, dismiss: function}
    constructor(args) {
        this.background_ = $("<div></div>").appendTo("body")
            .css({width: $(document).width(), height: $(document).height(), position: "absolute", left: 0, top: 0, "z-index": args.z_index || 900})
            .on("click", evt => av_utils.forward_event(evt, () => {
                if (args.dismiss)
                    args.dismiss();
                this.destroy();
            }))
            .show();
        if (args.element) {
            args.element.css("z-index", args.z_index ? args.z_index + 1 : 901);
        }
    }

    destroy() {
        if (this.background_) {
            this.background_.remove();
            delete this.background_;
        }
    }
}

// ----------------------------------------------------------------------

export function drag(evt, callback) {
    let mouse_pos = {left: evt.clientX, top: evt.clientY};
    document.onmouseup = () => {
        $("body").removeClass("av-unselectable");
        document.onmouseup = document.onmousemove = null;
    };
    document.onmousemove = evt2 => {
        $("body").addClass("av-unselectable");
        callback({left: evt2.clientX - mouse_pos.left, top: evt2.clientY - mouse_pos.top});
        mouse_pos = {left: evt2.clientX, top: evt2.clientY};
    };
}

// ----------------------------------------------------------------------

class Popup_Base {

    constructor(css_classes) {
        this.css_classes = css_classes;
    }

    destroy() {
        this.find_element().remove();
    }

    find_or_create() {
        let elt = this.find_element();
        if (elt.length === 0) {
            elt = this.create_element();
            if (this.css_classes)
                elt.addClass(this.css_classes);
        }
        return elt;
    }

    show(contents, parent, offsets_to_parent) {
        this.hide();
        const popup = this.find_or_create();
        popup.empty();
        if (typeof(contents) === "function")
            contents(popup);
        else
            popup.append(contents);
        popup.css({left: offsets_to_parent.left + parent.offset().left, top: offsets_to_parent.top + parent.offset().top}).show();
        return this;
    }

    // show_ul(text_rows, parent, offsets_to_parent) {
    //     this.show($("<ul></ul>").append(text_rows.map(text => "<li>" + text + "</li>").join("")), parent, offsets_to_parent);
    // }

    classes(classes) {
        if (classes)
            this.find_or_create().addClass(classes);
        return this;
    }

    hide() {
        this.find_element().hide();
    }
}

// ----------------------------------------------------------------------

const MousePopup_id = "amw201807-mouse-popup";

class MousePopup extends Popup_Base {

    find_element() {
        return $("#" + MousePopup_id);
    }

    create_element() {
        return $("<div id='" + MousePopup_id + "'></div>").appendTo($("body"));
    }
}

// returns popup div
export function mouse_popup_show(contents, parent, offsets_to_parent, classes="av201807-window-shadow") {
    return new MousePopup().classes(classes).show(contents, parent, offsets_to_parent).find_element();
}

export function mouse_popup_hide() {
    return $("#" + MousePopup_id).hide();
}

// ----------------------------------------------------------------------

export class BurgerMenu
{
    // {trigger: $("#trigger"), menu: $("#menu"), callback: item => console.log("burger-menu", item)}
    constructor(args) {
        this.menu_ = args.menu.addClass("av201807-burger-menu av201807-window-shadow");
        this.menu_.find("ul").addClass("av201807-window-shadow");
        this.menu_.find("li[menu]").on("click", evt => av_utils.forward_event(evt, () => this.clicked(evt.currentTarget)));
        this.menu_.find("li:has(>ul)").append("<div class='av-burger-menu-arrow'>&#9654</div>");
        this.callback_ = args.callback;
        this.trigger_ = args.trigger.addClass("av201807-burger-menu-trigger");
        this.trigger_.on("click", evt => av_utils.forward_event(evt, () => this.show()));
    }

    destroy() {
        this.trigger_.off("click");
        this.menu_.find("li[menu]").off("click");
    }

    show() {
        this.modal_support_ = new Modal({element: this.menu_, z_index: 1000, dismiss: () => this.hide()});
        const trigger_pos = this.trigger_.position();
        this.menu_.show().css({left: trigger_pos.left, top: trigger_pos.top + this.trigger_.height()});
        this.menu_.find("> li > ul").css("left", this.menu_.width());
    }

    hide() {
        this.menu_.hide();
        this.modal_support_.destroy();
        delete this.modal_support_;
    }

    clicked(target) {
        if (!target.matches(".av-menu-disabled") && target.matches("[menu]")) {
            this.hide();
            if (this.callback_)
                this.callback_(target.getAttribute("menu"));
        }
    }
}

// ----------------------------------------------------------------------

const MovableWindow_html = "\
<div class='amw201807-movable-window av201807-window-shadow'>\
  <div class='av-window-title'>\
    <div class='av-close av-left' title='Close'>&times;</div>\
    <div class='av-title'></div>\
    <div class='av-close av-right' title='Close'>&times;</div>\
  </div>\
  <div class='av-content'></div>\
  <div class='av-window-resizer'></div>\
</div>\
";

export class MovableWindow {

    // {title, content, id, parent: "center", classes: "", content_css: {width:, height:}, on_destroy: function}
    constructor(args) {
        if (args.id)
            this.div = $("body").find("#" + args.id);
        if (!this.div || this.div.length === 0) {
            this.div = $(MovableWindow_html).appendTo($("body"));
            if (args.id)
                this.div.attr("id", args.id);
            this.div.find(".av-close").on("click", () => this.destroy());
            this.div.find(".av-window-title").on("mousedown", evt => this.drag(evt, pos_diff => this.drag_window(pos_diff)));
            this.div.find(".av-window-resizer").on("mousedown", evt => this.drag(evt, pos_diff => this.resize_window(pos_diff)));
            if (args.classes)
                this.div.addClass(args.classes);
        }
        if (args.title)
            this.div.find(".av-title").empty().append(args.title);
        if (args.content)
            this.content().empty().append(args.content);
        if (args.content_css)
            this.content().css(args.content_css);
        this.position(args.parent);
        this.on_destroy = args.on_destroy;
    }

    destroy() {
        this.div.remove();
        if (this.on_destroy)
            this.on_destroy();
    }

    classes(classes) {
        if (classes)
            this.div.addClass(classes);
        return this;
    }

    content() {
        return this.div.find(".av-content");
    }

    drag(evt, callback) {
        let mouse_pos = {left: evt.clientX, top: evt.clientY};
        document.onmouseup = () => {
            document.onmouseup = document.onmousemove = null;
            this.div.find(".av-content").removeClass("av-unselectable");
            $("body").removeClass("av-unselectable");
        };
        document.onmousemove = evt2 => {
            this.div.find(".av-content").addClass("av-unselectable");
            $("body").addClass("av-unselectable");
            callback({left: mouse_pos.left - evt2.clientX, top: mouse_pos.top - evt2.clientY});
            mouse_pos = {left: evt2.clientX, top: evt2.clientY};
        };
    }

    drag_window(pos_diff) {
        this.div.css({left: this.div.offset().left - pos_diff.left, top : this.div.offset().top - pos_diff.top});
    }

    resize_window(pos_diff) {
        const element = this.div.find(".av-content");
        element.css({width: element.width() - pos_diff.left, height : element.height() - pos_diff.top});
    }

    position(parent) {
        if (!parent || parent === "center") {
            const wind = $(window);
            this.div.css({left: (wind.scrollLeft() + wind.width() - this.div.width()) / 2, top: (wind.scrollTop() + wind.height() - this.div.height()) / 2});
        }
        else {
            let offset = parent.offset();
            offset.left += parent.outerWidth();
            this.div.css(offset);
        }
    }
}

// ----------------------------------------------------------------------

export function movable_window_with_json(data, invoking_node, title) {
    new MovableWindow({title: title || data.name || data.description || data._id, content: `<pre class='json-highlight'>${av_utils.json_syntax_highlight(JSON.stringify(data, undefined, 2))}</pre>`, parent: invoking_node}).classes("av-json-data");
}

// ----------------------------------------------------------------------

export function movable_window_with_error(data, invoking_node) {
    if (typeof(data) === "object")
        data = `<pre class='json-highlight'>${av_utils.json_syntax_highlight(JSON.stringify(data, undefined, 2))}</pre>`;
    else
        data = `<p>${data}</p>`;
    new MovableWindow({title: "ERROR", content: data, parent: invoking_node}).classes("av-error");
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
