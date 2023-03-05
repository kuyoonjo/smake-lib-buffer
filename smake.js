const { LLVM } = require('@smake/llvm');
const { vscode } = require('@smake/llvm-vscode');
const { LibBuffer } = require('./lib');

const test = new LLVM('test', 'x86_64-linux-gnu');
test.files = ['test/test.cc'];
vscode(test);
LibBuffer.config(test);

module.exports = [test];
