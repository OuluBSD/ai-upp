#ifndef _SysInclude_SysInclude_h_
#define _SysInclude_SysInclude_h_

// Source: https://en.cppreference.com/w/cpp/header

// -std=c++17
#define ai_std_version 17
#if __GNUC__
	#ifdef __clang__
		#define HAVE_CLANG 1
	#else
		#define HAVE_GCC 1
	#endif
	#ifdef __MINGW32__
		#define HAVE_MINGW 1
	#endif
#endif
#if __linux
	#define HAVE_LINUX 1
#endif
#if __unix || __unix__ || __APPLE__
	#define HAVE_POSIX 1
#endif
#if defined(__WIN32) || defined(_WIN32) || defined(WIN32)
	#define HAVE_WIN32 1
#endif
#ifdef _MSC_VER
	#define HAVE_WIN32 1
#endif
#ifdef MSYS_ENV // add -DMSYS_ENV manually to build method
	#define HAVE_MSYS 1
#endif

#define HAVE_OPENGL 1
#define HAVE_OPENSSL 1
#define HAVE_SDL2 1

//pcre2.h
#define PCRE2_CODE_UNIT_WIDTH 8

// Multi-purpose headers
#include <cstdlib> //				General purpose utilities: program control, dynamic memory allocation, random numbers, sort and search
#if ai_std_version == 17
	#include <execution> //	(C++17):	Predefined execution policies for parallel versions of the algorithms and execution control components(since C++26)
#endif

// Language support library
#include <cfloat> //					Limits of floating-point types
#include <climits> //					Limits of integral types
#include <csetjmp> //					Macro (and function) that saves (and jumps) to an execution context
#include <csignal> //					Functions and macro constants for signal management
#include <cstdarg> //					Handling of variable length argument lists
#include <cstddef> //					Standard macros and typedefs
#include <exception> //					Exception handling utilities
#include <limits> //					Query properties of arithmetic types
#include <new> //						Low-level memory management utilities
#include <typeinfo> //					Runtime type information utilities
#if ai_std_version == 17
	#include <cstdint> //	(C++11)			Fixed-width integer types and limits of other types
	#include <initializer_list> //(C++11)	std::initializer_list class template
	#include <typeindex> //	(C++11)			std::type_index
#endif
#if ai_std_version == 20
	#include <compare> // (C++20)			Three-way comparison operator support
	#include <coroutine> // (C++20)			Coroutine support library
	#include <source_location> //(C++20)	Supplies means to obtain source code location
	#include <version> // (C++20)			Supplies macros for verifying implementation status of library
#endif
#if ai_std_version == 23
	#include <stdfloat> // (C++23)			Fixed-width floating-point types
#endif

// Concepts library
#if ai_std_version == 20
	#include <concepts> // (C++20)			Fundamental library concepts
#endif

// Diagnostics library
#include <cassert> //					Conditionally compiled macro that compares its argument to zero
#include <cerrno> //					Macro containing the last error number
#include <stdexcept> //					Standard exception types
#if ai_std_version == 11
	#include <system_error> ///(C++11)		Defines std::error_code, a platform-dependent error code
#endif
#if ai_std_version == 23
	#include <stacktrace> //(C++23)			Stacktrace library
#endif
#if ai_std_version == 26
	#include <debugging> //(C++26)			Debugging library
#endif

// Memory management library
#include <memory> //					High-level memory management utilities
#if ai_std_version == 11
	#include <scoped_allocator> //(C++11)	Nested allocator class
#endif
#if ai_std_version == 17
	#include <memory_resource> //(C++17)	Polymorphic allocators and memory resources
#endif

// Metaprogramming library
#include <ratio> //(C++11)				Compile-time rational arithmetic
#include <type_traits> //(C++11)			Compile-time type information utilities

// General utilities library
#include <bitset> //					std::bitset class template
#include <functional> //				Function objects, Function invocations, Bind operations and Reference wrappers
#include <utility> //					Various utility components
#include <tuple> //(C++11)				std::tuple class template
#include <optional> //(C++17)			std::optional class template
#if ai_std_version == 17
	#include <any> //(C++17)				std::any class
	#include <variant> //(C++17)			std::variant class template
#endif
#if ai_std_version == 20
	#include <bit> //(C++20)				Bit manipulation functions
#endif
#if ai_std_version == 23
	#include <expected> //(C++23)			std::expected class template
#endif

// Containers library
#include <deque> //						std::deque container
#include <list> //						std::list container
#include <map> //						std::map and std::multimap associative containers
#include <queue> //						std::queue and std::priority_queue container adaptors
#include <set> //						std::set and std::multiset associative containers
#include <stack> //						std::stack container adaptor
#include <vector> //					std::vector container
#if ai_std_version == 20
	#include <array> //(C++11)				std::array container
	#include <forward_list> //(C++11)		std::forward_list container
	#include <unordered_map> //(C++11)		std::unordered_map and std::unordered_multimap unordered associative containers
	#include <unordered_set> //(C++11)		std::unordered_set and std::unordered_multiset unordered associative containers
#endif
#if ai_std_version == 20
	#include <span> //(C++20)				std::span view
#endif
#if ai_std_version == 23
	#include <flat_map> //(C++23)			std::flat_map and std::flat_multimap container adaptors
	#include <flat_set> //(C++23)			std::flat_set and std::flat_multiset container adaptors
	#include <mdspan> //(C++23)				std::mdspan view
#endif
#if ai_std_version == 26
	#include <inplace_vector> //(C++26)		std::inplace_vector container
#endif

// Iterators library
#include <iterator> //					Range iterators

// Ranges library
#if ai_std_version == 20
	#include <ranges> //(C++20)				Range access, primitives, requirements, utilities and adaptors
#endif
#if ai_std_version == 23
	#include <generator> //(C++23)			std::generator class template
#endif

// Algorithms library
#include <algorithm> //					Algorithms that operate on ranges
#include <numeric> //					Numeric operations on values in ranges

// Strings library
#include <cstring> //					Various narrow character string handling functions
#include <string> //					std::basic_string class template
#include <string_view> //(C++17)		std::basic_string_view class template

