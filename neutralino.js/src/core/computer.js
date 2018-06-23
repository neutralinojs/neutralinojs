let $ = require('../lib/minAjax.js');

let getRamUsage = (s, e) => {
    $.ajax({
        url : '/computer/getRamUsage',
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
    getRamUsage : getRamUsage
}