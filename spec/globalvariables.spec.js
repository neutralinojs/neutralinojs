const assert = require('assert');

const runner = require('./runner');

describe('globalvariables.spec: Global variables relatedtests', () => {

    it('loads basic internal global variables to the window scope', async () => {
        runner.run(`
            let globals = [];
            for(let key in window) {
                if(key.includes('NL_')) {
                    globals.push(key);
                }
            }
            await __close(JSON.stringify(globals));
        `);

        let globals = JSON.parse(runner.getOutput());
        assert.ok(Array.isArray(globals));

        let checkGlobals = [
            'OS', 'VERSION', 'COMMIT', 'APPID', 'TOKEN', 'CWD', 'PATH', 'CMETHODS'
        ];

        for(let key of checkGlobals) {
            assert.ok(globals.includes('NL_' + key));
        }
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

    it('includes resource mode of the app in NL_RESMODE', async () => {
        runner.run(`
            await __close(NL_RESMODE.toString());
        `);
        assert.equal(runner.getOutput(), 'directory');
    });

    it('includes the extension loader status in NL_EXTENABLED', async () => {
        runner.run(`
            await __close(NL_EXTENABLED.toString());
        `, {args: '--enable-extensions'});

        let extStatus = JSON.parse(runner.getOutput());
        assert.ok(typeof extStatus == 'boolean');
        assert.equal(extStatus, true);
    });

    it('exports custom global variables', async () => {
        runner.run(`
            await __close(
                JSON.stringify({
                    NL_TEST1,
                    NL_TEST2,
                    NL_TEST3
                })
            );
        `);

        let customGlobals = JSON.parse(runner.getOutput());
        assert.ok(typeof customGlobals == 'object');

        // Refer neutralino.config.json's globalVariables prop.
        assert.ok(typeof customGlobals.NL_TEST1 == 'string');
        assert.equal(customGlobals.NL_TEST1, 'Hello');

        assert.ok(Array.isArray(customGlobals.NL_TEST2));
        assert.deepEqual(customGlobals.NL_TEST2, [2, 4, 5]);

        assert.ok(typeof customGlobals.NL_TEST3 == 'object');
        assert.deepEqual(customGlobals.NL_TEST3, {
            value1: 10,
            value2: {}
        });
    });

    it('sets NL_PORT properly', async () => {
        runner.run(`
            await __close(NL_PORT.toString());
        `, {args: '--port=53999'});
        assert.ok(runner.getOutput().includes('53999'));
    });

});
