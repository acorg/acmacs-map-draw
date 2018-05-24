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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
