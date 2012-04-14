Portable Musepack decoder
(C) 1999-2002 Buschmann/Klemm/Piecha/Wolf
(C) 2003-2004 Peter Pawlowski

See LGPL.txt for license/redistribution info.

Usage: see sample.cpp

Features:
- Switchable fixed-point and floating-point modes - enable/disable "#define MPC_FIXED_POINT" in mpc_math.h
- Endian-safe, verified running correctly on big-endian machines
- Multiinstance and multithread safe
- File access callbacks
- No assembly code used, for full portability

Verified correctly compiling/running under:
- win32 / x86 / MSVC6 + SP5 + processor pack - very fast floating-point mode (goes above 200x on ~2GHz machines), fixed-point mode is significantly slower (~60x)
- win32 / x86 / MSVC7.1 - slightly faster than MSVC6, fixed-point mode still relatively slow
- win32 / x86 / DMCPP - fixed-point mode faster than MSVC, floating-point mode slower than expected with strange slowdowns when compiled with speed optimizations enabled
- wince / ARM (32bit) / eVC4 + SP3 - fixed-point decoding speed on 400MHz XScale CPU is about 10x realtime
- Darwin 7.3.0, MacOS X 10.3.3 / PowerPC970/G5 / GCC 3.3 - about 80x-100x decoding speed in both modes on 2GHz G5, thanks to TrNSZ for testing/feedback

Package includes project files for MSVC6 and eVC4.