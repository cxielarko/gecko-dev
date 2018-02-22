setJitCompilerOption("ion.warmup.trigger", 0);
setJitCompilerOption("baseline.warmup.trigger", 0);

// Check for assertion failure in visitTypeOfV
function foo(x) { return typeof x; }
assertEq("bigint", foo(1n));
assertEq("bigint", foo(1n));
