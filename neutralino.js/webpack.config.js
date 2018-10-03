var path = require('path');

module.exports = {
  entry: {
            index : './src/index.js'
        },
  console.log(path);
  
  output: {
    filename: 'neutralino.js',
    path: path.resolve(__dirname, 'dist'),
    library: 'Neutralino',
    libraryTarget: 'var'
  }
};
