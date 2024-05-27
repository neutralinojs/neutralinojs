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
                await Neutralino.debug.log('Hello! This is an info message', 'INFO');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            let logFileContent = runner.getOutput();
            assert.ok(logFileContent.includes('Hello! This is an info message'), 'The log file should contain the log message');
            assert.ok(logFileContent.includes('INFO'), 'The log message should be INFO level');
        });

        it('logs an error message', async () => {
            runner.run(`
                await Neutralino.debug.log('Hello! This is an error message', 'ERROR');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            let logFileContent = runner.getOutput();
            assert.ok(logFileContent.includes('Hello! This is an error message', 'The log file should contain the log message'));
            assert.ok(logFileContent.includes('ERROR'), 'The log message should be ERROR level');
        });

        it('logs a warning message', async () => {
            runner.run(`
                await Neutralino.debug.log('Hello! This is a warning message', 'WARNING');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            let logFileContent = runner.getOutput();
            assert.ok(logFileContent.includes('Hello! This is a warning message', 'The log file should contain the log message'));
            assert.ok(logFileContent.includes('WARNING'), 'The log message should be WARNING level');
        });

        it('logs a debug message', async () => {
            runner.run(`
                await Neutralino.debug.log('Hello! This is a debug message', 'DEBUG');
                let logFileContent = await Neutralino.filesystem.readFile(NL_PATH + '/neutralinojs.log');
                await __close(logFileContent);
            `);
            let logFileContent = runner.getOutput();
            assert.ok(logFileContent.includes('Hello! This is a debug message', 'The log file should contain the log message'));
            assert.ok(logFileContent.includes('DEBUG'), 'The log message should be DEBUG level');
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
