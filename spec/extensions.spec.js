const assert = require('assert');

const runner = require('./runner');

describe('extensions.spec: extensions namespace tests', () => {

    describe('extensions.getStats', () => {
        it('returns extensions stats', async () => {
            runner.run(`
                setTimeout(async () => {
                    let stats = await Neutralino.extensions.getStats();
                    await __close(JSON.stringify(stats));
                }, 1000);
            `, {args: '--enable-extensions'});

            let stats = JSON.parse(runner.getOutput());
            assert.ok(typeof stats == 'object');
            assert.ok(typeof stats.loaded == 'object');
            assert.ok(stats.loaded.length > 0);
            assert.ok(stats.loaded.find((extension) => extension == 'js.neutralino.sampleextension'));
            assert.ok(typeof stats.connected == 'object');
            assert.ok(stats.loaded.length > 0);
            assert.ok(stats.connected.find((extension) => extension == 'js.neutralino.sampleextension'));
        });
    });

    describe('extensions.dispatch', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                setTimeout(async () => {
                    await Neutralino.extensions.dispatch('js.neutralino.sampleextension', 'testEvent', 'data');
                    await __close('done');
                }, 1000);
            `, {args: '--enable-extensions'});
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('extensions.broadcast', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                setTimeout(async () => {
                    await Neutralino.extensions.broadcast('testEvent', 'data');
                    await __close('done');
                }, 1000);
            `, {args: '--enable-extensions'});
            assert.equal(runner.getOutput(), 'done');
        });
    });
});
