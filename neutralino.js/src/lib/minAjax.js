// MIT License

// Copyright (c) 2018 Neutralinojs

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

 let authbasic = require('../auth/authbasic');

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

    if (!config.method) {
        config.method = true;
    }

    var xmlhttp = initXMLhttp();

    xmlhttp.onreadystatechange = function() {

        if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {

            if (config.success) {
                config.success(JSON.parse(xmlhttp.responseText), xmlhttp.readyState);
            }

        } 
        else if(xmlhttp.readyState == 4) {

            if(config.errorCallback){
                config.errorCallback();
            }
        }
    }
    
   if(typeof config.data != 'undefined')
        sendString = JSON.stringify(config.data);

    if (config.type == "GET") {
        xmlhttp.open("GET", config.url , config.method);
        xmlhttp.setRequestHeader("Authorization", "Basic " + authbasic.getToken());
        xmlhttp.send();
    }
    
    if (config.type == "POST") {
        xmlhttp.open("POST", config.url, config.method);
        xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        xmlhttp.setRequestHeader("Authorization", "Basic " + authbasic.getToken());
        xmlhttp.send(sendString);
    }



}

module.exports = {
    ajax : ajax
}