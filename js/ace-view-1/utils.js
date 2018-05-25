// ----------------------------------------------------------------------

// https://stackoverflow.com/questions/4810841/how-can-i-pretty-print-json-using-javascript
export function json_syntax_highlight(data, options={object_id_href_prefix: window.acv_url_prefix + "doc/"}) {
    data = data.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
    return data.replace(/("(\\u[a-zA-Z0-9]{4}|\\[^u]|[^\\"])*"(\s*:)?|\b(true|false|null)\b|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?)/g, match => {
        var cls = 'json-syntax-number';
        if (/^"/.test(match)) {
            if (/:$/.test(match)) {
                cls = 'json-syntax-key';
            }
            else if (/^"[0-9a-f]{24}"/.test(match)) {
                cls = 'json-syntax-object-id';
            }
            else {
                cls = 'json-syntax-string';
            }
        }
        else if (/true|false/.test(match)) {
            cls = 'json-syntax-boolean';
        }
        else if (/null/.test(match)) {
            cls = 'json-syntax-null';
        }
        if (cls === 'json-syntax-object-id') {
            const id_itself = match.replace(/"/g, '');
            return `"<a class="json-syntax-object-id" href="${options.object_id_href_prefix + id_itself}" target="_blank">${id_itself}</a>"`;
        }
        else
            return '<span class="' + cls + '">' + match + '</span>';
    });
}

// ----------------------------------------------------------------------

export function load_css(href) {
    $("head").append('<link rel="stylesheet" type="text/css" href="' + href + '">');
    // const link_element = document.createElement("link");
    // link_element.setAttribute("rel", "stylesheet");
    // link_element.setAttribute("type", "text/css");
    // link_element.setAttribute("href", href);
    // document.getElementsByTagName("head")[0].appendChild(link_element);
}

// ----------------------------------------------------------------------

export function join_collapse(args, separator=" ") {
    return args ? args.filter(arg => arg !== null && arg !== undefined && arg !== "").map(arg => "" + arg).filter(arg => arg !== "").join(separator) : "";
}

// ----------------------------------------------------------------------

export function format(template, args) {
    return template.replace(/\${([^}]+)}/g, (match, key) => {
        const replacement = args[key];
        return replacement === undefined ? match : replacement;
    });
}

// ----------------------------------------------------------------------

export function forward_event(evt, ...callbacks) {
    evt.stopPropagation();
    evt.preventDefault();
    callbacks.map(callback => callback(evt));
}

// ----------------------------------------------------------------------

export function arrays_equal_simple(a1, a2) {
    if (a1 === a2)
        return true;
    if (a1 == null || a2 == null || a1.length !== a2.length)
        return false;
    for (let i = 0; i < a1.length; ++i) {
        if (a1[i] !== a2[i])
            return false;
    }
    return true;
}

// ----------------------------------------------------------------------

const whocc_lab_name_fix = {MELB: "VIDRL", NIMR: "Crick"};

export function whocc_lab_name(name) {
    return whocc_lab_name_fix[name] || name;
}

// ----------------------------------------------------------------------

export function array_of_indexes(size, base=0) {
    return Array.apply(null, {length: size}).map((_,index) => index + base);
}

// ----------------------------------------------------------------------

export function ace_antigen_full_name(antigen, options={}) {
    const antigen_clades = (antigen.c && antigen.c.length > 0) ? "<" + antigen.c.join(" ") + ">" : null;
    const result = join_collapse([antigen.N, antigen.R].concat(antigen.a, antigen.P, antigen.D && "[" + antigen.D + "]", antigen_clades));
    return options.escape ? escape_html(result) : result;
}

// ----------------------------------------------------------------------

export function ace_serum_full_name(serum, options={}) {
    const result = join_collapse([serum.N, serum.R].concat(serum.a, serum.I));
    return options.escape ? escape_html(result) : result;
}

// ----------------------------------------------------------------------

export function antigen_serum_abbreviated_name(name, options={}) {
    const vt = text => text.match(/^(A\(H[0-9]N[0-9]\).*|B)/) ? null : text;
    const species = text => text.substr(0, 2);
    const loc = text => {
        const words = text.split(" ");
        return words.length === 1 ? capitalize(text.substr(0, 2)) : words.map(w => w.charAt(0)).join("").toUpperCase();
    };
    const year = text => text.length === 4 ? (options.exclude_year ? null : text.substr(2, 2)) : text;
    const fields = name.split("/");
    switch (fields.length) {
    case 4:
        return join_collapse([vt(fields[0]), loc(fields[1]), fields[2], year(fields[3])], "/");
    case 5:
        return join_collapse([vt(fields[0]), species(fields[1]), loc(fields[2]), fields[3], year(fields[4])], "/");
    default:
        return name;
    }
}

// ----------------------------------------------------------------------

export function ace_titer(chart, ag_no, sr_no) {
    if (chart.t.l) {
        return chart.t.l[ag_no][sr_no];
    }
    else {
        const tt = chart.t.d[ag_no]["" + sr_no];
        return tt === undefined ? "*" : tt;
    }
}

// ----------------------------------------------------------------------

export function capitalize(text) {
    return text.charAt(0).toUpperCase() + text.slice(1).toLowerCase();
}

// ----------------------------------------------------------------------

const escape_html_map = {'&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#039;'};

export function escape_html(text) {
    return text.replace(/[&<>"']/g, m => escape_html_map[m]);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
