let myapp = {
    myfunction : () => document.getElementById('info').innerHTML = `${NL_NAME} is running on port ${NL_PORT} inside ${NL_OS}.`
};
    
window.onload = () => {
    Neutralino.init({
        load: () => {
            myapp.myfunction();
        }
    });
}