// Text processing library
#include <cctype> //					Functions to determine the category of narrow characters
#include <clocale> //					C localization utilities
#include <cwchar> //					Various wide and multibyte string handling functions
#include <cwctype> //					Functions to determine the category of wide characters
#include <locale> //					Localization utilities
#include <codecvt> //(C++11)			(deprecated in C++17)(removed in C++26)	Unicode conversion facilities
#include <cuchar> //(C++11)				C-style Unicode character conversion functions
#include <regex> //(C++11)				Classes, algorithms and iterators to support regular expression processing
#include <charconv> //(C++17)			std::to_chars and std::from_chars
#if ai_std_version == 20
	#include <format> //(C++20)				Formatting library including std::format
#endif
#if ai_std_version == 26
	#include <text_encoding> //(C++26)		Text encoding identifications
#endif

// Numerics library
#include <cmath> //						Common mathematics functions
#include <complex> //					Complex number type
#include <valarray> //					Class for representing and manipulating arrays of values
#include <cfenv> //(C++11)				Floating-point environment access functions
#include <random> //(C++11)				Random number generators and distributions
#if ai_std_version == 20
	#include <numbers> //(C++20)			Math constants
#endif
#if ai_std_version == 26
	#include <linalg> //(C++26)				Basic linear algebra algorithms
#endif

// Time library
#include <ctime> //						C-style time/date utilities
#include <chrono> //(C++11)				C++ time utilities

// Input/output library
#include <cstdio> //					C-style input-output functions
#include <fstream> //					std::basic_fstream, std::basic_ifstream, std::basic_ofstream class templates and typedefs
#include <iomanip> //					Helper functions to control the format of input and output
#include <ios> //						std::ios_base class, std::basic_ios class template and typedefs
#include <iosfwd> //					Forward declarations of all classes in the input/output library
#include <iostream> //					Several standard stream objects
#include <istream> //					std::basic_istream class template and typedefs
#include <ostream> //					std::basic_ostream, std::basic_iostream class templates and typedefs
#include <sstream> //					std::basic_stringstream, std::basic_istringstream, std::basic_ostringstream class templates and typedefs
#include <streambuf> //					std::basic_streambuf class template
//#include <strstream> //					(deprecated in C++98)(removed in C++26)	std::strstream, std::istrstream, std::ostrstream
#include <cinttypes> //(C++11)			Formatting macros, intmax_t and uintmax_t math and conversions
#include <filesystem> //(C++17)			std::filesystem::path class and supporting functions
#if ai_std_version == 20
	#include <syncstream> //(C++20)			std::basic_osyncstream, std::basic_syncbuf and typedefs
#endif
#if ai_std_version == 23
	#include <print> // (C++23)				Formatted output library including std::print
	#include <spanstream> //(C++23)			std::basic_spanstream, std::basic_ispanstream, std::basic_ospanstream class templates and typedefs
#endif

// Concurrency support library
#include <atomic> // (C++11)			Atomic operations library
#include <condition_variable> //(C++11)	Thread waiting conditions
#include <future> //(C++11)				Primitives for asynchronous computations
#include <mutex> //(C++11)				Mutual exclusion primitives
#include <thread> //(C++11)				td::thread class and supporting functions
#include <shared_mutex> //(C++14)		Shared mutual exclusion primitives
#if ai_std_version == 20
	#include <barrier> // (C++20)			Barriers
	#include <latch> //(C++20)				Latches
	#include <semaphore> //(C++20)			Semaphores
	#include <stop_token> //(C++20)			Stop tokens for std::jthread
#endif
#if ai_std_version == 26
	#include <hazard_pointer> //(C++26)		Hazard pointers
	#include <rcu> //(C++26)				Read-copy update mechanisms
#endif
/* C compatibility headers


For some of the C standard library headers of the form xxx.h, the C++ standard library both
includes an identically-named header and another header of the form cxxx (all meaningful cxxx
headers are listed above). The intended use of headers of form xxx.h is for interoperability
only. It is possible that C++ source files need to include one of these headers in order to be
valid ISO C. Source files that are not intended to also be valid ISO C should not use any of
the C headers.

With the exception of complex.h, each xxx.h header included in the C++ standard library places
in the global namespace each name that the corresponding cxxx header would have placed in the
std namespace.

These headers are allowed to also declare the same names in the std namespace, and the
corresponding cxxx headers are allowed to also declare the same names in the global namespace:
including "cstdlib" definitely provides std::malloc and may also provide ::malloc. Including
"stdlib.h> definitely provides ::malloc and may also provide std::malloc. This applies even to
"functions and function overloads that are not part of C standard library.

Notes: xxx.h headers are deprecated in C++98 and undeprecated in C++23. These headers are
discouraged for pure C++ code, but not subject to future removal.
*/

#include <assert.h> //					Behaves same as "cassert"
#include <ctype.h> //					Behaves as if each name from "cctype" is placed in global namespace
#include <errno.h> //					Behaves same as "cerrno"
#include <float.h> //					Behaves same as "cfloat"
#include <limits.h> //					Behaves same as "climits"
#include <locale.h> //					Behaves as if each name from "clocale" is placed in global namespace
#include <math.h> //					Behaves as if each name from "cmath" is placed in global namespace, except for names of mathematical special functions
#include <setjmp.h> //					Behaves as if each name from "csetjmp" is placed in global namespace
#include <signal.h> //					Behaves as if each name from "csignal" is placed in global namespace
#include <stdarg.h> //					Behaves as if each name from "cstdarg" is placed in global namespace
#include <stddef.h> //					Behaves as if each name from "cstddef" is placed in global namespace, except for names of std::byte and related functions
#include <stdio.h> //					Behaves as if each name from "cstdio" is placed in global namespace
#include <stdlib.h> //					Behaves as if each name from "cstdlib" is placed in global namespace
#include <string.h> //					Behaves as if each name from "cstring" is placed in global namespace
#include <time.h> //					Behaves as if each name from "ctime" is placed in global namespace
#include <wchar.h> //					Behaves as if each name from "cwchar" is placed in global namespace
#include <wctype.h> //					Behaves as if each name from "cwctype" is placed in global namespace Special C compatibility headers
#include <fenv.h> //(C++11)				Behaves as if each name from "cfenv" is placed in global namespace
#include <inttypes.h> //(C++11)			Behaves as if each name from "cinttypes" is placed in global namespace
#include <stdint.h> //(C++11)			Behaves as if each name from "cstdint" is placed in global namespace
#include <uchar.h> //(C++11)			Behaves as if each name from "cuchar" is placed in global namespace

