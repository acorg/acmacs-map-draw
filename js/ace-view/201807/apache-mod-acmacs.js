import * as av_ace from "./ace-view.js";
import * as av_utils from "./utils.js";

// ----------------------------------------------------------------------

// args: {parent:, uri:, view_mode:, coloring:, }
export function show_antigenic_map_widget(args) {
    const widget_options = {
        view_mode: {mode: args.view_mode || "all"},
        coloring: args.coloring || "original",
        canvas_size: args.canvas_size,
        api: new Api({uri: args.uri}),
        on_data_load_failure: args.on_data_load_failure || (uri => console.error("failed to load antigenic map data from ", uri))
    };
    new av_ace.AntigenicMapWidget(args.parent, args.uri + "?acv=ace", widget_options);
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

    download_error_lines(args) {
        this._download({command: Object.assign({C: "download_error_lines", id: this.source_id, format: "csv", projection_no: 0}, args), suffix: ".error-lines.csv", blob_type: "application/octet-stream"});
    }

    download_distances_between_all_points_plain(args) {
        this._download({command: Object.assign({C: "download_distances_between_all_points", id: this.source_id, format: "text", projection_no: 0}, args), suffix: ".map-distances.txt", blob_type: "application/octet-stream"});
    }

    download_distances_between_all_points_csv(args) {
        this._download({command: Object.assign({C: "download_distances_between_all_points", id: this.source_id, format: "csv", projection_no: 0}, args), suffix: ".map-distances.csv", blob_type: "application/octet-stream"});
    }

    download_sequences_of_chart_as_fasta(args) {
        this._download({command: Object.assign({C: "download_sequences_of_chart_as_fasta", id: this.source_id}, args), suffix: ".fasta", blob_type: "application/octet-stream"});
    }

    async get_sequences() {
        return Promise.resolve(this._post_expect_json({command: {C: "sequences_of_chart", id: this.source_id}}));
    }

    // {command:, blob_type:}
    _download(args) {
        const pathname = this.uri.split("/");
        let filename = pathname[pathname.length - 1];
        if (filename.substr(filename.length - args.suffix.length, args.suffix.length) !== args.suffix)
            filename = filename + args.suffix;
        this._post_expect_blob(args).done(result => av_utils.download_blob({data: result, filename: filename}));
    }

    // {command:}
    _post_expect_blob(args) {
        return $.post({
            url: this.uri + "?acv=post",
            data: JSON.stringify(args.command),
            cache: false,
            xhr: () => { let xhr = new XMLHttpRequest(); xhr.responseType = "blob"; return xhr; }
        });
    }

    // {command:}
    _post_expect_json(args) {
        return $.post(this.uri + "?acv=post", JSON.stringify(args.command));
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
