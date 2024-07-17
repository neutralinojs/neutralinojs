const assert = require('assert');
const runner = require('./runner');

describe('app.spec: app namespace tests', () => {
    describe('app.exit', () => {
            
        it('works without parameters', async () => {
            let exitCode = runner.run(`
                setTimeout(() => {
                    Neutralino.app.exit();
                }, 2000);
            `);
            assert.ok(typeof exitCode != undefined);
        });

        it('works with parameters', async () => {
            let exitCode = runner.run(`
                setTimeout(() => {
                    Neutralino.app.exit(1);
                }, 2000);
            `);
            assert.ok(typeof exitCode != undefined);
        });
        
        it('throws an error for invalid exit codes', async () => {
            runner.run(`
                try {
                    await Neutralino.app.exit('invalid');
                } catch(err) {
                    await __close(err.code);
                }
            `);
            assert.strictEqual(runner.getOutput(), 'NE_RT_NATRTER');
        });
    });

    describe('app.killProcess', () => {
        it('closes the app immediately', async () => {
            let exitCode = runner.run(`
                setTimeout(() => {
                    Neutralino.app.killProcess();
                }, 2000);
            `);
            assert.ok(exitCode != 0);
        });
    });

    describe('app.getConfig', () => {
        it('JSON object contains the right fields', async () => {
            runner.run(`
                let config = await Neutralino.app.getConfig();
                await __close(JSON.stringify(config));
            `);
            const config = JSON.parse(runner.getOutput());
            assert.ok(typeof config === 'object', 'Expected config to be an object');
            assert.ok(typeof config.applicationId === 'string', 'Expected applicationId to be a string');
            assert.ok(typeof config.cli === 'object', 'Expected cli to be an object');
            assert.ok(typeof config.defaultMode === 'string', 'Expected defaultMode to be a string');
            assert.ok(typeof config.documentRoot === 'string', 'Expected documentRoot to be a string');
            assert.ok(Array.isArray(config.extensions), 'Expected extensions to be an object');
            assert.ok(typeof config.globalVariables === 'object', 'Expected globalVariables to be an object');
            assert.ok(typeof config.logging === 'object', 'Expected logging to be an object');
            assert.ok(typeof config.modes === 'object', 'Expected modes to be an object');
            assert.ok(Number.isInteger(config.port), 'Expected port to be an integer');
            assert.ok(typeof config.serverHeaders === 'object', 'Expected serverHeaders to be an object');
            assert.ok(typeof config.url === 'string', 'Expected url to be a string');
            assert.ok(typeof config.version === 'string', 'Expected version to be a string');
        });
    });

    describe('app.broadcast', () => {
        it('triggers the registered event callback', async () => {
            let exitCode = runner.run(`
                function onTestEvent(evt) {
                    __close('done');
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.app.broadcast('testEvent');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('triggers the registered event callback with data', async () => {
            let exitCode = runner.run(`
                function onTestEvent(evt) {
                    __close(evt.detail);
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.app.broadcast('testEvent', 'data');
            `);
            assert.equal(runner.getOutput(), 'data');
        });

        it('throws an error for missing params', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.app.broadcast();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });

        it('throws an error for an invalid event name', async () => {
            runner.run(`
                try {
                    await Neutralino.app.broadcast(12345); 
                } catch(err) {
                    await __close(err.code);
                }
        `   );
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER'); 
        });

        it('broadcasts an event with no listeners registered', async () => {
            runner.run(`
                try {
                    await Neutralino.app.broadcast('eventWithNoListener');
                    await __close('success');
                }
                catch(err) {
                    await __close('failed');
                }
        `   );
            assert.equal(runner.getOutput(), 'success');
        });

        it('handles high load by broadcasting multiple events rapidly', async () => {
            runner.run(`
                try {
                    for (let i = 0; i < 1000; i++) {
                        Neutralino.app.broadcast('testEvent' + i);
                    }
                    await __close('success');
                } catch (err) {
                await __close('failed');
            }
            `);
            assert.equal(runner.getOutput(), 'success');
        });

        it('triggers all registered listeners when broadcasting an event', async () => {
            runner.run(`
                let listener1Triggered = false;
                let listener2Triggered = false;
            
                function listener1() {
                    listener1Triggered = true;
                }
                
                function listener2() {
                    listener2Triggered = true;
                    __close(JSON.stringify({ listener1Triggered, listener2Triggered }));
                }
            
                await Neutralino.events.on('multiListenerEvent', listener1);
                await Neutralino.events.on('multiListenerEvent', listener2);
            
                await Neutralino.app.broadcast('multiListenerEvent');
            `);

            const output = JSON.parse(runner.getOutput());
            const bothListenersTriggered = output.listener1Triggered && output.listener2Triggered
            assert.strictEqual(bothListenersTriggered, true, 'Both listerners should be triggered');
        });

        it('handles errors in listeners gracefully', async () => {
            runner.run(`
                let safeListenerCalled = false;
            
                function errorListener() {
                    throw new Error('Test error');
                }
            
                function safeListener() {
                    safeListenerCalled = true;
                    __close(JSON.stringify({safeListenerCalled}));
                }
            
                await Neutralino.events.on('errorEvent', errorListener);
                await Neutralino.events.on('errorEvent', safeListener);
            
                await Neutralino.app.broadcast('errorEvent');
            `);
            const output = JSON.parse(runner.getOutput())
            assert.strictEqual(output.safeListenerCalled, true, 'Safe listener should be called');
        });
    
        it('executes listeners in the order they were added', async () => {
            runner.run(`
                let order = [];
            
                function listener1() {
                    order.push(1);
                }
            
                function listener2() {
                    order.push(2);
                    __close(JSON.stringify({order}));
                }
            
                await Neutralino.events.on('orderEvent', listener1);
                await Neutralino.events.on('orderEvent', listener2);
            
                await Neutralino.app.broadcast('orderEvent');
            `);
            const output = JSON.parse(runner.getOutput());
            assert.strictEqual(output.order.length, 2);
            assert.strictEqual(output.order[0], 1);
            assert.strictEqual(output.order[1], 2);
        });    
    });
});
