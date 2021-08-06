import { buffer } from '../src/buffer';
import { LLVM_Darwin } from 'smake';
import { addLibs } from '@smake/libs';
import { resolve } from 'path';

test('buffer', () => {
  class A extends LLVM_Darwin {
    files = [];
  }
  const B = addLibs(A, buffer, buffer, buffer);
  const b = new B();
  const p = resolve(__dirname, '..', 'include').replace(/\\/g, '/');
  expect(b.sysIncludedirs.includes(p)).toBe(true);
  expect(b.sysIncludedirs.length).toBe(3);
});
