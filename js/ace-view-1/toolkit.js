import * as acv_utils from "./utils.js";

acv_utils.load_css("/js/ad/map-draw/ace-view-1/toolkit.css");

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

const MousePopup_id = "amw201805-mouse-popup";

class MousePopup extends Popup_Base {

    find_element() {
        return $("#" + MousePopup_id);
    }

    create_element() {
        return $("<div id='" + MousePopup_id + "'></div>").appendTo($("body"));
    }
}

// returns popup div
export function mouse_popup_show(contents, parent, offsets_to_parent, classes="a-window-shadow") {
    return new MousePopup().classes(classes).show(contents, parent, offsets_to_parent).find_element();
}

export function mouse_popup_hide() {
    return $("#" + MousePopup_id).hide();
}

// ----------------------------------------------------------------------

export class Popup extends Popup_Base {

    constructor(css_classes) {
        super(css_classes);
        this.div_id = "amw201805-popup-" + window.amw201805.new_id();
    }

    find_element() {
        return $("#" + this.div_id);
    }

    create_element() {
        return $("<div class='amw201805-popup' id='" + this.div_id + "'></div>").appendTo($("body"));
    }
}

// ----------------------------------------------------------------------

const Modal_click_background_id = "amw201805-modal-click-background";

const Modal_click_background_html = "\
<div id='" + Modal_click_background_id + "'>\
</div>\
";


class ModalClickBackground
{
    constructor() {
        this.div = $("body").find("#" + Modal_click_background_id);
        if (this.div.length === 0)
            this.div = $(Modal_click_background_html).appendTo($("body"));
    }

    destroy() {
        this.div.remove();
    }

    show() {
        this.div.css({width: $(document).width(), height: $(document).height()});
        this.div.show();
    }

    hide() {
        this.div.hide();
    }

    onclick(callback) {
        this.div.on("click", callback);
    }
}

// ----------------------------------------------------------------------

export class Modal
{
    constructor(content) {
        this.menu = $("<div class='amw201805-burger-menu'>" + content + "</div>").appendTo($("body"));
        this.background = new ModalClickBackground();
        this.background.onclick(() => this.destroy());
    }

    destroy() {
        this.menu.remove();
        this.background.destroy();
    }

    classes(classes) {
        if (classes)
            this.menu.addClass(classes);
        return this;
    }

    find(selector) {
        return this.menu.find(selector);
    }

    show(element) {
        this.move_to_element(element);
        this.menu.show();
        this.background.show();
    }

    // hide() {
    //     this.menu.hide();
    //     this.background.hide();
    // }

    move_to_element(element) {
        const offset = element.offset();
        this.menu.css({left: offset.left + element.outerWidth(true) - this.menu.outerWidth(true), top: offset.top + element.outerHeight(true)});
    }
}


// ----------------------------------------------------------------------

const MovableWindow_html = "\
<div class='amw201805-movable-window a-window-shadow'>\
  <div class='a-window-title'>\
    <div class='a-close a-left' title='Close'>&times;</div>\
    <div class='a-title'></div>\
    <div class='a-close a-right' title='Close'>&times;</div>\
  </div>\
  <div class='a-content'></div>\
  <div class='a-window-resizer'></div>\
</div>\
";

export class MovableWindow {

    // {title, content, parent: "center"}
    constructor(args) {
        this.div = $(MovableWindow_html).appendTo($("body"));
        if (args.title)
            this.div.find(".a-title").append(args.title);
        if (args.content)
            this.div.find(".a-content").append(args.content);
        if (args.parent === "center") {
            const wind = $(window);
            this.div.css({left: (wind.scrollLeft() + wind.width() - this.div.width()) / 2, top: (wind.scrollTop() + wind.height() - this.div.height()) / 2});
        }
        else {
            this.div.css($(args.parent).offset());
        }
        this.div.find(".a-close").on("click", () => this.destroy());
        this.div.find(".a-title").on("mousedown", evt => this.drag(evt, pos_diff => this.drag_window(pos_diff)));
        this.div.find(".a-window-resizer").on("mousedown", evt => this.drag(evt, pos_diff => this.resize_window(pos_diff)));
    }

    destroy() {
        this.div.remove();
    }

    classes(classes) {
        if (classes)
            this.div.addClass(classes);
        return this;
    }

    drag(evt, callback) {
        let mouse_pos = {left: evt.clientX, top: evt.clientY};
        document.onmouseup = () => {
            document.onmouseup = document.onmousemove = null;
            this.div.find(".a-content").removeClass("a-unselectable");
            $("body").removeClass("a-unselectable");
        };
        document.onmousemove = evt2 => {
            this.div.find(".a-content").addClass("a-unselectable");
            $("body").addClass("a-unselectable");
            callback({left: mouse_pos.left - evt2.clientX, top: mouse_pos.top - evt2.clientY});
            mouse_pos = {left: evt2.clientX, top: evt2.clientY};
        };
    }

    drag_window(pos_diff) {
        this.div.css({left: this.div.offset().left - pos_diff.left, top : this.div.offset().top - pos_diff.top});
    }

    resize_window(pos_diff) {
        const element = this.div.find(".a-content");
        element.css({width: element.width() - pos_diff.left, height : element.height() - pos_diff.top});
    }
}

// ----------------------------------------------------------------------

export function movable_window_with_json(data, invoking_node, title) {
    new MovableWindow({title: title || data.name || data.description || data._id, content: `<pre class='json-highlight'>${acv_utils.json_syntax_highlight(JSON.stringify(data, undefined, 2))}</pre>`, parent: invoking_node});
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End: