# ASAN assert_fail

The `asan_assert_fail_qt` library can be preloaded to any application compiled with
ASAN support enabled to get fancy backtraces for assertion failures:

## Example Usage

Let's take the following code and compile it with ASAN:

```text
$ cat test-asan.cpp
#include <qglobal.h>

int main(int argc, char **argv)
{
    Q_ASSERT(argc == 2);
    return 0;
}
$ g++ -fsanitize=address -fPIC -lQt5Core -I/usr/include/qt -I/usr/include/qt/QtCore test-asan.cpp -o test-asan
```

When you run this as-is, the ASAN crash handler ASAN will not kick in, so all
you are left with is the normal assertion output:

```text
$ ./test-asan
0.000 fatal: unknown[test_qt.cpp:5]: ASSERT: "argc == 2" in file test_qt.cpp, line 5
Aborted (core dumped)
```

When we add our little `libasan_assert_fail_qt.so` utility, we do get a nice backtrace
instead. We have to also explicitly preload `libasan.so` as it has to come first:

```text
$ LD_PRELOAD="$(readlink -f /usr/lib/libasan.so) $(readlink -f path/to/libasan_assert_fail_qt.so)" ./test-asan
0.000 warning: unknown[unknown:0]: ASSERT: "argc == 2" in file test_qt.cpp, line 5
=================================================================
==247141==ERROR: AddressSanitizer: unknown-crash on address 0x000000000000 at pc 0x7ffc24e13ec0 bp 0x7ffc24e13e40 sp 0x7ffc24e13e40
READ of size 1 at 0x000000000000 thread T0
    #0 0x7ffc24e13ebf  ([stack]+0x1eebf)
    #1 0x7fa8e75bad0f in __asan::ErrorDescription::Print() /build/gcc/src/gcc/libsanitizer/asan/asan_errors.h:420
    #2 0x7fa8e75bad0f in __asan::ScopedInErrorReport::~ScopedInErrorReport() /build/gcc/src/gcc/libsanitizer/asan/asan_report.cc:140
    #3 0x7fa8e75ba668 in __asan::ReportGenericError(unsigned long, unsigned long, unsigned long, unsigned long, bool, unsigned long, unsigned int, bool) /build/gcc/src/gcc/libsanitizer/asan/asan_report.cc:458
    #4 0x7fa8e75ba773 in __asan_report_error /build/gcc/src/gcc/libsanitizer/asan/asan_report.cc:473
    #5 0x7fa8e749d1a6 in qt_assert(char const*, char const*, int) (/home/milian/projects/kdab/KDToolBox/build/qt/asan_assert_fail_qt/libasan_assert_fail_qt.so.1.0.0+0x11a6)
    #6 0x556cb2b43715 in main (/tmp/a.out+0x715)
    #7 0x7fa8e6a1d022 in __libc_start_main (/usr/lib/libc.so.6+0x27022)
    #8 0x556cb2b4361d in _start (/tmp/a.out+0x61d)
```
