const Mocha = require('mocha');
const fs = require('fs');
const path = require('path');

let mocha = new Mocha();
let testDir = '.';
let specModule = process.argv.length > 2 ? process.argv[2] : '';

fs.readdirSync(testDir).filter((file) => file.includes(specModule + '.spec.js'))
.forEach((file) => {
      mocha.addFile(path.join(testDir, file));

});
mocha.timeout(20000);
mocha.run((failures) => {
    process.exitCode = failures ? 1 : 0;
});
