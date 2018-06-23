/*|--minAjax.js--|
  |--(A Minimalistic Pure JavaScript Header for Ajax POST/GET Request )--|
  |--Author : flouthoc (gunnerar7@gmail.com)(http://github.com/flouthoc)--|
  |--Contributers : Add Your Name Below--|
  */
 function initXMLhttp() {
    var xmlhttp;
    if (window.XMLHttpRequest) {
        //code for IE7,firefox chrome and above
        xmlhttp = new XMLHttpRequest();
    } else {
        //code for Internet Explorer
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
    }

    return xmlhttp;
}

function ajax(config) {

    /*Config Structure
            url:"reqesting URL"
            type:"GET or POST"
            method: "(OPTIONAL) True for async and False for Non-async | By default its Async"
            debugLog: "(OPTIONAL)To display Debug Logs | By default it is false"
            data: "(OPTIONAL) another Nested Object which should contains reqested Properties in form of Object Properties"
            success: "(OPTIONAL) Callback function to process after response | function(data,status)"
    */

    if (!config.url) {

        if (config.debugLog == true)
            console.log("No Url!");
        return;

    }

    if (!config.type) {

        if (config.debugLog == true)
            console.log("No Default type (GET/POST) given!");
        return;

    }

    if (!config.method) {
        config.method = true;
    }


    if (!config.debugLog) {
        config.debugLog = false;
    }

    var xmlhttp = initXMLhttp();

    xmlhttp.onreadystatechange = function() {

        if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {

            if (config.success) {
                config.success(JSON.parse(xmlhttp.responseText), xmlhttp.readyState);
            }

            if (config.debugLog == true)
                console.log("SuccessResponse");
            if (config.debugLog == true)
                console.log("Response Data:" + xmlhttp.responseText);

        } else if(xmlhttp.readyState == 4) {

            if (config.debugLog == true)
                console.log("FailureResponse --> State:" + xmlhttp.readyState + "Status:" + xmlhttp.status);
          
            if(config.errorCallback){
                config.errorCallback();
            }
        }
    }
   
   if(typeof config.data != 'undefined')
        sendString = JSON.stringify(config.data);

    if (config.type == "GET") {
        xmlhttp.open("GET", config.url , config.method);
        xmlhttp.send();

        if (config.debugLog == true)
            console.log("GET fired at:" + config.url + "?" + sendString);
    }
    if (config.type == "POST") {
        xmlhttp.open("POST", config.url, config.method);
        xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        xmlhttp.send(sendString);

        if (config.debugLog == true)
            console.log("POST fired at:" + config.url + " || Data:" + sendString);
    }




}

module.exports = {
    ajax : ajax
}