setJitCompilerOption("ion.warmup.trigger", 0);
setJitCompilerOption("baseline.warmup.trigger", 0);

// Check for assertion failure in testValueTruthyKernel
function foo(x) { return x ? 1 : 0; }
assertEq(0, foo(0n));
assertEq(0, foo(0n));
