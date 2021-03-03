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

let runCommand = function(cmd, s, e) {
    $.ajax({
        url : '/os/runCommand',
        type : 'POST',
        data : {
          command : cmd
        },
        done : function(data){
            s(data);
        },
        problem : function (error) {
            e(error);
        }

    });

};

let getEnvar = function(v, s, e) {
    $.ajax({
        url : '/os/getEnvar',
        type : 'POST',
        data : {
          name : v
        },
        done : function(data){
            s(data);
        },
        problem : function(error) {
            e(error);
        }

    });

};


let dialogOpen = function(options, s, e) {
    $.ajax({
        url : '/os/dialogOpen',
        type : 'POST',
        data : {
          title : options["title"],
          isDirectoryMode : options["isDirectoryMode"]
        },
        done : function(data){
            s(data);
        },
        problem : function(error) {
            e(error);
        }

    });

};


let dialogSave = function(t, s, e) {
    $.ajax({
        url : '/os/dialogSave',
        type : 'POST',
        data : {
          title : t
        },
        done : function(data){
            s(data);
        },
        problem : function(error) {
            e(error);
        }

    });

};

let showNotification = function(options, s, e) {
    $.ajax({
        url : '/os/showNotification',
        type : 'POST',
        data : options,
        done : function(data){
            s(data);
        },
        problem : function(error) {
            e(error);
        }

    });

};

let showMessageBox = function(options, s, e) {
    $.ajax({
        url : '/os/showMessageBox',
        type : 'POST',
        data : options,
        done : function(data){
            s(data);
        },
        problem : function(error) {
            e(error);
        }

    });

};

module.exports = {
    runCommand : runCommand,
    getEnvar : getEnvar,
    dialogOpen : dialogOpen,
    dialogSave : dialogSave,
    showNotification: showNotification,
    showMessageBox: showMessageBox
}
