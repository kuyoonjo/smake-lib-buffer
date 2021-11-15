const { LLVM } = require('smake');
const { LibBuffer } = require('./lib');

const test = new LLVM('test', 'x86_64-linux-gnu');
test.files = ['test/test.cc'];
LibBuffer.config(test);

module.exports = [test];