/*
The header "stdatomic.h> declares names which are also provided in the C standard library, and defines the _Atomic macro which is a keyword in C. Unlike other xxx.h headers, corresponding "cstdatomic" is not provided.

*/
#if ai_std_version == 23
	#include <stdatomic.h> //(C++23)		Defines _Atomic and provides corresponding components in the C standard library
#endif

/*
Empty C headers

The headers "complex.h>, "ccomplex", "tgmath.h>, and "ctgmath" do not contain any content from the C standard library and instead merely include other headers from the C++ standard library.
*/
#include <ccomplex> //(C++11)			(deprecated in C++17)(removed in C++20)	Simply includes the header "complex"
#include <complex.h> //(C++11)			Simply includes the header "complex"
#include <ctgmath> //(C++11)			(deprecated in C++17)(removed in C++20)	Simply includes the headers "complex" and "cmath": the overloads equivalent to the contents of the C header tgmath.h are already provided by those headers
#include <tgmath.h> //(C++11)			Simply includes the headers "complex" and "cmath"

// Meaningless C headers

//The headers "ciso646", "cstdalign", and "cstdbool" are meaningless in C++ because the macros they provide in C are language keywords in C++.
#include <ciso646> //					(removed in C++20)	Empty header. The macros that appear in iso646.h in C are keywords in C++
#include <iso646.h> //					Has no effect
#if ai_std_version < 17
	#include <cstdalign> //(C++11)			(deprecated in C++17)(removed in C++20)	Defines one compatibility macro constant
	#include <cstdbool> // (C++11)			(deprecated in C++17)(removed in C++20)	Defines one compatibility macro constant
#endif
#include <stdalign.h> //(C++11)			Defines one compatibility macro constant
#include <stdbool.h> //(C++11)			Defines one compatibility macro constant



// /usr/lib/gcc/x86_64-pc-linux-gnu/13/include/g++-v13/
#include <stdatomic.h>
#if HAVE_LINUX
	#include <debug/debug.h>
	#include <debug/formatter.h>
	#include <debug/set.h>
	#include <debug/multimap.h>
	#include <debug/safe_base.h>
	#include <debug/assertions.h>
	#include <debug/map.h>
	#include <debug/safe_iterator.h>
	#include <debug/multiset.h>
	#include <debug/safe_sequence.h>
	#include <debug/helper_functions.h>
	#include <debug/safe_unordered_base.h>
	#include <debug/safe_unordered_container.h>
	#include <debug/stl_iterator.h>
	#include <debug/safe_container.h>
	#include <debug/safe_local_iterator.h>
	#include <debug/macros.h>
	
	#include <ext/pointer.h>
	//#include <ext/rc_string_base.h>
	#include <ext/throw_allocator.h>
	#include <ext/aligned_buffer.h>
	#include <ext/pod_char_traits.h>
	#include <ext/vstring.h>
	#include <ext/sso_string_base.h>
	#include <ext/stdio_filebuf.h>
	#include <ext/numeric_traits.h>
	#include <ext/bitmap_allocator.h>
	#include <ext/alloc_traits.h>
	#include <ext/typelist.h>
	#include <ext/vstring_fwd.h>
	#include <ext/atomicity.h>
	#include <ext/debug_allocator.h>
	#include <ext/codecvt_specializations.h>
	#include <ext/new_allocator.h>
	#include <ext/mt_allocator.h>
	#include <ext/enc_filebuf.h>
	#include <ext/concurrence.h>
	#include <ext/vstring_util.h>
	//#include <ext/ropeimpl.h>
	#include <ext/string_conversions.h>
	#include <ext/pool_allocator.h>
	
	#include <ext/type_traits.h>
	#include <ext/stdio_sync_filebuf.h>
	#include <ext/extptr_allocator.h>
	#include <ext/cast.h>
	#include <ext/malloc_allocator.h>
	
	#include <backward/hashtable.h>
	#include <backward/hash_fun.h>
	#include <backward/binders.h>
	#include <backward/auto_ptr.h>
	#include <backward/backward_warning.h>
	
	#include <parallel/list_partition.h>
	#include <parallel/compiletime_settings.h>
	#include <parallel/partition.h>
	#include <parallel/set_operations.h>
	#include <parallel/merge.h>
	#include <parallel/multiway_mergesort.h>
	#include <parallel/for_each_selectors.h>
	#include <parallel/workstealing.h>
	#include <parallel/multiway_merge.h>
	#include <parallel/features.h>
	#include <parallel/balanced_quicksort.h>
	#include <parallel/basic_iterator.h>
	#include <parallel/base.h>
	#include <parallel/find_selectors.h>
	#include <parallel/find.h>
	#include <parallel/omp_loop_static.h>
	#include <parallel/quicksort.h>
	#include <parallel/equally_split.h>
	#include <parallel/compatibility.h>
	#include <parallel/algobase.h>
	#include <parallel/settings.h>
	#include <parallel/random_shuffle.h>
	#include <parallel/search.h>
	#include <parallel/losertree.h>
	#include <parallel/par_loop.h>
	#include <parallel/algo.h>
	#include <parallel/algorithmfwd.h>
	#include <parallel/multiseq_selection.h>
	#include <parallel/checkers.h>
	#include <parallel/parallel.h>
	#include <parallel/for_each.h>
	#include <parallel/partial_sum.h>
	#include <parallel/unique_copy.h>
	#include <parallel/iterator.h>
	#include <parallel/omp_loop.h>
	#include <parallel/tags.h>
	#include <parallel/types.h>
	#include <parallel/sort.h>
	#include <parallel/random_number.h>
	#include <parallel/queue.h>
	#include <parallel/numericfwd.h>
#endif


#include <fenv.h>

#include <tgmath.h>
#include <math.h>
#include <stdlib.h>

#include <complex.h>
#if HAVE_GCC || HAVE_CLANG
	#include <cxxabi.h>
#endif

#if HAVE_WIN32
	#include <windows.h>
#endif

// /usr/include
#if HAVE_GCC || HAVE_CLANG
	#include <dirent.h>
	#include <pthread.h>
	#include <getopt.h>
	#include <ftw.h>
	#include <utime.h>
	#include <unistd.h>
	#include <sched.h>
	#include <semaphore.h>
	#include <libgen.h>
	#include <error.h>
	#include <strings.h>
#endif
#if HAVE_CLANG
	#include <ompx.h>
