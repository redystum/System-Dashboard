let servicesModal = document.getElementById("servicesModal");
let servicesModalBody = document.getElementById("servicesModalBody");
let servicesModalLoading = document.getElementById("servicesModalLoading");
let servicesModalClose = document.getElementById("servicesModalClose");

document.getElementById("showAllServices").addEventListener("click", function() {

    servicesModalLoading.style.display = "flex";
    servicesModalBody.style.display = "none";
    servicesModal.style.display = "flex";
    servicesModal.children[0].style.opacity = 1;
  
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/services.html", true);
    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            servicesModalBody.innerHTML = xhr.responseText;
            servicesModalBody.style.display = "block";
            servicesModalLoading.style.display = "none";
        }
    };
    xhr.send();
    
});

servicesModalClose.addEventListener("click", function() {
    servicesModal.style.display = "none";
    servicesModalBody.style.display = "none";
    servicesModalLoading.style.display = "none";
});

servicesModal.addEventListener("click", function(e) {
    if (e.target == servicesModal) {
        servicesModal.style.display = "none";
        servicesModalBody.style.display = "none";
        servicesModalLoading.style.display = "none";
    }
});

function addRelevantService(service) {
    let xhr = new XMLHttpRequest();
    xhr.open("POST", "/addRelevantService", true);
    xhr.setRequestHeader("Content-Type", "text/plain;charset=UTF-8");
    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            if (xhr.responseText == "OK") {
                document.getElementById("btn"+service).style.display = "none";
                document.getElementById("success"+service).style.display = "block";
            }
        } else if (xhr.readyState == 4 && xhr.status != 200) {
            document.getElementById("error"+service).style.display = "block";
        }
    };
    xhr.send(service);
}

function viewService(service){
    servicesModalLoading.style.display = "flex";
    servicesModalBody.style.display = "none";
    servicesModal.style.display = "flex";
    servicesModal.children[0].style.opacity = 1;
  
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/service.html?id=" + service, true);
    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            servicesModalBody.innerHTML = xhr.responseText;
            servicesModalBody.style.display = "block";
            servicesModalLoading.style.display = "none";
        }
    };
    xhr.send();
}

function removeFromRelevant(service){
    let xhr = new XMLHttpRequest();
    xhr.open("POST", "/removeRelevantService", true);
    xhr.setRequestHeader("Content-Type", "text/plain;charset=UTF-8");
    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            if (xhr.responseText == "OK") {
                document.getElementById("removeFromRelevant").style.display = "none";
                document.getElementById("removeFromRelevantSuccess").style.display = "block";
            }
        } else if (xhr.readyState == 4 && xhr.status != 200) {
            document.getElementById("removeFromRelevantError").style.display = "block";
        }
    };
    xhr.send(service);
}