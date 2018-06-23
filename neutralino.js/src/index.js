let filesystem = require('./core/filesystem');
let settings = require('./core/settings');
let os = require('./core/os');
let computer = require('./core/computer');
let init = require('./core/init');

module.exports =  {
    filesystem : filesystem,
    settings : settings,
    os : os,
    computer : computer,
    init : init
}