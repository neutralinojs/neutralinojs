let myapp = {
    myfunction : () => document.getElementById('info').innerHTML = `${NL_NAME} is running on port ${NL_PORT} inside ${NL_OS}.`
};
    

Neutralino.init({
    load: () => {
        myapp.myfunction();
    }
});