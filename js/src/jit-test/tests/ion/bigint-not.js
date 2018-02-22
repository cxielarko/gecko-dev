setJitCompilerOption("ion.warmup.trigger", 0);
setJitCompilerOption("baseline.warmup.trigger", 0);

// Check for assertion failure in testValueTruthyKernel
function foo(x) { return !x; }
assertEq(false, foo(1n));
assertEq(false, foo(1n));
