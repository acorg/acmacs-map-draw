// use <img> (safari) or <object> (chrome) to show pdf files
// e.g. <div class="pdf" src="image.pdf"></div>

const isSafari = /^((?!chrome|android).)*safari/i.test(navigator.userAgent);

document.addEventListener('DOMContentLoaded', function () {
    document.querySelectorAll(".pdf[src]").forEach(element => {
        const src = element.getAttribute("src");
        if (isSafari)
            element.innerHTML = `<img src="${src}">`;
        else
            element.innerHTML = `<object data="${src}#toolbar=0"></object>`;
    });
}, false);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
