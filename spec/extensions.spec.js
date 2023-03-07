const assert = require('assert');

const runner = require('./runner');

describe('extensions.spec: extensions namespace tests', () => {
    if(process.env.GITHUB_ACTIONS) { 
    describe('extensions.getStats, extensions.dispatch, extensions.broadcast', () => {
        it('exports functions to the app', async () => {
            runner.run(`
                let out = [
                    typeof Neutralino.extensions.getStats,
                    typeof Neutralino.extensions.dispatch,
                    typeof Neutralino.extensions.broadcast,
                ];
                await __close(JSON.stringify(out));
            `);
            let out = JSON.parse(runner.getOutput());
            assert.equal(out[0], 'function');
            assert.equal(out[1], 'function');
            assert.equal(out[2], 'function');
        });
    });
    }
    else {
    describe('extensions.getStats', () => {
        it('returns extensions stats', async () => {
            runner.run(``,
            { beforeInitCode: `
                Neutralino.events.on("extensionReady", async () => {
                    let stats = await Neutralino.extensions.getStats();
                    await __close(JSON.stringify(stats));
                });
            `, args: '--enable-extensions'});
            let o = runner.getOutput();
            console.log('outout',o);
            let stats = JSON.parse(o);
            assert.ok(typeof stats == 'object');
            assert.ok(Array.isArray(stats.loaded));
            assert.ok(stats.loaded.length > 0);
            assert.ok(stats.loaded.find((extension) => extension == 'js.neutralino.sampleextension'));
            assert.ok(Array.isArray(stats.connected));
            assert.ok(stats.loaded.length > 0);
            assert.ok(stats.connected.find((extension) => extension == 'js.neutralino.sampleextension'));
        });
    });

    describe('extensions.dispatch', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.extensions.dispatch('js.neutralino.sampleextension', 'testEvent', 'data');
                await __close('done');
            `, {args: '--enable-extensions'});
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('extensions.broadcast', () => {
        it('works without throwing errors', async () => {
            runner.run(``,
            { beforeInitCode: `
                Neutralino.events.on("extensionReady", async () => {
                    await Neutralino.extensions.broadcast('testEvent', 'data');
                    await __close('done');
                });
            `, args: '--enable-extensions'});
            assert.equal(runner.getOutput(), 'done');
        });
    });
    }
});
