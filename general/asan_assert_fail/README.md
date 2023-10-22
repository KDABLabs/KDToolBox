# ASAN assert_fail

The `asan_assert_fail` library can be preloaded to any application compiled with
ASAN support enabled to get fancy backtraces for assertion failures.

## Example Usage

Let's take the following code and compile it with ASAN:

```text
$ cat test-asan.cpp
#include <cassert>

int main(int argc, char **argv)
{
    assert(argc == 2);
    return 0;
}
$ g++ -fsanitize=address test-asan.cpp -o test-asan
```

When you run this as-is, the ASAN crash handler ASAN will not kick in, so all
you are left with is the normal assertion output:

```text
$ ./test-asan
test-asan: test.cpp:5: int main(int, char**): Assertion `argc == 2' failed.
Aborted (core dumped)
```

When we add our little `libasan_assert_fail.so` utility, we do get a nice backtrace
instead. We have to also explicitly preload `libasan.so` as it has to come first:

```text
$ LD_PRELOAD="$(readlink -f /usr/lib/libasan.so) $(readlink -f path/to/libasan_assert_fail.so)" ./test-asan
test-asan: test.cpp:5: int main(int, char**): Assertion `argc == 2' failed.
=================================================================
==241773==ERROR: AddressSanitizer: unknown-crash on address 0x000000000000 at pc 0x7ffd5a782050 bp 0x7ffd5a781ff0 sp 0x7ffd5a781ff0
READ of size 1 at 0x000000000000 thread T0
    #0 0x7ffd5a78204f  ([stack]+0x1f04f)
    #1 0x7f61a199cd0f in __asan::ErrorDescription::Print() /build/gcc/src/gcc/libsanitizer/asan/asan_errors.h:420
    #2 0x7f61a199cd0f in __asan::ScopedInErrorReport::~ScopedInErrorReport() /build/gcc/src/gcc/libsanitizer/asan/asan_report.cc:140
    #3 0x7f61a199c668 in __asan::ReportGenericError(unsigned long, unsigned long, unsigned long, unsigned long, bool, unsigned long, unsigned int, bool) /build/gcc/src/gcc/libsanitizer/asan/asan_report.cc:458
    #4 0x7f61a199c773 in __asan_report_error /build/gcc/src/gcc/libsanitizer/asan/asan_report.cc:473
    #5 0x7f61a187ec4c in __assert_fail (/home/milian/projects/kdab/KDToolBox/build/debug/asan_assert_fail/libasan_assert_fail.so.1.0.0+0xc4c)
    #6 0x560b69653981 in main (/tmp/test-asan+0x981)
    #7 0x7f61a133a022 in __libc_start_main (/usr/lib/libc.so.6+0x27022)
    #8 0x560b6965387d in _start (/tmp/test-asan+0x87d)
```
