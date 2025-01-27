let allServicesModal = document.getElementById("allServicesModal");
let allServicesTable = document.getElementById("allServicesTable");
let allServicesLoading = document.getElementById("allServicesLoading");
let allServicesClose = document.getElementById("allServicesClose");

document.getElementById("showAllServices").addEventListener("click", function() {

    allServicesModal.style.display = "flex";
    allServicesModal.children[0].style.opacity = 1;
    allServicesLoading.style.display = "flex";
  
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/services.html", true);
    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            allServicesTable.innerHTML = xhr.responseText;
            allServicesTable.style.display = "block";
            allServicesLoading.style.display = "none";
        }
    };
    xhr.send();
    
});

allServicesClose.addEventListener("click", function() {
    allServicesModal.style.display = "none";
    allServicesTable.style.display = "none";
    allServicesLoading.style.display = "none";
});

allServicesModal.addEventListener("click", function(e) {
    if (e.target == allServicesModal) {
        allServicesModal.style.display = "none";
        allServicesTable.style.display = "none";
        allServicesLoading.style.display = "none";
    }
});