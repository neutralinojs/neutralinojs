const assert = require('assert');

const runner = require('./runner');

describe('updater.spec: updater namespace tests', () => {

    describe('updater.checkForUpdates', () => {
        it('throws an error for missing params', async () => {
            runner.run(`
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
            runner.run(`
                let info = await Neutralino.updater
                    .checkForUpdates('http://127.0.0.1:8080/updater_test/update_info.json');

                await __close(JSON.stringify(info));
            `, { args: '--port=8080' });
            let info = JSON.parse(runner.getOutput());
            assert.ok(typeof info == 'object');
            assert.ok(typeof info.applicationId == 'string');
            assert.ok(typeof info.version == 'string');
            assert.ok(typeof info.resourcesURL == 'string');
        });

        it('handles network errors gracefully', async () => {
            runner.run(`
                try {
                    await Neutralino.updater
                        .checkForUpdates('http://127.0.0.1:99999/non_existent_file.json');
                }
                catch(error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_UP_CUPDERR');
        });

        it('throws an error for invalid URL format', async () => {
            runner.run(`
                try {
                    await Neutralino.updater
                        .checkForUpdates('invalid-url');
                }
                catch(error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_UP_CUPDERR');
        });

        it('handles multiple concurrent update checks gracefully', async () => {
            runner.run(`
                let updatePromises = [
                    Neutralino.updater.checkForUpdates('http://127.0.0.1:8080/updater_test/update_info.json'),
                    Neutralino.updater.checkForUpdates('http://127.0.0.1:8080/updater_test/update_info.json')
                ];
        
                try {
                    let results = await Promise.all(updatePromises);
                    await __close(JSON.stringify(results));
                }
                catch(error) {
                    await __close(error.code);
                }
            `, { args: '--port=8080' });
            let results = JSON.parse(runner.getOutput());
            assert.ok(Array.isArray(results) && results.length === 2);
        });  
        
        it('throws an error for empty update manifest', async () => {
            runner.run(`
                try {
                    await Neutralino.updater
                        .checkForUpdates('http://127.0.0.1:8080/updater_test/empty_update_info.json');
                }
                catch(error) {
                    await __close(error.code);
                }
            `, { args: '--port=8080' });
            assert.equal(runner.getOutput(), 'NE_UP_CUPDERR');
        });        
    });

    describe('updater.install', () => {
        it('throws an error if no updates fetched', async () => {
            runner.run(`
                try {
                    await Neutralino.updater.install();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_UP_UPDNOUF');
        });

        it('replaces resources.neu with resourcesURL data', async () => {
            runner.run(`
                await Neutralino.updater
                    .checkForUpdates('http://127.0.0.1:8080/updater_test/update_info.json');

                // Try deleting existing resources.neu just for testing
                try {
                    await Neutralino.filesystem.remove(NL_PATH + '/resources.neu');
                }
                catch(err) {
                    // ignore
                }

                await Neutralino.updater.install();
                let stats = await Neutralino.filesystem.getStats(NL_PATH + '/resources.neu');

                if(stats.isFile) {
                    await __close('ok');
                }
            `, { args: '--port=8080' });
            assert.equal(runner.getOutput(), 'ok');
        });
       
        it('throws an error if no update manifest is loaded', async () => {
            runner.run(`
                try {
                    await Neutralino.updater.install();
                }
                catch(error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_UP_UPDNOUF');
        });  

        it('throws an error for invalid file path during installation', async () => {
            runner.run(`
                await Neutralino.updater
                    .checkForUpdates('http://127.0.0.1:8080/updater_test/update_info.json');
        
                try {
                    await Neutralino.filesystem.writeFile(NL_PATH + '/invalid_path/resources.neu', 'data');
                    await Neutralino.updater.install();
                }
                catch(error) {
                    await __close(error.code);
                }
            `, { args: '--port=8080' });
            assert.equal(runner.getOutput(), 'NE_FS_FILWRER');
        });

    });

});
