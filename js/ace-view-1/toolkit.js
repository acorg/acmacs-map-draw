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
        let popup = this.find_or_create();
        popup.empty();
        if (typeof(contents) === "function")
            contents(popup);
        else
            popup.append(contents);
        popup.css({left: offsets_to_parent.left + parent.offset().left, top: offsets_to_parent.top + parent.offset().top}).show();
    }

    // show_ul(text_rows, parent, offsets_to_parent) {
    //     this.show($("<ul></ul>").append(text_rows.map(text => "<li>" + text + "</li>").join("")), parent, offsets_to_parent);
    // }

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

export function mouse_popup_show(contents, parent, offsets_to_parent) {
    new MousePopup().show(contents, parent, offsets_to_parent);
}

export function mouse_popup_hide() {
    $("#" + MousePopup_id).hide();
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

// export class Popup {

//     constructor(title, content, parent, classes="") {
//         this.div = $(`<div class='ADT_Popup1 adt-shadow ${classes}'><div class='adt-x-window-title'><div class='adt-x-close adt-x-left' title='Close'>&times;</div><div class='adt-x-title'>${title}</div><div class='adt-x-close adt-x-right' title='Close'>&times;</div></div><div class='adt-x-content'>${content}</div></div>`).appendTo($("body"));
//         if (parent === "center") {
//             const wind = $(window);
//             this.div.css({left: (wind.scrollLeft() + wind.width() - this.div.width()) / 2, top: (wind.scrollTop() + wind.height() - this.div.height()) / 2});
//         }
//         else {
//             this.div.css($(parent).offset());
//         }
//         this.div.find(".adt-x-close").on("click", () => this.destroy());
//         this.div.find(".adt-x-title").on("mousedown", evt => this.title_mouse_down(evt));
//     }

//     destroy() {
//         this.div.remove();
//     }

//     title_mouse_down(evt) {
//         this.pos_start = {left: evt.clientX, top: evt.clientY};
//         document.onmouseup = evt => this.title_mouse_up(evt);
//         document.onmousemove = evt => this.title_mouse_move(evt);
//     }

//     title_mouse_up(evt) {
//         document.onmouseup = document.onmousemove = null;
//     }

//     title_mouse_move(evt) {
//         this.pos_current = offset_sub(this.pos_start, {left: evt.clientX, top: evt.clientY});
//         this.pos_start = {left: evt.clientX, top: evt.clientY};
//         this.div.css(offset_sub(this.div.offset(), this.pos_current));
//     }
// }

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