#endif
#if HAVE_LINUX
	#include <cpio.h>
	#include <libkmod.h>
	#include <etip.h>
	#include <gnumake.h>
	#include <regex.h>
	#include <x264_config.h>
	#include <utf8proc.h>
	#include <sgtty.h>
	#include <tld.h>
	#include <form.h>
	#include <libv4l2.h>
	#include <libconfig.h>
	#include <sox.h>
	#include <aliases.h>
	#include <utmp.h>
	#include <mcheck.h>
	#include <libgpsmm.h>
	//#include <regexp.h>
	#include <gsasl.h>
	#include <newt.h>
	#include <ndp.h>
	#include <libssh2.h>
	#include <uniwidth.h>
	#include <a.out.h>
	#include <ucontext.h>
	//#include <sha.h>
	#include <gcrypt.h>
	#include <wayland-client.h>
	//#include <pcre2posix.h>
	#include <fmtmsg.h>
	#include <unigbrk.h>
	#include <cursesm.h>
	//#include <tclOODecls.h>
	#include <unilbrk.h>
	#include <ladspa.h>
	#include <dolphinvcs_export.h>
	#include <sqlite3ext.h>
	#include <tclPlatDecls.h>
	#include <syscall.h>
	#include <glob.h>
	#include <values.h>
	#include <atomic_ops_malloc.h>
	#include <termcap.h>
	#include <duktape.h>
	#include <ltdl.h>
	#include <tiffvers.h>
	#include <gnu-versions.h>
	#include <blake2.h>
	#include <tar.h>
	#include <assuan.h>
	#include <atasmart.h>
	#include <tiffio.hxx>
	#include <mpfr.h>
	#include <pty.h>
	#include <paper.h>
	#include <aio.h>
	#include <x264.h>
	#include <ncurses.h>
	#include <pcrecpp.h>
	#include <gmp.h>
	#include <uthash.h>
	#include <ini.h>
	#include <ctf.h>
	#include <ttyent.h>
	#include <syslog.h>
	#include <dts.h>
	#include <ar.h>
	#include <libssh2_publickey.h>
	#include <monetary.h>
	#include <sndfile.hh>
	#include <idn-free.h>
	#include <ulimit.h>
	#include <lz4frame.h>
	#include <gpgrt.h>
	#include <libv4lconvert.h>
	#include <err.h>
	#include <png.h>
	#include <lcms2_fast_float.h>
	#include <libnetlink.h>
	#include <archive.h>
	#include <autosprintf.h>
	#include <fnmatch.h>
	#include <tclTomMathDecls.h>
	#include <asoundlib.h>
	#include <term.h>
	#include <libv4l1.h>
	#include <pcre_scanner.h>
	#include <ctf-api.h>
	#include <snappy-sinksource.h>
	#include <crypt.h>
	#include <jbig2.h>
	#include <wait.h>
	#include <wayland-server.h>
	#include <mqueue.h>
	#include <mad.h>
	#include <ncurses_dll.h>
	#include <gc.h>
	#include <unctrl.h>
	#include <features.h>
	#include <stab.h>
	#include <libv4l1-videodev.h>
	#include <poll.h>
	#include <rhash.h>
	#include <sha2.h>
	#include <xvid.h>
	#include <mtdev-plumbing.h>
	#include <xcrypt.h>
	#include <sframe.h>
	#include <duk_config.h>
	#include <natspec.h>
	#include <dialog.h>
	#include <iconv.h>
	#include <portaudio.h>
	#include <out123.h>
	#include <wordexp.h>
	#include <libpsl.h>
	#include <pa_jack.h>
	#include <ulockmgr.h>
	#include <panel.h>
	#include <lcms2_threaded.h>
	#include <execinfo.h>
	#include <argz.h>
	#include <gsasl-version.h>
	#include <termios.h>
	#include <archive_entry.h>
	#include <idn-int.h>
	#include <turbojpeg.h>
	//#include <asyncns.h>
	#include <sass.h>
	#include <idna.h>
	#include <utmpx.h>
	#include <obstack.h>
	#include <wayland-version.h>
	#include <gdbm.h>
	#include <diagnostics.h>
	//#include <cursesapp.h>
	#include <byteswap.h>
	#include <idn2.h>
	#include <utarray.h>
	//#include <gmpxx.h>
	#include <stdc-predef.h>
	#include <qrencode.h>
	#include <uniname.h>
	#include <md2.h>
	#include <npth.h>
	#include <libintl.h>
	#include <lcms2_plugin.h>
	#include <wayland-client-core.h>
	#include <re_comp.h>
	#include <tclTomMath.h>
	#include <xtables.h>
	#include <gpgme.h>
	#include <canberra.h>
	#include <popt.h>
	#include <nss.h>
	#include <magic.h>
	#include <ieee754.h>
	//#include <fuse.h>
	#include <eti.h>
	//#include <libiberty.h>
	#include <wayland-server-protocol.h>
	#include <term_entry.h>
	#include <ip6tables.h>
	#include <dlg_keys.h>
	#include <einfo.h>
	#include <mpc.h>
	//#include <pcap-namedb.h>
	#include <dotconf.h>
	#include <sysexits.h>
	#include <envz.h>
	#include <tiffconf.h>
	#include <libinput.h>
	#include <syn123.h>
	#include <utstack.h>
	#include <uv.h>
	#include <unitypes.h>
	#include <sha1.h>
	#include <link.h>
	#include <lmdb.h>
	#include <dmtx.h>
	#include <wayland-client-protocol.h>
	//#include <xf86drmMode.h>
	#include <snappy-stubs-public.h>
	//#include <md5.h>
	#include <fstab.h>
	#include <initreq.h>
	#include <mpdecimal.h>
	#include <endian.h>
	#include <uniconv.h>
	#include <ares_dns.h>
	#include <mpg123.h>
	#include <mtdev.h>
	#include <netdb.h>
	#include <sass2scss.h>
	#include <pcre2.h>
	#include <wayland-cursor.h>
	//#include <slcurses.h>
	#include <gumbo.h>
	#include <FlexLexer.h>
	#include <libyasm-stdint.h>
	//#include <canberra-gtk.h>
	#include <langinfo.h>
	#include <pcre_stringpiece.h>
	//#include <libnvme-mi.h>
	#include <pcap-bpf.h>
	#include <argon2.h>
	#include <argp.h>
	#include <decimal.hh>
	#include <cursesp.h>
	#include <grp.h>
	#include <dolphin_export.h>
	#include <shadow.h>
	#include <ksba.h>
	#include <pcap.h>
	#include <spawn.h>
	#include <utempter.h>
	#include <tiffio.h>
	#include <pwd.h>
	#include <gpg-error.h>
	#include <tcpd.h>
	#include <gettext-po.h>
	#include <zdict.h>
	#include <gshadow.h>
	#include <ifaddrs.h>
	#include <printf.h>
	#include <zstd_errors.h>
	//#include <jmorecfg.h>
	#include <mntent.h>
	#include <unicase.h>
	#include <libssh2_sftp.h>
	#include <unictype.h>
	#include <symcat.h>
	#include <features-time64.h>
	#include <tclOO.h>
	#include <alloca.h>
	#include <lastlog.h>
	#include <iptables.h>
	#include <passwdqc.h>
	#include <keyutils.h>
	#include <utringbuffer.h>
	#include <stdio_ext.h>
	#include <rc.h>
	//#include <dis-asm.h>
	#include <tiff.h>
	#include <stdbit.h>
	#include <freeaptx.h>
	#include <gc_cpp.h>
	#undef buttons
	#include <gpm.h>
	#include <dlfcn.h>
	#include <mujs.h>
	#include <resolv.h>
	#include <wayland-util.h>
	#include <pr29.h>
	#include <wayland-egl-backend.h>
	#include <plugin-api.h>
	#include <uniwbrk.h>
	#include <fpu_control.h>
	#include <uninorm.h>
	#include <snappy-c.h>
	#include <utlist.h>
	#include <dca.h>
	#include <menu.h>
	#include <fts.h>
	//#include <md4.h>
	#include <termio.h>
	#include <libudev.h>
	#include <libv4l-plugin.h>
	#include <atomic_ops_stack.h>
	#include <dwarf.h>
	#include <dlg_colors.h>
	#include <cursesw.h>
	#include <lz4hc.h>
	#include <wayland-server-core.h>
	//#include <seccomp-syscalls.h>
	#include <ares_build.h>
	#include <libstemmer.h>
	#include <samplerate.h>
	#include <xtables-version.h>
	#include <ares_version.h>
	#include <proc_service.h>
	#include <nl_types.h>
	#include <gawkapi.h>
	#include <ares.h>
	#include <libsync.h>
	#include <tclDecls.h>
	#include <thread_db.h>
	#include <sframe-api.h>
	#include <INIReader.h>
	#include <snappy.h>
	#include <rhash_torrent.h>
	#include <yaml.h>
	#include <fmt123.h>
	//#include <bfd.h>
	#include <lv2.h>
	#include <libv4l2rds.h>
	#include <pcrecpparg.h>
	#include <mpf2mpfr.h>
	#include <wayland-egl.h>
	#include <wayland-egl-core.h>
	#include <jerror.h>
	#include <curses.h>
	#include <tcl.h>
	#include <paths.h>
	#include <threads.h>
	#include <dlg_config.h>
	#include <pipeline.h>
	#include <atomic_ops.h>
	//#include <elf.h>
	#include <stringprep.h>
	#include <libmtp.h>
	#include <libyasm.h>
	#include <gsasl-mech.h>
	#include <fluidsynth.h>
	#include <libconfig.h++>
	#include <libdevmapper.h>
	#include <ares_dns_record.h>
	#include <unistr.h>
	#include <rmd160.h>
	#include <lz4.h>
	#include <sndfile.h>
	#include <ares_nameser.h>
	#include <lcms2.h>
	#include <unistdio.h>
	#include <unimetadata.h>
	#include <gelf.h>
	#include <jconfig.h>
	#include <libcryptsetup.h>
	#include <gconv.h>
	#include <com_err.h>
	//#include <slang.h>
	//#include <bfdlink.h>
	#include <jpeglib.h>
	#include <pngconf.h>
	#include <mtdev-mapping.h>
	#include <textstyle.h>
	#include <libelf.h>
	#include <pa_linux_alsa.h>
	#include <pnglibconf.h>
	//#include <sha256.h>
	#include <zstd.h>
	#include <pcre.h>
	#include <punycode.h>
	#include <gbm.h>
	#include <ansidecl.h>
	//#include <cursslk.h>
	#include <utstring.h>
	#include <seccomp.h>
	#include <gps.h>
	#include <netinet/ip.h>
	#include <netinet/igmp.h>
	#include <netinet/udp.h>
	#include <netinet/in.h>
	#include <netinet/ip6.h>
	#include <netinet/ether.h>
	#include <netinet/ip_icmp.h>
	#include <netinet/if_fddi.h>
	#include <netinet/in_systm.h>
	#include <netinet/icmp6.h>
	#include <netinet/if_tr.h>
	#include <netinet/tcp.h>
	#include <netinet/if_ether.h>
	#include <net/ppp-comp.h>
	#include <net/if_arp.h>
	#include <net/ppp_defs.h>
	#include <net/if_shaper.h>
