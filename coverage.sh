dir=.smake/test/debug/aarch64-apple-darwin/
bin=/opt/homebrew/opt/llvm/bin

LLVM_PROFILE_FILE="$dir/test.profraw" $dir/test
$bin/llvm-profdata merge -sparse $dir/test.profraw -o $dir/test.profdata
# $bin/llvm-cov show --ignore-filename-regex=doctest $dir/test -instr-profile=$dir/test.profdata
# $bin/llvm-cov report --ignore-filename-regex=doctest $dir/test -instr-profile=$dir/test.profdata
$bin/llvm-cov export -format=lcov --ignore-filename-regex=doctest $dir/test -instr-profile=$dir/test.profdata > $dir/test.lcov.info
genhtml $dir/test.lcov.info -o coverage
