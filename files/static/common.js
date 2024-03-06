// INCLUDES
$(document).ready(function () {
    $(".include-navbar").load("files/navbar.html");
    $(".include-footer").load("files/footer.html");
});

function AjaxRequest(url, onSuccess) {
    const xhr = new XMLHttpRequest();
    xhr.open("GET", url, true);

    xhr.onload = (event) => {
        if (xhr.readyState === 4) {
            if (xhr.status === 200) {
                onSuccess(xhr.responseText);
            } else {
                console.error("Ajax did not recieve 200: " + xhr.statusText);
            }
        }
    }

    xhr.onerror = (event) => {
        console.error("Failed to send ajax: " + xhr.statusText);
    };

    xhr.send(null);
}


function findFileCommon(data, dateElem, titleElem, loadFileFunc) {
    data = JSON.parse(data);
    var fileId = window.location.href.split("=").slice(-1)[0];
    fileId = fileId.split("#")[0];

    for (let i = 0; i < data.length; i++) {
        d = data[i];
        if (d.id == fileId) {
            // Found matching id, load file
            AjaxRequest(d.path, loadFileFunc);
    
            // last updated
            if (dateElem) {
                dateElem.innerHTML = d.last_updated;
            }
            // title
            if (titleElem) {
                titleElem.innerHTML += " - " + d.id
            }
            return d.id;
        }
    }
}

function loadFileFormatted(data, pageElem) {
    lines = data.split("\r\n")
    firstLine = `<div class="header-main">${lines[0]}</div>`
    pageElem.innerHTML = firstLine;

    for (let i = 1; i < lines.length; i++) {
        if (lines[i]) {
            // normal line
            line = `${lines[i]}<br>`

            // header lines
            if (line.startsWith("# ")) {
                line = `<div class="header-1">${lines[i].substring(2)}</div>`
            }
            if (line.startsWith("## ")) {
                line = `<div class="header-2">${lines[i].substring(3)}</div>`
            }

            // add to page
            pageElem.innerHTML += line;
        }
    }
}

function loadFilePlain(data, pageElem) {
    lines = data.split("\r\n");
    pageElem.innerHTML = lines[0];
    for (let i = 1; i < lines.length; i++) {
        line = "\r\n" + lines[i];
        pageElem.innerHTML += line;
    }
}

function loadFileCommon(data, pageElem, formatted=true) {
    if (pageElem) {
        if (formatted) {
            loadFileFormatted(data, pageElem);
        } else {
            loadFilePlain(data, pageElem);
        }
    }    
}