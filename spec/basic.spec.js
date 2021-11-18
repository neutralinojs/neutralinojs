const assert = require('assert');

const runner = require('./runner');

describe('basic.spec: Basic app tests', () => {
    it('Loads source files and runs native methods', async () => {
        runner.run(`
            await __close('Hello');
        `);
        assert.equal(runner.getOutput(), 'Hello');
    });

    it('Loads basic internal global variables to the window scope', async () => {
        runner.run(`
            let basicGlobals = ['OS', 'PORT', 'TOKEN', 'APPID'];
            let hasGlobals = true;
            for(let item of basicGlobals) {
                if(typeof window['NL_' + item] == 'undefined') {
                    hasGlobals = false;
                    break;
                }
            }
            await __close(hasGlobals.toString());
        `);
        assert.equal(runner.getOutput(), 'true');
    });
});
