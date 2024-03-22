const { LLVM } = require('@smake/llvm');
const { vscode } = require('@smake/llvm-vscode');
const { LibBuffer } = require('./lib');

const test = new LLVM('test', 'aarch64-apple-darwin');
test.files = ['test/test.cc'];
test.cxflags = [
  ...test.cxflags,
  '-fprofile-instr-generate -fcoverage-mapping',
];
test.ldflags = [
  ...test.ldflags,
  '-fprofile-instr-generate -fcoverage-mapping',
];
vscode(test);
LibBuffer.config(test);

module.exports = [test];
