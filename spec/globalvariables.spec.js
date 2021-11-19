const assert = require('assert');

const runner = require('./runner');

describe('globalvariables.spec: Global variables relatedtests', () => {

    it('loads basic internal global variables to the window scope', async () => {
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

    it('adds command-line arguments to NL_ARGS', async () => {
        runner.run(`
            await __close(NL_ARGS.toString());
        `, {args: '--test-arg'});
        assert.ok(runner.getOutput().includes('--test-arg'));
    });

    it('updates NL_PID with the process identifier', async () => {
        runner.run(`
            await __close(NL_PID.toString());
        `);
        assert.ok(parseInt(runner.getOutput()) > 0);
    });

});
