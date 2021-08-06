const { LLVM_Darwin, LLVM_Linux, LLVM_Win32 } = require('smake');
const { readdirSync } = require('fs');
const { join } = require('path');
const { addLibs } = require('@smake/libs');
const { buffer } = require('./lib');

const items = readdirSync(join(__dirname, 'test'))
  .filter(x => x.endsWith('.cc'))
  .map(x => x.replace(/\.cc$/, ''));

const targets = items.map(item => {
  const darwin_x86_64 = class extends LLVM_Darwin {
    includedirs = [
      ...super.includedirs,
    ];
    cxflags = [
      ...super.cxflags,
      '-std=c++17'
    ];
    files = ['test/' + item + '.cc'];
  };
  Object.defineProperty(darwin_x86_64, 'name', { value: item + '_darwin_x86_64' });

  const linux_arm64 = class extends LLVM_Linux {
    target = 'aarch64-linux-gnu';
    includedirs = [
      ...super.includedirs,
    ];
    ldflags = [
      ...super.ldflags,
      '-pthread',
      '-static-libstdc++',
    ];
    files = ['test/' + item + '.cc'];
  };
  Object.defineProperty(linux_arm64, 'name', { value: item + '_linux_arm64' });

  const win_x86_64 = class extends LLVM_Win32 {
    ARCH = 'x86_64';
    useClangHeaders = true;
    includedirs = [
      ...super.includedirs,
    ];
    cxflags = [
      ...super.cxflags,
      '-D_WIN32_WINNT=0x0A00',
      '-D_CRT_SECURE_NO_WARNINGS',
    ];
    ldflags = [
      ...super.ldflags,
    ];
    files = ['test/' + item + '.cc'];
  };
  Object.defineProperty(win_x86_64, 'name', { value: item + '_win32_x86_64' });
  return [darwin_x86_64, linux_arm64, win_x86_64];
}).reduce((a, b) => a.concat(b), []);

module.exports = {
  targets: targets.map(x => addLibs(x, buffer)),
};