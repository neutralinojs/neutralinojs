const assert = require('assert');

const runner = require('./runner');

describe('debug.spec: debug namespace tests', () => {
    describe('debug.log', () => {
        it('logs a simple message', async () => {
            let exitCode = runner.run(`
                await Neutralino.debug.log('Hello! This is a simple message');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            assert.ok(runner.getOutput().includes('Hello! This is a simple message'));
        });

        it('logs an error message', async () => {
            let exitCode = runner.run(`
                await Neutralino.debug.log('Hello! This is an error message', 'ERROR');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            assert.ok(runner.getOutput().includes('Hello! This is an error message'));
        });
    });
});
