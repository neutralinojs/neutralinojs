const assert = require('assert');

const runner = require('./runner');

describe('events.spec: events namespace tests', () => {

    describe('events.on', () => {
        it('sets an event without throwing errors', async () => {
            let exitCode = runner.run(`
                function onTestEvent() {}
                await Neutralino.events.on('testEvent', onTestEvent);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('events.off', () => {
        it('unsets an event without throwing errors', async () => {
            let exitCode = runner.run(`
                function onTestEvent() {}
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.off('testEvent', onTestEvent);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('events.dispatch', () => {
        it('triggers the callback when the event is dispatched', async () => {
            let exitCode = runner.run(`
                function onTestEvent() {
                    __close('done');
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.dispatch('testEvent');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('events.dispatch', () => {
        it('triggers the callback with data', async () => {
            let exitCode = runner.run(`
                function onTestEvent(evt) {
                    __close(evt.detail);
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.dispatch('testEvent', 'data');
            `);
            assert.equal(runner.getOutput(), 'data');
        });
    });

    describe('events.broadcast', () => {
        it('triggers the registered event callback', async () => {
            let exitCode = runner.run(`
                function onTestEvent(evt) {
                    __close('done');
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.broadcast('testEvent');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('triggers the registered event callback with data', async () => {
            let exitCode = runner.run(`
                function onTestEvent(evt) {
                    __close(evt.detail);
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.broadcast('testEvent', 'data');
            `);
            assert.equal(runner.getOutput(), 'data');
        });

        it('throws an error for missing params', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.events.broadcast();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });
    });
});
