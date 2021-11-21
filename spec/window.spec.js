const assert = require('assert');

const runner = require('./runner');

describe('window.spec: window namespace tests', () => {

    describe('window.setTitle', () => {
        it('works without parameters', async () => {
            runner.run(`
                await Neutralino.window.setTitle();
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('works with parameters', async () => {
            runner.run(`
                await Neutralino.window.setTitle('New title');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('window.maximize', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.window.maximize();
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('window.isMaximized', () => {
        it('returns a boolean value', async () => {
            runner.run(`
                let value = await Neutralino.window.isMaximized();
                await __close(value.toString());
            `);
            assert.ok(typeof JSON.parse(runner.getOutput()) == 'boolean');
        });
    });

    describe('window.unmaximize', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.window.unmaximize();
                await __close('done');
            `, {args: '--window-maximize'});
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('window.minimize', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.window.minimize();
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('window.setFullScreen', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.window.setFullScreen();
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('window.exitFullScreen', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.window.exitFullScreen();
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('window.isFullScreen', () => {
        it('returns a boolean value', async () => {
            runner.run(`
                let value = await Neutralino.window.isFullScreen();
                await __close(value.toString());
            `);
            assert.ok(typeof JSON.parse(runner.getOutput()) == 'boolean');
        });
    });

    describe('window.show', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.window.show();
                await __close('done');
            `, {args: '--window-hidden'});
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('window.hide', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.window.hide();
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('window.focus', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.window.focus();
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('window.isVisible', () => {
        it('returns a boolean value', async () => {
            runner.run(`
                let value = await Neutralino.window.isVisible();
                await __close(value.toString());
            `);
            assert.ok(typeof JSON.parse(runner.getOutput()) == 'boolean');
        });
    });

    describe('window.setIcon', () => {
        it('sets the icon without throwing errores', async () => {
            runner.run(`
                await Neutralino.window.setIcon('/resources/icons/appIcon.png');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('throws errors for missing params', async () => {
            runner.run(`
                try {
                    await Neutralino.window.setIcon();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });
    });

    describe('window.move', () => {
        it('moves the window without throwing errores', async () => {
            runner.run(`
                await Neutralino.window.move(10, 5);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('throws errors for missing params', async () => {
            runner.run(`
                try {
                    await Neutralino.window.move(10);
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });
    });

    describe('window.setDraggableRegion', () => {
        it('registers draggable region without throwing errores', async () => {
            runner.run(`
                await Neutralino.window.setDraggableRegion(document.body);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('throws errors for missing params', async () => {
            runner.run(`
                try {
                    await Neutralino.window.setDraggableRegion();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_WD_DOMNOTF');
        });
    });

    describe('window.unsetDraggableRegion', () => {
        it('unregisters draggable region without throwing errores', async () => {
            runner.run(`
                await Neutralino.window.setDraggableRegion(document.body);
                await Neutralino.window.unsetDraggableRegion(document.body);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('throws errors for missing params', async () => {
            runner.run(`
                try {
                    await Neutralino.window.setDraggableRegion();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_WD_DOMNOTF');
        });
    });

    describe('window.setSize', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.window.setSize({
                    width: 100,
                    height: 200,
                    minWidth: 20,
                    minHeight: 20,
                    maxWidth: 500,
                    maxHeight: 500,
                    resizable: true
                });
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('doesns\'t throw errors for missing params', async () => {
            runner.run(`
                await Neutralino.window.setSize();
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

});
