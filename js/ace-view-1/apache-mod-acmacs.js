import * as acv_m from "./ace-view.js";

// ----------------------------------------------------------------------

// args: {parent:, uri:, view_mode:, coloring:, }
export function show_antigenic_map_widget(args) {
    const widget_options = {
        view_mode: args.view_mode || "best-projection",
        coloring: args.coloring || "default",
        api: new Api({uri: args.uri})
    };
    new acv_m.AntigenicMapWidget(args.parent, args.uri + "?acv=ace", widget_options);
}

// ----------------------------------------------------------------------

class Api
{
    constructor(args) {
        this.uri = args.uri;
    }

    download_pdf(args) {
        this._download({command: {C: "download_pdf"}, suffix: ".pdf"});
    }

    download_ace(args) {
        this._download({command: {C: "download_ace"}, suffix: ".ace"});
    }

    download_save(args) {
        this._download({command: {C: "download_save"}, suffix: ".save"});
    }

    download_layout_plain(args) {
        this._download({command: Object.assign({C: "download_layout", id: this.source_id, format: "text", projection_no: 0}, args), suffix: ".layout.txt", blob_type: "application/octet-stream"});
    }

    download_layout_csv(args) {
        this._download({command: Object.assign({C: "download_layout", id: this.source_id, format: "csv", projection_no: 0}, args), suffix: ".layout.csv", blob_type: "application/octet-stream"});
    }

    download_table_map_distances_plain(args) {
        this._download({command: Object.assign({C: "download_table_map_distances", id: this.source_id, format: "text", projection_no: 0}, args), suffix: ".table-map-distances.txt", blob_type: "application/octet-stream"});
    }

    download_table_map_distances_csv(args) {
        this._download({command: Object.assign({C: "download_table_map_distances", id: this.source_id, format: "csv", projection_no: 0}, args), suffix: ".table-map-distances.csv", blob_type: "application/octet-stream"});
    }

    // download_error_lines(args) {
    //     this._download({command: Object.assign({C: "download_error_lines", id: this.source_id, projection_no: 0}, args), blob_type: "application/octet-stream"});
    // }

    download_distances_between_all_points_plain(args) {
        this._download({command: Object.assign({C: "download_distances_between_all_points", id: this.source_id, format: "text", projection_no: 0}, args), suffix: ".map-distances.txt", blob_type: "application/octet-stream"});
    }

    download_distances_between_all_points_csv(args) {
        this._download({command: Object.assign({C: "download_distances_between_all_points", id: this.source_id, format: "csv", projection_no: 0}, args), suffix: ".map-distances.csv", blob_type: "application/octet-stream"});
    }

    // // {command:, blob_type:}
    _download(args) {
        $.post({
            url: this.uri + "?acv=post",
            data: JSON.stringify(args.command),
            cache: false,
            xhr: () => { let xhr = new XMLHttpRequest(); xhr.responseType= 'blob'; return xhr; }
        }).done(result => {
            const pathname = this.uri.split("/");
            let filename = pathname[pathname.length - 1];
            if (filename.substr(filename.length - args.suffix.length, args.suffix.length) !== args.suffix)
                filename = filename + args.suffix;
            const url = window.URL.createObjectURL(result);
            const link = $(`<a href='${url}' download='${filename}'></a>`).appendTo($("body"));
            link[0].click();
            link.remove();
            window.setTimeout(() =>  window.URL.revokeObjectURL(url), 100);   // For Firefox it is necessary to delay revoking the ObjectURL
        });
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
