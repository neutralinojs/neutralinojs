const assert = require('assert');

const runner = require('./runner');

describe('config.spec: App configuration tests', () => {

    it('extends the default user agent', async () => {
        runner.run(`
            await __close(navigator.userAgent);
        `, {args: '--window-extend-user-agent-with="TestUserAgentValue"'});

        assert.ok(runner.getOutput().includes('TestUserAgentValue'));
    });

});
