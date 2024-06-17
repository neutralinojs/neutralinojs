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
            let stats = JSON.parse(o);
            assert.ok(typeof stats == 'object');
            assert.ok(Array.isArray(stats.loaded));
            assert.ok(stats.loaded.length > 0);
            assert.ok(stats.loaded.find((extension) => extension == 'js.neutralino.sampleextension'));
            assert.ok(Array.isArray(stats.connected));
            assert.ok(stats.loaded.length > 0);
            assert.ok(stats.connected.find((extension) => extension == 'js.neutralino.sampleextension'));
        });

        it('returns empty stats when no extensions are loaded', async () => {
            runner.run(`
                let stats = await Neutralino.extensions.getStats();
                await __close(JSON.stringify(stats));
            `, {args: '--enable-extensions=false'});  
            let o = runner.getOutput();
            let stats = JSON.parse(o);
            assert.ok(typeof stats == 'object');
            assert.ok(Array.isArray(stats.loaded));
            assert.ok(stats.loaded.length == 0);
            assert.ok(Array.isArray(stats.connected));
            assert.ok(stats.connected.length == 0);
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

        it('dispatches event with complex data', async () => {
            runner.run(`
                const complexData = { key1: 'value1', key2: 2, key3: [1, 2, 3] };
                await Neutralino.extensions.dispatch('js.neutralino.sampleextension', 'testEvent', complexData);
                await __close('done');
            `, {args: '--enable-extensions'});
            assert.equal(runner.getOutput(), 'done');
        });

        it('handles dispatch with empty event name gracefully', async () => {
            runner.run(`
                await Neutralino.extensions.dispatch('js.neutralino.sampleextension', '', 'data');
                await __close('done');
            `, {args: '--enable-extensions'});
            assert.equal(runner.getOutput(), 'done');
        });

        it('handles dispatch with invalid extension id gracefully', async () => {
            runner.run(`
                try {
                    await Neutralino.extensions.dispatch('invalid.extension.id', 'testEvent', 'data');
                    await __close('done');
                } catch (error) {
                    await __close(error.code);
                }
            `, {args: '--enable-extensions'});
            assert.equal(runner.getOutput(), 'NE_EX_EXTNOTL');
        });

        it('handles dispatch with large data payload', async () => {
            runner.run(`
                const largeData = 'N'.repeat(1024 * 1024); 
                await Neutralino.extensions.dispatch('js.neutralino.sampleextension', 'testEvent', largeData);
                await __close('done');
            `, {args: '--enable-extensions'});
            assert.equal(runner.getOutput(), 'done');
        });

        it('dispatches multiple events concurrently', async () => {
            runner.run(`
                const dispatches = [
                    Neutralino.extensions.dispatch('js.neutralino.sampleextension', 'firstEvent', 'data1'),
                    Neutralino.extensions.dispatch('js.neutralino.sampleextension', 'secondEvent', 'data2')
                ];
                await Promise.all(dispatches);
                await __close('done');
            `, {args: '--enable-extensions'});
            assert.equal(runner.getOutput(), 'done');
        });

        it('throws an error when the extensions are not enabled', async () => {
            runner.run(`
                try {
                    await Neutralino.extensions.dispatch('js.neutralino.notconnectedextension', 'testEvent', 'data');
                    await __close('done');
                } catch (error) {
                    await __close(error.code);
                }
            `, {args: '--enable-extensions=false'});
            assert.equal(runner.getOutput(), 'NE_EX_EXTNOTL');
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

        it('handles complex data', async () => {
            runner.run(``,
            { beforeInitCode: `
                Neutralino.events.on("extensionReady", async () => {
                    const complexData = { key1: 'value1', key2: 2, key3: [1, 2, 3] };
                    await Neutralino.extensions.broadcast('testEvent', complexData);
                    await __close('done');
                });
            `, args: '--enable-extensions'});
            assert.equal(runner.getOutput(), 'done');
        });  

        it('broadcasts multiple events concurrently', async () => {
            runner.run(``,
            { beforeInitCode: `
                Neutralino.events.on("extensionReady", async () => {
                    const broadcasts = [
                        Neutralino.extensions.broadcast('firstEvent', 'data1'),
                        Neutralino.extensions.broadcast('secondEvent', 'data2')
                    ];
                    await Promise.all(broadcasts);
                    await __close('done');
                });
            `, args: '--enable-extensions'});
            assert.equal(runner.getOutput(), 'done');
        });
    });
    }
});
