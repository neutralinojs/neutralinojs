const assert = require('assert');
const path = require('path');

const runner = require('./single_instance_runner');

describe('single_instance.spec: Single-instance launch tests', function() {
    // Process startup can be slower on shared CI runners.
    this.timeout(45000);

    it('forwards all arguments and the working directory', async () => {
        const forwardedArgs = [
            '--custom-action=preview',
            'forwarded path δοκιμή.json',
            'second image.png',
            'sample-app://documents/42?mode=review'
        ];
        const secondaryCwd = path.resolve('..');
        const result = await runner.runSecondInstance(forwardedArgs, {
            cwd: secondaryCwd
        });
        const events = JSON.parse(result.output);
        const event = events[0];

        assert.equal(result.secondaryExitCode, 0);
        assert.equal(events.length, 1);
        assert.ok(Array.isArray(event.args));
        for(const arg of forwardedArgs) assert.ok(event.args.includes(arg));
        assert.equal(path.resolve(event.cwd), secondaryCwd);
        assert.ok(typeof event.requestId == 'string' && event.requestId.length > 0);
    });

    it('notifies the primary for a later launch without user arguments', async () => {
        const result = await runner.runSecondInstance();
        const events = JSON.parse(result.output);
        const event = events[0];

        assert.equal(result.secondaryExitCode, 0);
        assert.equal(events.length, 1);
        assert.ok(Array.isArray(event.args));
        assert.ok(event.args.length > 0);
        assert.equal(path.resolve(event.cwd), path.resolve(result.secondaryCwd));
        assert.ok(typeof event.requestId == 'string' && event.requestId.length > 0);
    });

    it('queues every forwarded event during concurrent cold launches', async () => {
        const result = await runner.runSingleInstanceRace(10);
        const events = JSON.parse(result.output);
        const forwardedMarkers = events.map(event =>
            event.args.find(arg => result.markers.includes(arg)));

        assert.equal(result.exitCodes.length, 10);
        assert.equal(result.exitCodes.filter(code => code === 0).length, 9);
        assert.equal(events.length, 9);
        assert.equal(new Set(forwardedMarkers).size, 9);
        assert.ok(forwardedMarkers.every(marker => marker));
        assert.equal(new Set(events.map(event => event.requestId)).size, 9);
    });

    it('allows a new primary after the previous primary terminates', async () => {
        assert.equal(await runner.runPrimaryTakeover(), true);
    });

    it('fails closed when the primary cannot accept another launch', async () => {
        const result = await runner.runQueueOverflow();

        assert.equal(result.acceptedExitCodes.length, 128);
        assert.ok(result.acceptedExitCodes.every(code => code === 0));
        assert.equal(result.rejectedExitCode, 1);
        assert.equal(result.primaryStillRunning, true);
    });
});
