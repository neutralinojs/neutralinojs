let $ = require('../lib/minAjax.js');

let runCommand = (cmd, s, e) => {
    $.ajax({
        url : '/os/runCommand',
        type : 'POST',
        data : {
          command : cmd
        },
        success : function(data){
            s(data);
        },
        errorCallback : () => {
            e();
        }
    
    });

};

let getEnvar = (v, s, e) => {
    $.ajax({
        url : '/os/getEnvar',
        type : 'POST',
        data : {
          name : v
        },
        success : function(data){
            s(data);
        },
        errorCallback : () => {
            e();
        }
    
    });

};


module.exports = {
    runCommand : runCommand,
    getEnvar : getEnvar
}