#endif

#include <inttypes.h>
#include <fcntl.h>
#include <stdint.h>
#include <zlib.h>
//#include <ripemd.h>
//#include <sha512.h>
//#include <nlist.h>
#include <time.h>
//#include <cursesf.h>
#include <fenv.h>
#include <uchar.h>
#include <tgmath.h>
#include <omp.h>
#include <errno.h>
#include <math.h>
#include <memory.h>
#include <malloc.h>
#include <stdio.h>
#include <zconf.h>
#include <lzma.h>
#include <search.h>
#include <locale.h>
#include <bzlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <expat_config.h>
//#include <pcreposix.h>
#include <ctype.h>
#include <stdlib.h>
#include <expat_external.h>
#include <assert.h>
#include <wchar.h>
#include <wctype.h>
#include <expat.h>
//#include <tag_enum.h>
#include <sqlite3.h>
#include <complex.h>
//#include <xf86drm.h>
#include <limits.h>
//#include <libtasn1.h>


#if HAVE_LINUX
	//#include <net/if_packet.h>
	#include <net/ethernet.h>
	#include <net/if_ppp.h>
	#include <net/if_slip.h>
	#include <net/if.h>
	#include <net/route.h>
	//#include <sys/pidfd.h>
	#include <sys/xattr.h>
	#include <sys/gmon.h>
	#include <sys/capability.h>
	#include <sys/ucontext.h>
	#include <sys/personality.h>
	#include <sys/fsuid.h>
	#include <sys/epoll.h>
	#include <sys/statfs.h>
	#include <sys/fcntl.h>
	#include <sys/syscall.h>
	#include <sys/times.h>
	#include <sys/sem.h>
	//#include <sys/vm86.h>
	#include <sys/socketvar.h>
	#include <sys/syslog.h>
	#include <sys/utsname.h>
	#include <sys/io.h>
	#include <sys/debugreg.h>
	#include <sys/vlimit.h>
	#include <sys/sendfile.h>
	#include <sys/klog.h>
	#include <sys/asoundlib.h>
	#include <sys/user.h>
	#include <sys/acl.h>
	#include <sys/wait.h>
	#include <sys/rseq.h>
	//#include <sys/platform/x86.h>
	#include <sys/reboot.h>
	#include <sys/procfs.h>
	#include <sys/poll.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/mtio.h>
	#include <sys/file.h>
	#include <sys/termios.h>
	#include <sys/sysinfo.h>
	#include <sys/errno.h>
	#include <sys/mount.h>
	#include <sys/timepps.h>
	#include <sys/vfs.h>
	#include <sys/kd.h>
	#include <sys/sysmacros.h>
	#include <sys/timex.h>
	#include <sys/stat.h>
	#include <sys/fanotify.h>
	#include <sys/timerfd.h>
	#include <sys/shm.h>
	#include <sys/unistd.h>
	#include <sys/param.h>
	#include <sys/ttychars.h>
	#include <sys/vt.h>
	#include <sys/swap.h>
	#include <sys/signal.h>
	#include <sys/resource.h>
	#include <sys/ptrace.h>
	#include <sys/auxv.h>
	#include <sys/acct.h>
	#include <sys/single_threaded.h>
	#include <sys/select.h>
	#include <sys/cdefs.h>
	#include <sys/reg.h>
	#include <sys/ipc.h>
	#include <sys/prctl.h>
	#include <sys/inotify.h>
	#include <sys/timeb.h>
	#include <sys/psx_syscall.h>
	#include <sys/uio.h>
	#include <sys/perm.h>
	#include <sys/mman.h>
	//#include <sys/elf.h>
	#include <sys/un.h>
	#include <sys/types.h>
	#include <sys/profil.h>
	#include <sys/signalfd.h>
	#include <sys/gmon_out.h>
	#include <sys/msg.h>
	#include <sys/quota.h>
	#include <sys/eventfd.h>
	#include <sys/ttydefaults.h>
	#include <sys/ioctl.h>
	#include <sys/queue.h>
	#include <sys/raw.h>
	#include <sys/random.h>
	#include <sys/statvfs.h>
	#include <sys/soundcard.h>
	#include <sys/pci.h>
	#include <sys/dir.h>
	#include <sys/bitypes.h>
	
	#include <x86_64-pc-linux-gnu/libavutil/avconfig.h>
	#include <x86_64-pc-linux-gnu/gmp.h>
	#include <x86_64-pc-linux-gnu/gpgrt.h>
	//#include <x86_64-pc-linux-gnu/va/va_dri2.h>
	//#include <x86_64-pc-linux-gnu/va/va_x11.h>
	//#include <x86_64-pc-linux-gnu/va/va_dricommon.h>
	#include <x86_64-pc-linux-gnu/autosprintf.h>
	#include <x86_64-pc-linux-gnu/SDL2/SDL_config.h>
	#include <x86_64-pc-linux-gnu/SDL2/SDL_platform.h>
	//#include <x86_64-pc-linux-gnu/SDL2/close_code.h>
	//#include <x86_64-pc-linux-gnu/SDL2/begin_code.h>
	#include <x86_64-pc-linux-gnu/mad.h>
	#include <x86_64-pc-linux-gnu/textstyle/version.h>
	#include <x86_64-pc-linux-gnu/textstyle/stdbool.h>
	#include <x86_64-pc-linux-gnu/textstyle/woe32dll.h>
	#include <x86_64-pc-linux-gnu/tiffconf.h>
	#include <x86_64-pc-linux-gnu/gpg-error.h>
	#include <x86_64-pc-linux-gnu/gettext-po.h>
	#include <x86_64-pc-linux-gnu/libxslt/xsltconfig.h>
	#include <x86_64-pc-linux-gnu/ogg/config_types.h>
	#include <x86_64-pc-linux-gnu/ext2fs/ext2_types.h>
	#include <x86_64-pc-linux-gnu/openssl/configuration.h>
	#include <x86_64-pc-linux-gnu/ares_build.h>
	//#include <x86_64-pc-linux-gnu/sasl/md5global.h>
	//#include <x86_64-pc-linux-gnu/bfd.h>
	#include <x86_64-pc-linux-gnu/nettle/version.h>
	#include <x86_64-pc-linux-gnu/jconfig.h>
	#include <x86_64-pc-linux-gnu/textstyle.h>
	#include <arpa/nameser_compat.h>
	#include <arpa/inet.h>
	#include <arpa/telnet.h>
	#include <arpa/tftp.h>
	#include <arpa/nameser.h>
	#include <arpa/ftp.h>
