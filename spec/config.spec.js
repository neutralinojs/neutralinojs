const assert = require('assert');

const runner = require('./runner');

describe('config.spec: App configuration tests', () => {

    it('extends the default user agent', async () => {
        runner.run(`
            await __close(navigator.userAgent);
        `, {args: '--window-extend-user-agent-with="TestUserAgentValue"'});

        assert.ok(runner.getOutput().includes('TestUserAgentValue'));
    });

    it('does not crash when a non-numeric value is passed to an integer CLI arg', async () => {
        runner.run(`
            await __close(NL_PORT.toString());
        `, {args: '--port=notanumber'});

        let output = runner.getOutput();
        assert.ok(output.length > 0, 'App crashed — no output was written');
        assert.ok(!isNaN(parseInt(output)), 'NL_PORT is not a valid number after skipped override');
    });

    it('does not crash when an out-of-range value is passed to an integer CLI arg', async () => {
        runner.run(`
            await __close(NL_PORT.toString());
        `, {args: '--port=99999999999'});

        let output = runner.getOutput();
        assert.ok(output.length > 0, 'App crashed — no output was written');
        assert.ok(!isNaN(parseInt(output)), 'NL_PORT is not a valid number after skipped override');
    });

    it('does not crash when an integer CLI arg has trailing non-numeric characters', async () => {
        runner.run(`
            await __close(NL_PORT.toString());
        `, {args: '--port=42abc'});

        let output = runner.getOutput();
        assert.ok(output.length > 0, 'App crashed — no output was written');
        assert.ok(!isNaN(parseInt(output)), 'NL_PORT is not a valid number after skipped override');
    });

    it('does not apply an invalid integer override but still applies valid overrides in the same run', async () => {
        runner.run(`
            await __close(navigator.userAgent);
        `, {args: '--port=notanumber --window-extend-user-agent-with="PartialOverrideCheck"'});

        let output = runner.getOutput();
        assert.ok(output.length > 0, 'App crashed — no output was written');
        assert.ok(output.includes('PartialOverrideCheck'),
            'String override was not applied after skipping invalid int override');
    });

});
