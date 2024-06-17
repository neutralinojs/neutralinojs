const assert = require('assert');

const runner = require('./runner');

describe('events.spec: events namespace tests', () => {

    describe('events.on', () => {
        it('sets an event without throwing errors', async () => {
            runner.run(`
                function onTestEvent() {}
                await Neutralino.events.on('testEvent', onTestEvent);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('triggers the event handler when an event is dispatched', async () => {
            runner.run(`
                function onTestEvent() {
                    __close('done');
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.dispatch('testEvent');
            `);
            assert.strictEqual(runner.getOutput(), 'done');
        });

        it('handles multiple events', async () => {    
            runner.run(`
                let isEvent1Triggered = false;
                let isEvent2Triggered = false;
                function onTestEvent1() {
                    isEvent1Triggered = true;
                }
                function onTestEvent2() {
                    isEvent2Triggered = true;
                    __close(JSON.stringify({isEvent1Triggered, isEvent2Triggered}));
                }
                await Neutralino.events.on('testEvent1', onTestEvent1);
                await Neutralino.events.on('testEvent2', onTestEvent2);
                await Neutralino.events.dispatch('testEvent1');
                await Neutralino.events.dispatch('testEvent2');
            `);
            const output = JSON.parse(runner.getOutput())
            bothEventsTriggered = output.isEvent1Triggered && output.isEvent2Triggered
            assert.strictEqual(bothEventsTriggered, true);
        });

        it('ensures event handlers are not duplicated', async () => {
            runner.run(`
                let callCount = 0;
                function onTestEvent() {
                    callCount++;
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.on('testEvent', onTestEvent); 
                await Neutralino.events.dispatch('testEvent');
                setTimeout(async () => {
                    await __close(JSON.stringify(callCount));
                }, 1000);
            `);
            const output = JSON.parse(runner.getOutput())
            assert.strictEqual(output, 1);
        });
    });

    describe('events.off', () => {
        it('unsets an event without throwing errors', async () => {
            runner.run(`
                function onTestEvent() {}
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.off('testEvent', onTestEvent);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('does not trigger the event handler after it is unset', async () => {
            runner.run(`
                let eventTriggered = false;
                function onTestEvent() {
                    eventTriggered = true;
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.off('testEvent', onTestEvent);
                await Neutralino.events.dispatch('testEvent');
                setTimeout(async () => {
                    await __close(JSON.stringify(eventTriggered));
                }, 1000);
            `);
            output = JSON.parse(runner.getOutput())
            assert.strictEqual(output, false);
        });

        it('does nothing if trying to unset a handler that was not set', async () => {
            runner.run(`
                let eventTriggered = false;
                function onTestEvent() {
                    eventTriggered = true;
                }
                await Neutralino.events.off('testEvent', onTestEvent);
                await Neutralino.events.dispatch('testEvent');
                setTimeout(async () => {
                    await __close(JSON.stringify(eventTriggered));
                }, 1000);
            `);
            output = JSON.parse(runner.getOutput())
            assert.strictEqual(output, false);
        });

        it('can unset multiple handlers for the same event', async () => {
            runner.run(`
                let isEvent1Triggered = false;
                let isEvent2Triggered = false;
    
                function onTestEvent1() {
                    isEvent1Triggered = true;
                }
                function onTestEvent2() {
                    isEvent2Triggered = true;
                }
    
                await Neutralino.events.on('testEvent', onTestEvent1);
                await Neutralino.events.on('testEvent', onTestEvent2);
                await Neutralino.events.off('testEvent', onTestEvent1);
                await Neutralino.events.off('testEvent', onTestEvent2);
                await Neutralino.events.dispatch('testEvent');
                setTimeout(async () => {
                    await __close(JSON.stringify({ isEvent1Triggered, isEvent2Triggered }));
                }, 1000);
            `);
            const output = JSON.parse(runner.getOutput());
            const bothEventsNotTriggered = output.isEvent1Triggered && output.isEvent2Triggered;
            assert.strictEqual(bothEventsNotTriggered, false);
        });
    });

    describe('events.dispatch', () => {
        it('triggers the callback when the event is dispatched', async () => {
            runner.run(`
                function onTestEvent() {
                    __close('done');
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.dispatch('testEvent');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('triggers the callback with data', async () => {
            runner.run(`
                function onTestEvent(evt) {
                    __close(evt.detail);
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.dispatch('testEvent', 'data');
            `);
            assert.equal(runner.getOutput(), 'data');
        });

        it('does not throw an error when no event listeners are registered', async () => {
            runner.run(`
                await Neutralino.events.dispatch('testEvent');
                setTimeout(async () => {
                    await __close('done');
                }, 1000);
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('triggers multiple callbacks for the same event', async () => {
            runner.run(`
                let isEvent1Triggered = false;
                let isEvent2Triggered = false;
                function onTestEvent1() {
                    isEvent1Triggered = true;
                }
                function onTestEvent2() {
                    isEvent2Triggered = true;
                    __close(JSON.stringify({isEvent1Triggered, isEvent2Triggered}));
                }
                await Neutralino.events.on('testEvent', onTestEvent1);
                await Neutralino.events.on('testEvent', onTestEvent2);
                await Neutralino.events.dispatch('testEvent');
            `);
            const output = JSON.parse(runner.getOutput());
            const bothEventsNotTriggered = output.isEvent1Triggered && output.isEvent2Triggered;
            assert.strictEqual(bothEventsNotTriggered, true);
        });

        it('does not trigger the callback after it is removed', async () => {
            runner.run(`
                function onTestEvent() {
                    __close('triggered');
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.off('testEvent', onTestEvent);
                await Neutralino.events.dispatch('testEvent');
                await __close('not triggered');
            `);
            assert.equal(runner.getOutput(), 'not triggered');
        });

        it('triggers the callback multiple times for multiple dispatches', async () => {
            runner.run(`
                let callCount = 0;
                function onTestEvent() {
                    callCount++;
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.dispatch('testEvent');
                await Neutralino.events.dispatch('testEvent');
                setTimeout(async () => {
                    await __close(JSON.stringify(callCount));
                }, 1000);               
            `);
            const output = JSON.parse(runner.getOutput());
            assert.strictEqual(output, 2);
        });

        it('passes multiple data arguments to the callback', async () => {
            runner.run(`
                function onTestEvent(evt) {
                    __close(JSON.stringify([evt.detail.arg1, evt.detail.arg2]));
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.dispatch('testEvent', { arg1: 'data1', arg2: 'data2' });
            `);
            const result = JSON.parse(runner.getOutput());
            assert.deepEqual(result, ['data1', 'data2']);
        });

        it('handles errors in the callback gracefully', async () => {
            runner.run(`
                function onTestEvent() {
                    throw new Error('Test error');
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.dispatch('testEvent');
                setTimeout(async () => {
                    await __close('done');
                }, 1000);
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('continues to trigger remaining callbacks if one of them throws an error', async () => {
            runner.run(`
                let isEvent2Triggered = false;
                function onTestEvent1() {
                    throw new Error('Test error');
                }
                function onTestEvent2() {
                    isEvent2Triggered = true;
                    __close(JSON.stringify(isEvent2Triggered));
                }
                await Neutralino.events.on('testEvent', onTestEvent1);
                await Neutralino.events.on('testEvent', onTestEvent2);
                await Neutralino.events.dispatch('testEvent');
            `);
            const output = JSON.parse(runner.getOutput());
            assert.strictEqual(output, true);
        });


    });

    describe('events.broadcast', () => {
        it('triggers the registered event callback', async () => {
            runner.run(`
                function onTestEvent(evt) {
                    __close('done');
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.broadcast('testEvent');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('triggers the registered event callback with data', async () => {
            runner.run(`
                function onTestEvent(evt) {
                    __close(evt.detail);
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.broadcast('testEvent', 'data');
            `);
            assert.equal(runner.getOutput(), 'data');
        });

        it('throws an error for missing params', async () => {
            runner.run(`
                try {
                    await Neutralino.events.broadcast();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });

        it('does not throw an error when no event listeners are registered', async () => {
            runner.run(`
                await Neutralino.events.broadcast('testEvent');
                setTimeout(async () => {
                    await __close('done');
                }, 1000);
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('triggers multiple callbacks for the same event', async () => {
            runner.run(`
                isEvent1Triggered = false;
                isEvent2Triggered = false;
                function onTestEvent1() {
                    isEvent1Triggered = true;
                }
                function onTestEvent2() {
                    isEvent2Triggered = true;
                    __close(JSON.stringify({isEvent1Triggered, isEvent2Triggered}));
                }
                await Neutralino.events.on('testEvent', onTestEvent1);
                await Neutralino.events.on('testEvent', onTestEvent2);
                await Neutralino.events.broadcast('testEvent');
            `);
            const output = JSON.parse(runner.getOutput());
            const bothEventsNotTriggered = output.isEvent1Triggered && output.isEvent2Triggered;
            assert.strictEqual(bothEventsNotTriggered, true);
        });

        it('does not trigger the callback after it is removed', async () => {
            runner.run(`
                function onTestEvent() {
                    __close('triggered');
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.off('testEvent', onTestEvent);
                await Neutralino.events.broadcast('testEvent');
                setTimeout(async () => {
                    await __close('not triggered');
                }, 1000);
            `);
            assert.equal(runner.getOutput(), 'not triggered');
        });

        it('triggers the callback multiple times for multiple broadcasts', async () => {
            runner.run(`
                let callCount = 0;
                function onTestEvent() {
                    callCount++;
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.broadcast('testEvent');
                await Neutralino.events.broadcast('testEvent');
                setTimeout(async () => {
                    await __close(JSON.stringify(callCount));
                }, 1000);    
            `);
            const output = JSON.parse(runner.getOutput())
            assert.strictEqual(output, 2);
        });

        it('passes multiple data arguments to the callback', async () => {
            runner.run(`
                function onTestEvent(evt) {
                    __close(JSON.stringify([evt.detail.arg1, evt.detail.arg2]));
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.broadcast('testEvent', { arg1: 'data1', arg2: 'data2' });
            `);
            const result = JSON.parse(runner.getOutput());
            assert.deepEqual(result, ['data1', 'data2']);
        });

        it('handles errors in the callback gracefully', async () => {
            runner.run(`
                function onTestEvent() {
                    throw new Error('Test error');
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.events.broadcast('testEvent');
                setTimeout(async () => {
                    await __close('done');
                }, 1000);
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('continues to trigger remaining callbacks if one throws an error', async () => {
            runner.run(`
                let isEvent2Triggered = false;
                function onTestEvent1(evt) {
                    throw new Error('Test error');
                }
                function onTestEvent2(evt) {
                    isEvent2Triggered = true;
                    __close(JSON.stringify(isEvent2Triggered));
                }
                await Neutralino.events.on('testEvent', onTestEvent1);
                await Neutralino.events.on('testEvent', onTestEvent2);
                await Neutralino.events.broadcast('testEvent');
            `);
            const output = JSON.parse(runner.getOutput());
            assert.strictEqual(output, true);
        });         
    });
});
