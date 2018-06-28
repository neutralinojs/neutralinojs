let getToken = () => {
    return localStorage.getItem('NL_TOKEN');
}


module.exports = {
    getToken : getToken
}