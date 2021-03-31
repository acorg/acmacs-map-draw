// Tabs https://www.w3schools.com/howto/howto_js_tabs.asp
//
// <script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
// <script src="/js/acmacs-d/map-draw/tabs.js"></script>
//
// tabs with maps (pdf), procrustes(pdf), ace-view widget (via <script src="/js/acmacs-d/map-draw/ace-view/201807/widget.js"></script>)
//
// <div class="ac-tabs">
//   <div class="tabcontent pdf tab-default" title="Map"                src="map.pdf"></div>
//   <div class="tabcontent pdf"             title="Procrustes"         src="pc.pdf"></div>
//   <div class="tabcontent ace-view-widget" title="Interactive viewer" src="map.ace"></div>
// </div>

if (!window.hasOwnProperty("$"))
    throw 'jquery is required, include \<script src="https://code.jquery.com/jquery-3.6.0.min.js"\>\</script\>';
$(document).ready(() => {
    $(".ac-tabs").each((_, ac_tab) => {
        const $ac_tab = $(ac_tab);
        const $tab = $("<div></div>").addClass("tablinks").prependTo($ac_tab);
        $ac_tab.children(".tabcontent").each((_, tabcontent) => {
            const button = $("<button></button>")
                  .addClass("tablink")
                  .html(tabcontent.getAttribute("title"))
                  .on("click", ev => {
                      $ac_tab.children(".tabcontent").each((_, tc) => tc.style.display = "none");
                      $ac_tab.find(".tablink").each((_, tl) => tl.classList.remove("active"));
                      tabcontent.style.display = "block";
                      ev.currentTarget.classList.add("active");
                      if ($(tabcontent).hasClass("ace-view-widget"))
                          activate_antigenic_map_widget(tabcontent);
                  })
                  .appendTo($tab);
            if ($(tabcontent).hasClass("tab-default"))
                button.addClass("tab-default");
        });
        $tab.children(".tab-default").click();
    });
});
