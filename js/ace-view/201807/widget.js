// <script src="/js/acmacs-d/map-draw/ace-view/201807/widget.js"></script>
//
// creates activate_antigenic_map_widget function:
// activate_antigenic_map_widget(div)
// see /js/acmacs-d/map-draw/tabs.js for usage example

if (!window.hasOwnProperty("$"))
    throw 'jquery is required, include \<script src="https://code.jquery.com/jquery-3.6.0.min.js"\>\</script\>';
window.activate_antigenic_map_widget = function(div) {
    const $div = $(div);
    let widget = $div.data("ace_view_widget");
    if (!widget) {
        import("/js/acmacs-d/map-draw/ace-view/201807/apache-mod-acmacs.js").then(mod => {
            try {
                const widget = mod.show_antigenic_map_widget({
                    parent: $div,
                    canvas_size: 800,
                    coloring: "default",
                    uri: div.getAttribute("src") || div.getAttribute("name"),
                    on_data_load_failure: err => {
                        console.error("Cannot load map from " + div.getAttribute("name"), err);
                    }
                });
                $div.data("ace_view_widget", widget);
            }
            catch (err) {
                console.error("Cannot load map from " + div.getAttribute("name"), err);
            }
        });
    }
    else {
        widget.redraw();
    }
}
