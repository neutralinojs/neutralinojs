const assert = require('assert');

const runner = require('./runner');

describe('debug.spec: debug namespace tests', () => {
    describe('debug.log', () => {
        it('logs a simple message', async () => {
            runner.run(`
                await Neutralino.debug.log('Hello! This is a simple message');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            assert.ok(runner.getOutput().includes('Hello! This is a simple message'));
        });

        it('logs an info message', async () => {
            runner.run(`
                await Neutralino.debug.log('Hello! This is a info message', 'INFO');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            assert.ok(runner.getOutput().includes('Hello! This is a info message'));
        });

        it('logs an error message', async () => {
            runner.run(`
                await Neutralino.debug.log('Hello! This is an error message', 'ERROR');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            assert.ok(runner.getOutput().includes('Hello! This is an error message'));
        });

        it('logs a warning message', async () => {
            runner.run(`
                await Neutralino.debug.log('Hello! This is a warning message', 'WARNING');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            assert.ok(runner.getOutput().includes('Hello! This is a warning message'));
        });

        it('logs a debug message', async () => {
            runner.run(`
                await Neutralino.debug.log('Hello! This is a debug message', 'DEBUG');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            assert.ok(runner.getOutput().includes('Hello! This is a debug message'));
        });

        it('logs a message with special characters', async () => {
            runner.run(`
                await Neutralino.debug.log('Special characters: @#$%^&*☁☀☊☄');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            assert.ok(runner.getOutput().includes('Special characters: @#$%^&*☁☀☊☄'));
        });

        it('handles empty log messages gracefully', async () => {
            runner.run(`
                await Neutralino.debug.log('');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            assert.ok(runner.getOutput().includes(''));
        });

        it('logs a large message', async () => {
            const largeLogMessage = 'Neutralino'.repeat(10000); 
            runner.run(`
                await Neutralino.debug.log('${largeLogMessage}');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            assert.ok(runner.getOutput().includes(largeLogMessage));
        });
    });
});