#endif


#if HAVE_OPENGL
	#include <GL/gl.h>
	//#include <GL/glxint.h>
	#include <GL/glu.h>
	#if HAVE_GCC || HAVE_CLANG
		#include <GL/glext.h>
		#include <GL/glcorearb.h>
	#endif
	#if HAVE_LINUX
		#include <GL/freeglut.h>
		#include <GL/freeglut_ucall.h>
		#include <GL/freeglut_std.h>
		#include <GL/freeglut_ext.h>
		#include <GL/glut.h>
		//#include <GL/glx.h>
		#include <GL/internal/glcore.h>
		#include <GL/internal/dri_interface.h>
		//#include <GL/glxext.h>
		//#include <GL/glxmd.h>
		//#include <GL/glxproto.h>
	#endif
#endif

#if HAVE_OPENSSL
	#include <openssl/ssl.h>
	#include <openssl/opensslv.h>
	#include <openssl/modes.h>
	#include <openssl/srp.h>
	#include <openssl/rsaerr.h>
	#include <openssl/async.h>
	#include <openssl/tserr.h>
	#include <openssl/sha.h>
	#include <openssl/ct.h>
	#include <openssl/bioerr.h>
	#include <openssl/ec.h>
	#include <openssl/sslerr_legacy.h>
	#include <openssl/stack.h>
	#include <openssl/dsaerr.h>
	#include <openssl/crypto.h>
	#include <openssl/self_test.h>
	#include <openssl/ecdsa.h>
	#include <openssl/fips_names.h>
	#include <openssl/e_os2.h>
	#include <openssl/httperr.h>
	#include <openssl/comp.h>
	#include <openssl/cms.h>
	#include <openssl/fipskey.h>
	#include <openssl/e_ostime.h>
	#include <openssl/ripemd.h>
	#include <openssl/aes.h>
	#include <openssl/cryptoerr.h>
	#include <openssl/sslerr.h>
	#include <openssl/pkcs12.h>
	#include <openssl/crmferr.h>
	#include <openssl/ecdh.h>
	#include <openssl/err.h>
	#include <openssl/des.h>
	#include <openssl/camellia.h>
	#include <openssl/bn.h>
	#include <openssl/proverr.h>
	#include <openssl/storeerr.h>
	#include <openssl/dtls1.h>
	#include <openssl/objectserr.h>
	#include <openssl/rsa.h>
	#include <openssl/ocsp.h>
	#include <openssl/crmf.h>
	#include <openssl/params.h>
	//#include <openssl/asn1_mac.h>
	#include <openssl/core.h>
	#include <openssl/x509_vfy.h>
	#include <openssl/pkcs12err.h>
	#include <openssl/decoder.h>
	#include <openssl/cmp.h>
	#include <openssl/store.h>
	#include <openssl/asn1.h>
	#include <openssl/symhacks.h>
	#include <openssl/ocsperr.h>
	#include <openssl/ecerr.h>
	#include <openssl/dsa.h>
	#include <openssl/txt_db.h>
	#include <openssl/buffererr.h>
	#include <openssl/ui.h>
	#include <openssl/ess.h>
	#include <openssl/tls1.h>
	#include <openssl/md2.h>
	#include <openssl/thread.h>
	#include <openssl/core_object.h>
	#include <openssl/cryptoerr_legacy.h>
	#include <openssl/rc2.h>
	#include <openssl/comperr.h>
	#include <openssl/core_dispatch.h>
	#include <openssl/kdferr.h>
	#include <openssl/rand.h>
	#include <openssl/encodererr.h>
	#include <openssl/lhash.h>
	#include <openssl/dherr.h>
	#include <openssl/idea.h>
	#include <openssl/md5.h>
	#include <openssl/cterr.h>
	#include <openssl/pem.h>
	#include <openssl/cmac.h>
	#include <openssl/cmp_util.h>
	#include <openssl/encoder.h>
	#include <openssl/decodererr.h>
	#include <openssl/obj_mac.h>
	#include <openssl/evperr.h>
	#include <openssl/srtp.h>
	#include <openssl/dh.h>
	#include <openssl/prov_ssl.h>
	#include <openssl/seed.h>
	#include <openssl/conferr.h>
	#include <openssl/buffer.h>
	#include <openssl/configuration.h>
	#include <openssl/ssl2.h>
	#include <openssl/mdc2.h>
	#include <openssl/bio.h>
	#include <openssl/conf.h>
	#include <openssl/kdf.h>
	#include <openssl/objects.h>
	#include <openssl/asyncerr.h>
	#include <openssl/http.h>
	#include <openssl/trace.h>
	#include <openssl/pkcs7err.h>
	#include <openssl/x509.h>
	#include <openssl/engineerr.h>
	#include <openssl/x509err.h>
	#include <openssl/uierr.h>
	#include <openssl/core_names.h>
	#include <openssl/param_build.h>
	#include <openssl/ebcdic.h>
	#include <openssl/ts.h>
	#include <openssl/rc4.h>
	#include <openssl/randerr.h>
	#include <openssl/md4.h>
	#include <openssl/rc5.h>
	#include <openssl/cmserr.h>
	#include <openssl/engine.h>
	#include <openssl/ssl3.h>
	#include <openssl/cmperr.h>
	#include <openssl/blowfish.h>
	#include <openssl/hmac.h>
	#include <openssl/quic.h>
	#include <openssl/bnerr.h>
	#include <openssl/x509v3.h>
	#include <openssl/provider.h>
	#include <openssl/pem2.h>
	#include <openssl/safestack.h>
	#include <openssl/cast.h>
	#include <openssl/hpke.h>
	#include <openssl/types.h>
	#include <openssl/esserr.h>
	#include <openssl/whrlpool.h>
	#include <openssl/conf_api.h>
	#include <openssl/pemerr.h>
	#include <openssl/asn1err.h>
	#include <openssl/opensslconf.h>
	#include <openssl/ossl_typ.h>
	#include <openssl/x509v3err.h>
	#include <openssl/pkcs7.h>
	#include <openssl/asn1t.h>
	#include <openssl/conftypes.h>
	#include <openssl/macros.h>
	#include <openssl/evp.h>
