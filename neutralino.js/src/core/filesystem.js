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

let $ = require('../lib/minAjax.js');

let createDirectory = function (dirName, s, e) {
    $.ajax({
        url : '/filesystem/createDirectory',
        type : 'POST',
        data : {
          dir : dirName
        },
        done : function(data){
            s(data);
        },
        problem : function (error) {
            e(error);
        }
    
    });

};


let removeDirectory = function (dirName, s, e) {
    $.ajax({
        url : '/filesystem/removeDirectory',
        type : 'POST',
        data : {
          dir : dirName
        },
        done : function(data){
            s(data);
        },
        problem : function(error) {
            e(error);
        }
    
    });

};

let writeFile = function (fileName, content, s, e) {
    $.ajax({
        url : '/filesystem/writeFile',
        type : 'POST',
        data : {
          filename : fileName,
          content : content
        },
        done : function(data){
            s(data);
        },
        problem : function(error) {
            e(error);
        }
    
    });

};

let readFile = function (fileName, s, e) {
    $.ajax({
        url : '/filesystem/readFile',
        type : 'POST',
        data : {
          filename : fileName
        },
        done : function(data){
            s(data);
        },
        problem : function(error) {
            e(error);
        }
    
    });

};


let removeFile = function (fileName, s, e) {
    $.ajax({
        url : '/filesystem/removeFile',
        type : 'POST',
        data : {
          filename : fileName
        },
        done : function(data){
            s(data);
        },
        problem : function(error) {
            e(error);
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