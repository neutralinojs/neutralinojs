const assert = require('assert');
const fs = require('fs');
const runner = require('./runner');

const CONFIG_PATH = '../bin/test-scopes.config.json';
const BASE_CONFIG = JSON.parse(fs.readFileSync('../bin/neutralino.config.json', 'utf8'));

function runWithConfig(code, scopes) {
    let testConfig = JSON.parse(JSON.stringify(BASE_CONFIG));
    if (scopes !== undefined) {
        testConfig.filesystem = { scopes: scopes };
    }
    fs.writeFileSync(CONFIG_PATH, JSON.stringify(testConfig, null, 2));
    runner.run(code, { args: '--config-file=/test-scopes.config.json' });
    try {
        fs.unlinkSync(CONFIG_PATH);
    } catch(err) {}
}

describe('scopes.spec: filesystem scopes security tests', () => {

    it('allows all operations when filesystem.scopes is not defined', async () => {
        runWithConfig(`
            try {
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test_nodef.txt', 'hello');
                await __close('done');
            } catch(e) {
                await __close(e.code);
            }
        `);
        assert.equal(runner.getOutput(), 'done');
    });

    it('blocks everything when scopes is an empty array', async () => {
        // When scopes is [], even the test runner's internal __close() function 
        // will fail to write to .tmp/output.txt. Thus, the output will be empty.
        runWithConfig(`
            try {
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test_empty.txt', 'hello');
                await __close('done');
            } catch(e) {
                await __close(e.code);
            }
        `, []);
        assert.equal(runner.getOutput(), '');
    });

    describe('with active scopes', () => {
        const testScopes = ["${NL_PATH}/.tmp"];

        it('allows access inside the configured scope', async () => {
            runWithConfig(`
                try {
                    await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test_allowed.txt', 'hello');
                    await __close('done');
                } catch(e) {
                    await __close(e.code);
                }
            `, testScopes);
            assert.equal(runner.getOutput(), 'done');
        });

        it('blocks access outside the configured scope', async () => {
            runWithConfig(`
                try {
                    await Neutralino.filesystem.writeFile(NL_PATH + '/test_outside.txt', 'hello');
                    await __close('NO_ERROR_THROWN');
                } catch(e) {
                    await __close(e.code);
                }
            `, testScopes);
            assert.equal(runner.getOutput(), 'NE_FS_SCOPERR');
        });

        it('blocks prefix spoofing attacks', async () => {
            runWithConfig(`
                try {
                    // Attempt to access a directory that shares the string prefix
                    // but is NOT a subcomponent. E.g. .tmp-malicious instead of .tmp
                    await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp-malicious');
                    await __close('NO_ERROR_THROWN');
                } catch(e) {
                    await __close(e.code);
                }
            `, testScopes);
            assert.equal(runner.getOutput(), 'NE_FS_SCOPERR');
        });

        it('resolves relative path components securely', async () => {
            runWithConfig(`
                try {
                    // This path lexically starts with .tmp but traverses out using ..
                    await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/../test_relative.txt', 'hello');
                    await __close('NO_ERROR_THROWN');
                } catch(e) {
                    await __close(e.code);
                }
            `, testScopes);
            assert.equal(runner.getOutput(), 'NE_FS_SCOPERR');
        });

        it('handles copy operations requiring both source and destination to be allowed', async () => {
            runWithConfig(`
                try {
                    await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/src.txt', 'hello');
                    // .tmp is allowed, but the destination is outside
                    await Neutralino.filesystem.copy(NL_PATH + '/.tmp/src.txt', NL_PATH + '/dest.txt');
                    await __close('NO_ERROR_THROWN');
                } catch(e) {
                    await __close(e.code);
                }
            `, testScopes);
            assert.equal(runner.getOutput(), 'NE_FS_SCOPERR');
        });
    });

});
