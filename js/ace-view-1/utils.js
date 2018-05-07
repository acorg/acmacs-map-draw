// ----------------------------------------------------------------------

// https://stackoverflow.com/questions/4810841/how-can-i-pretty-print-json-using-javascript
export function json_syntax_highlight(data, options={object_id_href_prefix: "doc/"}) {
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
    const link_element = document.createElement("link");
    link_element.setAttribute("rel", "stylesheet");
    link_element.setAttribute("type", "text/css");
    link_element.setAttribute("href", href);
    document.getElementsByTagName("head")[0].appendChild(link_element);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
