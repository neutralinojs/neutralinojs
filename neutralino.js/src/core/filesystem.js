let $ = require('../lib/minAjax.js');

let createDirectory = (dirName, s, e) => {
    $.ajax({
        url : '/filesystem/createDirectory',
        type : 'POST',
        data : {
          dir : dirName
        },
        success : function(data){
            s(data);
        },
        errorCallback : () => {
            e();
        }
    
    });

};


let removeDirectory = (dirName, s, e) => {
    $.ajax({
        url : '/filesystem/removeDirectory',
        type : 'POST',
        data : {
          dir : dirName
        },
        success : function(data){
            s(data);
        },
        errorCallback : () => {
            e();
        }
    
    });

};

let writeFile = (fileName, content, s, e) => {
    $.ajax({
        url : '/filesystem/writeFile',
        type : 'POST',
        data : {
          filename : fileName,
          content : content
        },
        success : function(data){
            s(data);
        },
        errorCallback : () => {
            e();
        }
    
    });

};

let readFile = (fileName, s, e) => {
    $.ajax({
        url : '/filesystem/readFile',
        type : 'POST',
        data : {
          filename : fileName
        },
        success : function(data){
            s(data);
        },
        errorCallback : () => {
            e();
        }
    
    });

};


let removeFile = (fileName, s, e) => {
    $.ajax({
        url : '/filesystem/removeFile',
        type : 'POST',
        data : {
          filename : fileName
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
    createDirectory : createDirectory,
    removeDirectory : removeDirectory,
    writeFile : writeFile,
    readFile : readFile,
    removeFile : removeFile
}