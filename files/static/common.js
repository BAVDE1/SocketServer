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
