var path = require('path');

module.exports = {
  entry: {
            index : './src/index.js'
        },
  output: {
    filename: 'neutralino.js',
    path: path.resolve(__dirname, 'dist'),
    library: 'Neutralino',
    libraryTarget: 'var'
  }
};