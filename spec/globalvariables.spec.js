const assert = require('assert');
const path = require('path');
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

    it('handles an unexpected NL_ARGS value', async () => {
        runner.run(`
            await __close(NL_ARGS.toString());
        `, {args: '--unknown-arg'});
        assert.ok(runner.getOutput().includes('--unknown-arg'));
    });

    it('handles NL_PID when not running in a process context', async () => {
        runner.run(`
            if (typeof NL_PID === 'undefined') {
                await __close('undefined');
            } else {
                await __close(NL_PID.toString());
            }
        `);
        let output = runner.getOutput();
        assert.ok(output === 'undefined' || parseInt(output) > 0);
    });

    it('handles NL_RESMODE with an unexpected value', async () => {
        runner.run(`
            NL_RESMODE = 'unexpected_mode';
    
            await __close(NL_RESMODE);
        `);
    
        assert.notEqual(runner.getOutput(), 'directory', 'NL_RESMODE should not be set to the default value "directory" with an unexpected input');
    });

    it('handles NL_EXTENABLED with various command-line arguments', async () => {
        runner.run(`
            await __close(NL_EXTENABLED.toString());
        `, {args: '--enable-extensions --another-arg'});
        let extStatus = JSON.parse(runner.getOutput());
        assert.ok(typeof extStatus == 'boolean');
        assert.equal(extStatus, true);

        runner.run(`
            await __close(NL_EXTENABLED.toString());
        `, {args: '--disable-extensions'});
        extStatus = JSON.parse(runner.getOutput());
        assert.ok(typeof extStatus == 'boolean');
        assert.equal(extStatus, false);
    });

    it('tests the mutability of NL_OS', async () => {
        runner.run(`
            NL_OS = 'Darwin';

            await __close(NL_OS);`);
        
        assert.equal(runner.getOutput(), 'Darwin');
    });

    it('handles changing custom global variables', async () => {
        runner.run(`
            const originalTest1 = NL_TEST1;
            const originalTest2 = JSON.stringify(NL_TEST2);
            const originalTest3 = JSON.stringify(NL_TEST3);

            NL_TEST1 = 'ChangedValue';
            NL_TEST2.push(6);
            NL_TEST3.value3 = 'Added';

            await __close(
                JSON.stringify({
                    NL_TEST1,
                    NL_TEST2: JSON.stringify(NL_TEST2),
                    NL_TEST3: JSON.stringify(NL_TEST3)
                })
            );
        `);

        const modifiedGlobals = JSON.parse(runner.getOutput());

        assert.equal(modifiedGlobals.NL_TEST1, 'ChangedValue');
        assert.deepEqual(JSON.parse(modifiedGlobals.NL_TEST2), [2, 4, 5, 6]);
        assert.deepEqual(JSON.parse(modifiedGlobals.NL_TEST3), {
            value1: 10,
            value2: {},
            value3: 'Added'
        });
    });

    it('verifies NL_PATH conforms to expected file path format', async () => {
        runner.run(`
            await __close(NL_PATH.toString());
        `);
    
        const filePath = runner.getOutput();
    
        const isRelativePath = !path.isAbsolute(filePath);

        const normalizedPath = path.normalize(filePath);
    
        const isValidPathStructure = normalizedPath === filePath || normalizedPath.endsWith(path.basename(normalizedPath));
    
        assert.ok(isRelativePath && isValidPathStructure, `NL_PATH "${filePath}" does not conform to the expected relative file path format.`);

    });

    it('should be able to access NL_OS in the global scope', () => {
        runner.run(`
            await __close(NL_OS);
        `);
        assert.equal(typeof runner.getOutput(), 'string');
    });
    
    it('should be able to access NL_OS within a function', async () => {
        runner.run(`
            async function testFn() {
                await __close(typeof NL_OS);
            }
            await testFn(); 
        `);
    
        assert.equal(runner.getOutput(), 'string', 'NL_OS should be a string when accessed within the function');
    });

    it('should be able to access NL_OS within a nested function', async () => {
        runner.run(`
            async function outerFn() {
                async function innerFn() {
                    await __close(typeof NL_OS);
                }
                await innerFn();
            }
            outerFn();
        `);
        
        assert.equal(runner.getOutput(), 'string', 'NL_OS should be defined as a string in the nested function');
    });

    it('should not persist unintended modifications across tests', async () => {
        runner.run(`
            NL_TEST1 = 'TempValue';
            await __close(NL_TEST1);
        `);
            
        assert.equal(runner.getOutput(), 'TempValue');
            
        runner.run(`
            await __close(NL_TEST1);
        `);

        assert.equal(runner.getOutput(), 'Hello');  
    });
});
