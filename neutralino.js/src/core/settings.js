let $ = require('../lib/minAjax.js');

let getSettings = (s,e) => {
    $.ajax({
        url : '/settings.json',
        type : 'GET',
        success : function(data){
            s(data);
        },
        errorCallback : () => {
            e();
        }
    
    });

};


module.exports = {
    getSettings : getSettings
}