#endif

#if HAVE_SDL2
	#undef lines
	#undef GC
	#if HAVE_LINUX || HAVE_MSYS
		#include <SDL2/SDL.h>
		#include <SDL2/SDL_mutex.h>
		#include <SDL2/SDL_gamecontroller.h>
		#include <SDL2/SDL_platform.h>
		#include <SDL2/SDL_copying.h>
		#include <SDL2/SDL_clipboard.h>
		#include <SDL2/SDL_render.h>
		#include <SDL2/SDL_joystick.h>
		//#include <SDL2/close_code.h>
		#include <SDL2/SDL_opengl.h>
		#include <SDL2/SDL_test.h>
		#include <SDL2/SDL_test_crc32.h>
		#include <SDL2/SDL_locale.h>
		#include <SDL2/SDL_hints.h>
		//#include <SDL2/SDL_syswm.h>
		#include <SDL2/SDL_sensor.h>
		#include <SDL2/SDL_blendmode.h>
		#include <SDL2/SDL_video.h>
		#include <SDL2/SDL_error.h>
		#include <SDL2/SDL_stdinc.h>
		#include <SDL2/SDL_events.h>
		#include <SDL2/SDL_atomic.h>
		#include <SDL2/SDL_test_fuzzer.h>
		#include <SDL2/SDL_mouse.h>
		#include <SDL2/SDL_filesystem.h>
		#include <SDL2/SDL_assert.h>
		#include <SDL2/SDL_endian.h>
		#include <SDL2/SDL_test_random.h>
		#include <SDL2/SDL_audio.h>
		#include <SDL2/SDL_gesture.h>
		#include <SDL2/SDL_thread.h>
		#include <SDL2/SDL_guid.h>
		#include <SDL2/SDL_log.h>
		#include <SDL2/SDL_rect.h>
		#include <SDL2/SDL_touch.h>
		#include <SDL2/SDL_hidapi.h>
		#include <SDL2/SDL_haptic.h>
		#include <SDL2/SDL_main.h>
		#include <SDL2/SDL_test_harness.h>
		#include <SDL2/SDL_loadso.h>
		//#include <SDL2/begin_code.h>
		#include <SDL2/SDL_rwops.h>
		#include <SDL2/SDL_test_memory.h>
		#include <SDL2/SDL_messagebox.h>
		#include <SDL2/SDL_name.h>
		#include <SDL2/SDL_cpuinfo.h>
		#include <SDL2/SDL_test_compare.h>
		#include <SDL2/SDL_test_md5.h>
		#include <SDL2/SDL_test_assert.h>
		#include <SDL2/SDL_version.h>
		#include <SDL2/SDL_revision.h>
		#include <SDL2/SDL_opengl_glext.h>
		#include <SDL2/SDL_system.h>
		#include <SDL2/SDL_metal.h>
		#include <SDL2/SDL_misc.h>
		#include <SDL2/SDL_test_log.h>
		#include <SDL2/SDL_surface.h>
		#include <SDL2/SDL_keyboard.h>
		#include <SDL2/SDL_test_images.h>
		#include <SDL2/SDL_test_common.h>
		#include <SDL2/SDL_timer.h>
		#include <SDL2/SDL_pixels.h>
		#include <SDL2/SDL_scancode.h>
		#include <SDL2/SDL_keycode.h>
		#include <SDL2/SDL_shape.h>
		#include <SDL2/SDL_test_font.h>
		#include <SDL2/SDL_types.h>
		#include <SDL2/SDL_quit.h>
		#include <SDL2/SDL_config.h>
		#include <SDL2/SDL_power.h>
		#include <SDL2/SDL_bits.h>
		#include <SDL2/SDL_vulkan.h>
		#if HAVE_LINUX
			#include <SDL2/SDL_egl.h>
			#include <SDL2/SDL_opengles2.h>
			#include <SDL2/SDL_opengles2_gl2platform.h>
			#include <SDL2/SDL_opengles2_khrplatform.h>
			#include <SDL2/SDL_opengles2_gl2ext.h>
			#include <SDL2/SDL_opengles2_gl2.h>
			#include <SDL2/SDL_opengles.h>
		#endif
	#endif
	#if HAVE_WIN32 && !HAVE_MINGW
		#include <SDL_assert.h>
		#include <SDL_atomic.h>
		#include <SDL_audio.h>
		#include <SDL_bits.h>
		#include <SDL_blendmode.h>
		#include <SDL_clipboard.h>
		#include <SDL_config_android.h>
		#include <SDL_config.h>
		#include <SDL_config.h.cmake>
		#include <SDL_config.h.in>
		#include <SDL_config_iphoneos.h>
		//#include <SDL_config_macosx.h>
		//#include <SDL_config_macosx.h.orig>
		#include <SDL_config_minimal.h>
		#include <SDL_config_pandora.h>
		#include <SDL_config_psp.h>
		#include <SDL_config_windows.h>
		#include <SDL_config_winrt.h>
		#include <SDL_config_wiz.h>
		#include <SDL_copying.h>
		#include <SDL_cpuinfo.h>
		//#include <SDL_egl.h>
		#include <SDL_endian.h>
		#include <SDL_error.h>
		#include <SDL_events.h>
		#include <SDL_filesystem.h>
		#include <SDL_gamecontroller.h>
		#include <SDL_gesture.h>
		#include <SDL_guid.h>
		#include <SDL.h>
		#include <SDL_haptic.h>
		#include <SDL_hidapi.h>
		#include <SDL_hints.h>
		#include <SDL_joystick.h>
		#include <SDL_keyboard.h>
		#include <SDL_keycode.h>
		#include <SDL_loadso.h>
		#include <SDL_locale.h>
		#include <SDL_log.h>
		#include <SDL_main.h>
		#include <SDL_messagebox.h>
		#include <SDL_metal.h>
		#include <SDL_misc.h>
		#include <SDL_mouse.h>
		#include <SDL_mutex.h>
		#include <SDL_name.h>
		/*#include <SDL_opengles2_gl2ext.h>
		#include <SDL_opengles2_gl2.h>
		#include <SDL_opengles2_gl2platform.h>
		#include <SDL_opengles2.h>
		#include <SDL_opengles2_khrplatform.h>
		#include <SDL_opengles.h>*/
		#include <SDL_opengl_glext.h>
		#include <SDL_opengl.h>
		#include <SDL_pixels.h>
		#include <SDL_platform.h>
		#include <SDL_power.h>
		#include <SDL_quit.h>
		#include <SDL_rect.h>
		#include <SDL_render.h>
		#include <SDL_revision.h>
		#include <SDL_rwops.h>
		#include <SDL_scancode.h>
		#include <SDL_sensor.h>
		#include <SDL_shape.h>
		#include <SDL_stdinc.h>
		#include <SDL_surface.h>
		#include <SDL_system.h>
		//#include <SDL_syswm.h>
		#include <SDL_test_assert.h>
		#include <SDL_test_common.h>
		#include <SDL_test_compare.h>
		#include <SDL_test_crc32.h>
		#include <SDL_test_font.h>
		#include <SDL_test_fuzzer.h>
		#include <SDL_test.h>
		#include <SDL_test_harness.h>
		#include <SDL_test_images.h>
		#include <SDL_test_log.h>
		#include <SDL_test_md5.h>
		#include <SDL_test_memory.h>
		#include <SDL_test_random.h>
		#include <SDL_thread.h>
		#include <SDL_timer.h>
		#include <SDL_touch.h>
		#include <SDL_types.h>
		#include <SDL_version.h>
		#include <SDL_video.h>
		#include <SDL_vulkan.h>
	#endif
#endif

#endif
