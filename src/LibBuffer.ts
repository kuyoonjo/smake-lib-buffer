import { resolve } from 'path';
import { LLVM } from '@smake/llvm';

export abstract class LibBuffer {
  static config(llvm: LLVM) {
    llvm.includedirs = [
      ...llvm.includedirs,
      resolve(__dirname, '..', 'include').replace(/\\/g, '/'),
    ];
  }
}
