const assert = require('assert');

const runner = require('./runner');

describe('updater.spec: app namespace tests', () => {

    describe('updater.checkForUpdates', () => {
        it('throws an error for missing params', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.updater.checkForUpdates();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });

        it('works with parameters', async () => {
            let exitCode = runner.run(`
                let info = await Neutralino.updater
                    .checkForUpdates('http://localhost:8080/updater_test/update_info.json')

                await __close(JSON.stringify(info));
            `, { args: '--port=8080' });
            let info = JSON.parse(runner.getOutput());
            assert.ok(typeof info == 'object');
            assert.ok(typeof info.applicationId == 'string');
            assert.ok(typeof info.version == 'string');
            assert.ok(typeof info.resourcesURL == 'string');
        });
    });

    describe('updater.install', () => {
        it('throws an error if no updates fetched', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.updater.install();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_UP_UPDNOUF');
        });

        it('replaces res.neu with resourcesURL data', async () => {
            let exitCode = runner.run(`
                await Neutralino.updater
                    .checkForUpdates('http://localhost:8080/updater_test/update_info.json')

                //Try deleting existing res.neu just for testing
                try {
                    await Neutralino.filesystem.removeFile(NL_PATH + '/res.neu');
                }
                catch(err) {
                    // ignore
                }

                await Neutralino.updater.install();
                let stats = await Neutralino.filesystem.getStats(NL_PATH + '/res.neu');

                if(stats.isFile) {
                    await __close('ok');
                }
            `, { args: '--port=8080' });
            assert.equal(runner.getOutput(), 'ok');
        });
    });

});
