
window.myApp = {
    showInfo: () => {
        document.getElementById('info').innerHTML = NL_APPID + " is running on port " +
                    NL_PORT + " inside " + NL_OS + ".<br/><br/>" + "<span>v" + NL_VERSION + "</span>";
    },
    openDocs: () => {
        Neutralino.app.open({
            url: "https://neutralino.js.org/docs"
        });
    }
};

Neutralino.init();
window.myApp.showInfo();
