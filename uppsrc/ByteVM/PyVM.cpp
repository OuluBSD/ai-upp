#include "ByteVM.h"
#include <Core/Core.h>

#ifdef PLATFORM_POSIX
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace Upp {

static String GetRelPath(String path, String base) {
	path = NormalizePath(path);
	base = NormalizePath(base);
	if(path.StartsWith(base)) {
		String res = path.Mid(base.GetCount());
		if(res.StartsWith("/") || res.StartsWith("\\")) res = res.Mid(1);
		return res;
	}
	return path;
}

struct PyKV : Moveable<PyKV> {
	PyValue k, v;
	PyKV() {}
	PyKV(PyValue k, PyValue v) : k(k), v(v) {}
};

static PyValue builtin_print(const Vector<PyValue>& args, void*)
{
	for(int i = 0; i < args.GetCount(); i++) {
		if(i) Cout() << " ";
		Cout() << args[i].ToString();
	}
	Cout() << "\n";
	return PyValue::None();
}

static PyValue builtin_len(const Vector<PyValue>& args, void*)
{
	if(args.GetCount() == 0) return PyValue(0);
	return PyValue(args[0].GetCount());
}

static PyValue builtin_range(const Vector<PyValue>& args, void*) {
	int64 start = 0, stop = 0, step = 1;
	if(args.GetCount() == 1) stop = args[0].AsInt64();
	else if(args.GetCount() == 2) { start = args[0].AsInt64(); stop = args[1].AsInt64(); }
	else if(args.GetCount() == 3) { start = args[0].AsInt64(); stop = args[1].AsInt64(); step = args[2].AsInt64(); }
	return PyValue(new PyRangeIter(start, stop, step));
}

static PyValue builtin_complex(const Vector<PyValue>& args, void*) {
	double real = 0, imag = 0;
	if(args.GetCount() >= 1) real = args[0].AsDouble();
	if(args.GetCount() >= 2) imag = args[1].AsDouble();
	return PyValue(std::complex<double>(real, imag));
}

static PyValue builtin_bool(const Vector<PyValue>& args, void*) {
	if(args.GetCount() >= 1) return PyValue(args[0].IsTrue());
	return PyValue(false);
}

static PyValue builtin_iter(const Vector<PyValue>& args, void*) {
	if(args.GetCount() == 0) return PyValue::None();
	PyValue obj = args[0];
	if(obj.GetType() == PY_LIST || obj.GetType() == PY_TUPLE)
		return PyValue(new PyVectorIter(obj.GetArray()));
	if(obj.IsIterator())
		return obj;
	return PyValue::None();
}

static PyValue builtin_next(const Vector<PyValue>& args, void*) {
	if(args.GetCount() == 0) return PyValue::None();
	PyValue iterator = args[0];
	if(iterator.IsIterator())
		return iterator.GetIter().Next();
	return PyValue::None();
}

static PyValue builtin_math_fabs(const Vector<PyValue>& args, void*) {
	if(args.GetCount() == 0) return PyValue(0.0);
	return PyValue(std::abs(args[0].AsDouble()));
}

static PyValue builtin_abs(const Vector<PyValue>& args, void*) {
	if(args.GetCount() == 0) return PyValue(0);
	PyValue v = args[0];
	if(v.IsInt()) return PyValue(std::abs(v.AsInt64()));
	if(v.IsFloat()) return PyValue(std::abs(v.AsDouble()));
	if(v.GetType() == PY_COMPLEX) return PyValue(std::abs(v.GetComplex()));
	return v;
}

static PyValue builtin_str(const Vector<PyValue>& args, void*) {
	if(args.GetCount() == 0) return PyValue("");
	return PyValue(args[0].ToString());
}

static PyValue builtin_dir(const Vector<PyValue>& args, void*) {
	PyValue list = PyValue::List();
	if(args.GetCount() == 0) return list; // TODO: return local scope
	PyValue obj = args[0];
	if(obj.GetType() == PY_DICT) {
		const VectorMap<PyValue, PyValue>& dict = obj.GetDict();
		for(int i = 0; i < dict.GetCount(); i++) {
			PyValue k = dict.GetKey(i);
			if (k.GetType() == PY_STR) list.Add(k);
			else list.Add(PyValue(k.ToString()));
		}
	}
	// TODO: support other types
	return list;
}

static PyValue builtin_min(const Vector<PyValue>& args, void*) {
	if(args.GetCount() == 0) return PyValue::None();
	PyValue m;
	const Vector<PyValue> *items = &args;
	if(args.GetCount() == 1 && (args[0].GetType() == PY_LIST || args[0].GetType() == PY_TUPLE))
		items = &args[0].GetArray();
	
	if(items->IsEmpty()) return PyValue::None();
	m = (*items)[0];
	for(int i = 1; i < items->GetCount(); i++)
		if((*items)[i] < m) m = (*items)[i];
	return m;
}

static PyValue builtin_max(const Vector<PyValue>& args, void*) {
	if(args.GetCount() == 0) return PyValue::None();
	PyValue m;
	const Vector<PyValue> *items = &args;
	if(args.GetCount() == 1 && (args[0].GetType() == PY_LIST || args[0].GetType() == PY_TUPLE))
		items = &args[0].GetArray();
	
	if(items->IsEmpty()) return PyValue::None();
	m = (*items)[0];
	for(int i = 1; i < items->GetCount(); i++)
		if(m < (*items)[i]) m = (*items)[i];
	return m;
}

static PyValue builtin_sum(const Vector<PyValue>& args, void*) {
	if(args.GetCount() == 0) return PyValue(0);
	const Vector<PyValue> *items = &args;
	if(args.GetCount() >= 1 && (args[0].GetType() == PY_LIST || args[0].GetType() == PY_TUPLE))
		items = &args[0].GetArray();
	
	PyValue s(0);
	if (items->GetCount() > 0 && (*items)[0].GetType() == PY_STR) s = PyValue("");
	
	for(const auto& v : *items) {
		if(s.GetType() == PY_STR) s = PyValue(s.GetStr() + v.GetStr());
		else if(s.GetType() == PY_COMPLEX || v.GetType() == PY_COMPLEX) s = PyValue(s.GetComplex() + v.GetComplex());
		else if(s.IsFloat() || v.IsFloat()) s = PyValue(s.AsDouble() + v.AsDouble());
		else s = PyValue(s.AsInt64() + v.AsInt64());
	}
	return s;
}

static PyValue builtin_os_getcwd(const Vector<PyValue>& args, void*) {
	return PyValue(GetCurrentDirectory());
}

static PyValue builtin_os_mkdir(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::None();
	RealizeDirectory(args[0].ToString());
	return PyValue::None();
}

static PyValue builtin_os_rmdir(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::None();
	DeleteFolderDeep(args[0].ToString());
	return PyValue::None();
}

static PyValue builtin_os_rename(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue::None();
	return PyValue(FileMove(args[0].ToString(), args[1].ToString()));
}

static PyValue builtin_os_getenv(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::None();
	String val = GetEnv(args[0].ToString());
	if(::Upp::IsNull(val)) {
		if(args.GetCount() >= 2) return args[1];
		return PyValue::None();
	}
	return PyValue(val);
}

static PyValue builtin_os_putenv(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue::None();
	SetEnv(args[0].ToString(), args[1].ToString());
	return PyValue::None();
}

static PyValue builtin_os_getpid(const Vector<PyValue>& args, void*) {
#ifdef flagWIN32
	return PyValue((int64)GetCurrentProcessId());
#else
	return PyValue((int64)getpid());
#endif
}

static PyValue builtin_os_getuid(const Vector<PyValue>& args, void*) {
#ifdef flagWIN32
	return PyValue((int64)0);
#else
	return PyValue((int64)getuid());
#endif
}

static PyValue builtin_os_getgid(const Vector<PyValue>& args, void*) {
#ifdef flagWIN32
	return PyValue((int64)0);
#else
	return PyValue((int64)getgid());
#endif
}

static PyValue builtin_os_listdir(const Vector<PyValue>& args, void*) {
	String path = ".";
	if(args.GetCount() >= 1) path = args[0].ToString();
	PyValue list = PyValue::List();
	FindFile ff(AppendFileName(path, "*"));
	while(ff) {
		if(ff.GetName() != "." && ff.GetName() != "..")
			list.Add(PyValue(ff.GetName()));
		ff.Next();
	}
	return list;
}

static PyValue builtin_os_remove(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::None();
	FileDelete(args[0].ToString());
	return PyValue::None();
}

static PyValue builtin_os_chdir(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::None();
	SetCurrentDirectory(args[0].ToString());
	return PyValue::None();
}

static PyValue builtin_os_path_exists(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(false);
	String path = args[0].ToString();
	return PyValue(FileExists(path) || DirectoryExists(path));
}

static PyValue builtin_os_path_isdir(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(false);
	return PyValue(DirectoryExists(args[0].ToString()));
}

static PyValue builtin_os_path_isfile(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(false);
	return PyValue(FileExists(args[0].ToString()));
}

static PyValue builtin_os_path_getsize(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0);
	return PyValue((int64)GetFileLength(args[0].ToString()));
}

static PyValue builtin_os_path_islink(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(false);
#ifdef flagWIN32
	return PyValue(false);
#else
	struct stat st;
	if(lstat(args[0].ToString(), &st) == 0)
		return PyValue(S_ISLNK(st.st_mode));
	return PyValue(false);
#endif
}

static PyValue builtin_os_path_lexists(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(false);
#ifdef flagWIN32
	return builtin_os_path_exists(args, NULL);
#else
	struct stat st;
	return PyValue(lstat(args[0].ToString(), &st) == 0);
#endif
}

static PyValue builtin_os_path_getatime(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	FindFile ff(args[0].ToString());
	if(ff) return PyValue((double)(Time(ff.GetLastAccessTime()) - Time(1970, 1, 1)));
	return PyValue(0.0);
}

static PyValue builtin_os_path_getmtime(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	FindFile ff(args[0].ToString());
	if(ff) return PyValue((double)(Time(ff.GetLastWriteTime()) - Time(1970, 1, 1)));
	return PyValue(0.0);
}

static PyValue builtin_os_path_getctime(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	FindFile ff(args[0].ToString());
	if(ff) {
#ifdef flagWIN32
		return PyValue((double)(Time(ff.GetCreationTime()) - Time(1970, 1, 1)));
#else
		return PyValue((double)(Time(ff.GetLastChangeTime()) - Time(1970, 1, 1)));
#endif
	}
	return PyValue(0.0);
}

static PyValue builtin_os_path_realpath(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue("");
	return PyValue(GetFullPath(args[0].ToString()));
}

static PyValue builtin_os_path_relpath(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue("");
	String path = args[0].ToString();
	String start = GetCurrentDirectory();
	if(args.GetCount() >= 2) start = args[1].ToString();
	return PyValue(GetRelPath(path, start));
}

static PyValue builtin_os_path_samefile(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue(false);
	return PyValue(PathIsEqual(args[0].ToString(), args[1].ToString()));
}

static PyValue builtin_os_path_join(const Vector<PyValue>& args, void*) {
	if(args.GetCount() == 0) return PyValue("");
	String path = args[0].ToString();
	for(int i = 1; i < args.GetCount(); i++)
		path = AppendFileName(path, args[i].ToString());
	return PyValue(path);
}

static PyValue builtin_os_path_abspath(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue("");
	return PyValue(GetFullPath(args[0].ToString()));
}

static PyValue builtin_os_path_basename(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue("");
	return PyValue(GetFileName(args[0].ToString()));
}

static PyValue builtin_os_path_dirname(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue("");
	return PyValue(GetFileFolder(args[0].ToString()));
}

static PyValue builtin_os_path_split(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::Tuple();
	String path = args[0].ToString();
	PyValue t = PyValue::Tuple();
	t.Add(PyValue(GetFileFolder(path)));
	t.Add(PyValue(GetFileName(path)));
	return t;
}

static PyValue builtin_os_path_splitext(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::Tuple();
	String path = args[0].ToString();
	PyValue t = PyValue::Tuple();
	String ext = GetFileExt(path);
	if(ext.GetCount()) {
		t.Add(PyValue(path.Left(path.GetCount() - ext.GetCount() - 1)));
		t.Add(PyValue("." + ext));
	} else {
		t.Add(PyValue(path));
		t.Add(PyValue(""));
	}
	return t;
}

static PyValue builtin_os_path_isabs(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(false);
	String path = args[0].ToString();
#ifdef flagWIN32
	return PyValue((path.GetCount() >= 2 && IsAlpha(path[0]) && path[1] == ':') || path.StartsWith("\\") || path.StartsWith("/"));
#else
	return PyValue(path.StartsWith("/"));
#endif
}

static PyValue builtin_os_path_normpath(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue("");
	return PyValue(NormalizePath(args[0].ToString()));
}

static PyValue builtin_os_path_normcase(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue("");
#ifdef flagWIN32
	return PyValue(ToLower(args[0].ToString()));
#else
	return args[0];
#endif
}

static PyValue builtin_os_getlogin(const Vector<PyValue>& args, void*) {
	return PyValue(GetUserName());
}

static PyValue builtin_os_getppid(const Vector<PyValue>& args, void*) {
#ifdef flagWIN32
	return PyValue((int64)0); 
#else
	return PyValue((int64)getppid());
#endif
}

static PyValue builtin_os_makedirs(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::None();
	RealizeDirectory(args[0].ToString());
	return PyValue::None();
}

static PyValue builtin_os_removedirs(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::None();
	DeleteFolderDeep(args[0].ToString());
	return PyValue::None();
}

static PyValue builtin_os_replace(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue::None();
	return PyValue(FileMove(args[0].ToString(), args[1].ToString()));
}

static PyValue builtin_os_truncate(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue::None();
	String path = args[0].ToString();
	int64 length = args[1].AsInt64();
	FileOut out(path);
	if(out) out.SetSize(length);
	return PyValue::None();
}

static PyValue builtin_os_system(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(-1);
	return PyValue((int64)system(args[0].ToString()));
}

static PyValue builtin_os_cpu_count(const Vector<PyValue>& args, void*) {
	return PyValue((int64)CPU_Cores());
}

static PyValue builtin_os_urandom(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue("");
	int n = args[0].AsInt();
	Buffer<byte> b(n);
	for(int i = 0; i < n; i++) b[i] = (byte)Random(256);
	return PyValue(String((const char*)~b, n));
}

static PyValue builtin_os_readlink(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue("");
#ifdef flagWIN32
	return PyValue(""); 
#else
	char buf[1024];
	ssize_t len = readlink(args[0].ToString(), buf, sizeof(buf)-1);
	if(len != -1) {
		buf[len] = '\0';
		return PyValue(String(buf));
	}
	return PyValue("");
#endif
}

static PyValue builtin_os_symlink(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue::None();
#ifdef flagWIN32
	return PyValue::None();
#else
	symlink(args[0].ToString(), args[1].ToString());
	return PyValue::None();
#endif
}

static PyValue builtin_os_abort(const Vector<PyValue>& args, void*) {
	abort();
	return PyValue::None();
}

static PyValue builtin_os_access(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(false);
	String path = args[0].ToString();
	int mode = 0;
	if(args.GetCount() >= 2) mode = args[1].AsInt();
	return PyValue(access(path, mode) == 0);
}

static PyValue builtin_os_chmod(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue::None();
	chmod(args[0].ToString(), args[1].AsInt());
	return PyValue::None();
}

static PyValue builtin_subprocess_run(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::None();
	String cmd;
	if(args[0].GetType() == PY_LIST || args[0].GetType() == PY_TUPLE) {
		const Vector<PyValue>& v = args[0].GetArray();
		for(int i = 0; i < v.GetCount(); i++) {
			if(i) cmd << " ";
			cmd << v[i].ToString();
		}
	} else {
		cmd = args[0].ToString();
	}
	
	int res = system(cmd);
	PyValue obj = PyValue::Dict();
	obj.SetItem(PyValue("returncode"), PyValue(res == 0 ? 0 : (res >> 8)));
	return obj;
}

static PyValue builtin_str_endswith(const Vector<PyValue>& args, void* user_data) {
	if(args.GetCount() < 2) return PyValue(false);
	String self = args[0].ToString();
	return PyValue(self.EndsWith(args[1].ToString()));
}

static PyValue builtin_str_join(const Vector<PyValue>& args, void* user_data) {
	if(args.GetCount() < 2) return PyValue("");
	String self = args[0].ToString();
	String res;
	PyValue list = args[1];
	for(int i = 0; i < list.GetCount(); i++) {
		if(i) res << self;
		res << list.GetItem(i).ToString();
	}
	return PyValue(res);
}

static PyValue builtin_os_uname(const Vector<PyValue>& args, void*) {
	PyValue t = PyValue::Tuple();
#ifdef flagWIN32
	t.Add(PyValue("Windows"));
#elif defined(flagLINUX)
	t.Add(PyValue("Linux"));
#elif defined(flagMACOS)
	t.Add(PyValue("Darwin"));
#else
	t.Add(PyValue("Unknown"));
#endif
	t.Add(PyValue(GetComputerName()));
	t.Add(PyValue("")); // release
	t.Add(PyValue("")); // version
	t.Add(PyValue("")); // machine
	return t;
}

static PyValue builtin_sys_exit(const Vector<PyValue>& args, void*) {
	int code = 0;
	if(args.GetCount() >= 1) code = args[0].AsInt();
	throw Exc("EXIT:" + AsString(code));
	return PyValue::None();
}

static PyValue builtin_math_asinh(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(asinh(args[0].AsDouble())); }
static PyValue builtin_math_acosh(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(acosh(args[0].AsDouble())); }
static PyValue builtin_math_atanh(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(atanh(args[0].AsDouble())); }
static PyValue builtin_math_sinh(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(sinh(args[0].AsDouble())); }
static PyValue builtin_math_cosh(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(cosh(args[0].AsDouble())); }
static PyValue builtin_math_tanh(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(tanh(args[0].AsDouble())); }
static PyValue builtin_math_log10(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(log10(args[0].AsDouble())); }
static PyValue builtin_math_log2(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(log2(args[0].AsDouble())); }
static PyValue builtin_math_log1p(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(log1p(args[0].AsDouble())); }
static PyValue builtin_math_expm1(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(expm1(args[0].AsDouble())); }
static PyValue builtin_math_erf(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(erf(args[0].AsDouble())); }
static PyValue builtin_math_erfc(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(erfc(args[0].AsDouble())); }
static PyValue builtin_math_gamma(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(tgamma(args[0].AsDouble())); }
static PyValue builtin_math_lgamma(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(lgamma(args[0].AsDouble())); }
static PyValue builtin_math_fmod(const Vector<PyValue>& args, void*) { if(args.GetCount() < 2) return PyValue(0.0); return PyValue(fmod(args[0].AsDouble(), args[1].AsDouble())); }
static PyValue builtin_math_remainder(const Vector<PyValue>& args, void*) { if(args.GetCount() < 2) return PyValue(0.0); return PyValue(remainder(args[0].AsDouble(), args[1].AsDouble())); }
static PyValue builtin_math_copysign(const Vector<PyValue>& args, void*) { if(args.GetCount() < 2) return PyValue(0.0); return PyValue(copysign(args[0].AsDouble(), args[1].AsDouble())); }
static PyValue builtin_math_nextafter(const Vector<PyValue>& args, void*) { if(args.GetCount() < 2) return PyValue(0.0); return PyValue(nextafter(args[0].AsDouble(), args[1].AsDouble())); }

static PyValue builtin_math_cbrt(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(cbrt(args[0].AsDouble())); }
static PyValue builtin_math_exp2(const Vector<PyValue>& args, void*) { if(args.GetCount() < 1) return PyValue(0.0); return PyValue(exp2(args[0].AsDouble())); }

static PyValue builtin_math_isclose(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue(false);
	double a = args[0].AsDouble();
	double b = args[1].AsDouble();
	double rel_tol = 1e-09;
	double abs_tol = 0.0;
	if(args.GetCount() >= 3) rel_tol = args[2].AsDouble();
	if(args.GetCount() >= 4) abs_tol = args[3].AsDouble();
	if(std::isinf(a) || std::isinf(b)) return PyValue(a == b);
	return PyValue(std::abs(a - b) <= std::max(rel_tol * std::max(std::abs(a), std::abs(b)), abs_tol));
}

static PyValue builtin_math_isqrt(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0);
	int64 n = args[0].AsInt64();
	if(n < 0) return PyValue(0); // Should really raise ValueError
	if(n == 0) return PyValue(0);
	int64 x = (int64)sqrt((double)n);
	if((x + 1) * (x + 1) <= n) x++;
	else if(x * x > n) x--;
	return PyValue(x);
}

static int64 py_gcd(int64 a, int64 b) {
	while(b) { a %= b; Swap(a, b); }
	return a;
}

static PyValue builtin_math_lcm(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0);
	int64 res = std::abs(args[0].AsInt64());
	for(int i = 1; i < args.GetCount(); i++) {
		int64 b = std::abs(args[i].AsInt64());
		if(res == 0 || b == 0) { res = 0; break; }
		res = (res / py_gcd(res, b)) * b;
	}
	return PyValue(res);
}

static PyValue builtin_math_ldexp(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue(0.0);
	return PyValue(ldexp(args[0].AsDouble(), (int)args[1].AsInt64()));
}

static PyValue builtin_math_frexp(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	int exp;
	double m = frexp(args[0].AsDouble(), &exp);
	PyValue res = PyValue::Tuple();
	res.Add(PyValue(m));
	res.Add(PyValue(exp));
	return res;
}

static PyValue builtin_math_modf(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	double iptr;
	double f = modf(args[0].AsDouble(), &iptr);
	PyValue res = PyValue::Tuple();
	res.Add(PyValue(f));
	res.Add(PyValue(iptr));
	return res;
}

static PyValue builtin_math_ulp(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	double x = args[0].AsDouble();
	if(std::isnan(x)) return PyValue(x);
	if(std::isinf(x)) return PyValue(std::numeric_limits<double>::infinity());
	x = std::abs(x);
	return PyValue(nextafter(x, std::numeric_limits<double>::infinity()) - x);
}

static PyValue builtin_math_comb(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue(0);
	int64 n = args[0].AsInt64();
	int64 k = args[1].AsInt64();
	if(k < 0 || k > n) return PyValue(0);
	if(k == 0 || k == n) return PyValue(1);
	if(k > n / 2) k = n - k;
	double r = 1;
	for(int64 i = 1; i <= k; i++) r = r * (n - i + 1) / i;
	return PyValue((int64)(r + 0.5));
}

static PyValue builtin_math_perm(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0);
	int64 n = args[0].AsInt64();
	int64 k = n;
	if(args.GetCount() >= 2) k = args[1].AsInt64();
	if(k < 0 || k > n) return PyValue(0);
	int64 r = 1;
	for(int64 i = 0; i < k; i++) r *= (n - i);
	return PyValue(r);
}

static PyValue builtin_math_dist(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue(0.0);
	const Vector<PyValue>& p = args[0].GetArray();
	const Vector<PyValue>& q = args[1].GetArray();
	int n = std::min(p.GetCount(), q.GetCount());
	double s2 = 0;
	for(int i = 0; i < n; i++) {
		double d = p[i].AsDouble() - q[i].AsDouble();
		s2 += d * d;
	}
	return PyValue(sqrt(s2));
}

static PyValue builtin_math_prod(const Vector<PyValue>& args, void*) {
	if(args.GetCount() == 0) return PyValue(1.0);
	const Vector<PyValue> *items = &args;
	double start = 1.0;
	if(args.GetCount() >= 1 && (args[0].GetType() == PY_LIST || args[0].GetType() == PY_TUPLE)) {
		items = &args[0].GetArray();
		if(args.GetCount() >= 2) start = args[1].AsDouble();
	}
	double p = start;
	for(const auto& v : *items) p *= v.AsDouble();
	return PyValue(p);
}

static PyValue builtin_math_sumprod(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue(0.0);
	const Vector<PyValue>& p = args[0].GetArray();
	const Vector<PyValue>& q = args[1].GetArray();
	int n = std::min(p.GetCount(), q.GetCount());
	double s = 0;
	for(int i = 0; i < n; i++) s += p[i].AsDouble() * q[i].AsDouble();
	return PyValue(s);
}

static PyValue builtin_math_sqrt(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	return PyValue(sqrt(args[0].AsDouble()));
}

static PyValue builtin_math_pow(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue(0.0);
	return PyValue(pow(args[0].AsDouble(), args[1].AsDouble()));
}

static PyValue builtin_math_isinf(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(false);
	return PyValue(std::isinf(args[0].AsDouble()));
}

static PyValue builtin_math_isnan(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(false);
	return PyValue(std::isnan(args[0].AsDouble()));
}

static PyValue builtin_math_isfinite(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(false);
	return PyValue(std::isfinite(args[0].AsDouble()));
}

static PyValue builtin_math_gcd(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue(0);
	return PyValue(py_gcd(args[0].AsInt64(), args[1].AsInt64()));
}

static PyValue builtin_math_factorial(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0);
	int64 n = args[0].AsInt64();
	if(n < 0) return PyValue(0);
	int64 r = 1;
	for(int64 i = 2; i <= n; i++) r *= i;
	return PyValue(r);
}

static PyValue builtin_math_hypot(const Vector<PyValue>& args, void*) {
	double s2 = 0;
	for(const auto& v : args) {
		double d = v.AsDouble();
		s2 += d * d;
	}
	return PyValue(sqrt(s2));
}

static PyValue builtin_math_trunc(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0);
	return PyValue((int64)args[0].AsDouble());
}

static PyValue builtin_math_fsum(const Vector<PyValue>& args, void*) {
	if(args.GetCount() == 0) return PyValue(0.0);
	const Vector<PyValue> *items = &args;
	if(args.GetCount() == 1 && (args[0].GetType() == PY_LIST || args[0].GetType() == PY_TUPLE))
		items = &args[0].GetArray();
	double s = 0;
	for(const auto& v : *items) s += v.AsDouble();
	return PyValue(s);
}

static PyValue builtin_math_asin(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	return PyValue(asin(args[0].AsDouble()));
}

static PyValue builtin_math_acos(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	return PyValue(acos(args[0].AsDouble()));
}

static PyValue builtin_math_atan(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	return PyValue(atan(args[0].AsDouble()));
}

static PyValue builtin_math_atan2(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue(0.0);
	return PyValue(atan2(args[0].AsDouble(), args[1].AsDouble()));
}

static PyValue builtin_math_sin(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	return PyValue(sin(args[0].AsDouble()));
}

static PyValue builtin_math_cos(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	return PyValue(cos(args[0].AsDouble()));
}

static PyValue builtin_math_tan(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	return PyValue(tan(args[0].AsDouble()));
}

static PyValue builtin_math_radians(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	return PyValue(args[0].AsDouble() * M_PI / 180.0);
}

static PyValue builtin_math_degrees(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	return PyValue(args[0].AsDouble() * 180.0 / M_PI);
}

static PyValue builtin_math_exp(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	return PyValue(exp(args[0].AsDouble()));
}

static PyValue builtin_math_log(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	if(args.GetCount() >= 2) return PyValue(log(args[0].AsDouble()) / log(args[1].AsDouble()));
	return PyValue(log(args[0].AsDouble()));
}

static PyValue builtin_math_ceil(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	return PyValue(ceil(args[0].AsDouble()));
}

static PyValue builtin_math_floor(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue(0.0);
	return PyValue(floor(args[0].AsDouble()));
}

static PyValue builtin_time_time(const Vector<PyValue>& args, void*) {
	return PyValue((double)(GetUtcTime() - Time(1970, 1, 1)));
}

static PyValue builtin_time_sleep(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::None();
	Upp::Sleep((int)(args[0].AsDouble() * 1000));
	return PyValue::None();
}

static PyValue builtin_time_ctime(const Vector<PyValue>& args, void*) {
	Time t = GetSysTime();
	if(args.GetCount() >= 1) t = Time(1970, 1, 1) + (int64)args[0].AsDouble();
	return PyValue(Format(t));
}

static PyValue builtin_time_perf_counter(const Vector<PyValue>& args, void*) {
	static int64 start = msecs();
	return PyValue((double)(msecs() - start) / 1000.0);
}

static PyValue time_to_tuple(Time t) {
	PyValue res = PyValue::Tuple();
	res.Add(PyValue(t.year));
	res.Add(PyValue(t.month));
	res.Add(PyValue(t.day));
	res.Add(PyValue(t.hour));
	res.Add(PyValue(t.minute));
	res.Add(PyValue(t.second));
	res.Add(PyValue(DayOfWeek(t))); // tm_wday
	res.Add(PyValue(DayOfYear(t))); // tm_yday
	res.Add(PyValue(-1)); // tm_isdst
	return res;
}

static PyValue builtin_time_gmtime(const Vector<PyValue>& args, void*) {
	Time t = GetUtcTime();
	if(args.GetCount() >= 1) t = Time(1970, 1, 1) + (int64)args[0].AsDouble();
	return time_to_tuple(t);
}

static PyValue builtin_time_localtime(const Vector<PyValue>& args, void*) {
	Time t = GetSysTime();
	if(args.GetCount() >= 1) {
		t = Time(1970, 1, 1) + (int64)args[0].AsDouble(); // Not perfect as it doesn't account for local TZ properly but okay for now
	}
	return time_to_tuple(t);
}

static PyValue builtin_json_dumps(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue("");
	bool pretty = false;
	if(args.GetCount() >= 2) {
		if(args[1].IsInt() && args[1].AsInt() > 0) pretty = true;
	}
	return PyValue(AsJSON(args[0].ToValue(), pretty));
}

static PyValue builtin_json_loads(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::None();
	return PyValue::FromValue(ParseJSON(args[0].ToString()));
}

static PyValue builtin_json_dump(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue::None();
	SaveFile(args[1].ToString(), AsJSON(args[0].ToValue()));
	return PyValue::None();
}

static PyValue builtin_json_load(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::None();
	return PyValue::FromValue(ParseJSON(LoadFile(args[0].ToString())));
}

PyVM::PyVM()
{
	globals.GetAdd(PyValue("__name__")) = PyValue("__main__");

	PyValue p_print = PyValue::Function("print");
	p_print.GetLambdaRW().builtin = builtin_print;
	globals.GetAdd(PyValue("print")) = p_print;

	PyValue p_len = PyValue::Function("len");
	p_len.GetLambdaRW().builtin = builtin_len;
	globals.GetAdd(PyValue("len")) = p_len;
	
	PyValue p_range = PyValue::Function("range");
	p_range.GetLambdaRW().builtin = builtin_range;
	globals.GetAdd(PyValue("range")) = p_range;

	PyValue p_complex = PyValue::Function("complex");
	p_complex.GetLambdaRW().builtin = builtin_complex;
	globals.GetAdd(PyValue("complex")) = p_complex;

	PyValue p_bool = PyValue::Function("bool");
	p_bool.GetLambdaRW().builtin = builtin_bool;
	globals.GetAdd(PyValue("bool")) = p_bool;

	PyValue p_str = PyValue::Function("str");
	p_str.GetLambdaRW().builtin = builtin_str;
	globals.GetAdd(PyValue("str")) = p_str;


	PyValue p_iter = PyValue::Function("iter");
	p_iter.GetLambdaRW().builtin = builtin_iter;
	globals.GetAdd(PyValue("iter")) = p_iter;

	PyValue p_next = PyValue::Function("next");
	p_next.GetLambdaRW().builtin = builtin_next;
	globals.GetAdd(PyValue("next")) = p_next;

	PyValue p_dir = PyValue::Function("dir");
	p_dir.GetLambdaRW().builtin = builtin_dir;
	globals.GetAdd(PyValue("dir")) = p_dir;

	PyValue p_min = PyValue::Function("min");
	p_min.GetLambdaRW().builtin = builtin_min;
	globals.GetAdd(PyValue("min")) = p_min;

	PyValue p_max = PyValue::Function("max");
	p_max.GetLambdaRW().builtin = builtin_max;
	globals.GetAdd(PyValue("max")) = p_max;

	PyValue p_sum = PyValue::Function("sum");
	p_sum.GetLambdaRW().builtin = builtin_sum;
	globals.GetAdd(PyValue("sum")) = p_sum;

	PyValue p_abs = PyValue::Function("abs");
	p_abs.GetLambdaRW().builtin = builtin_abs;
	globals.GetAdd(PyValue("abs")) = p_abs;

	// os module
	PyValue os = PyValue::Dict();
	os.SetItem(PyValue("getcwd"), PyValue::Function("getcwd", builtin_os_getcwd));
	os.SetItem(PyValue("mkdir"), PyValue::Function("mkdir", builtin_os_mkdir));
	os.SetItem(PyValue("rmdir"), PyValue::Function("rmdir", builtin_os_rmdir));
	os.SetItem(PyValue("rename"), PyValue::Function("rename", builtin_os_rename));
	os.SetItem(PyValue("getenv"), PyValue::Function("getenv", builtin_os_getenv));
	os.SetItem(PyValue("putenv"), PyValue::Function("putenv", builtin_os_putenv));
	os.SetItem(PyValue("getpid"), PyValue::Function("getpid", builtin_os_getpid));
	os.SetItem(PyValue("getuid"), PyValue::Function("getuid", builtin_os_getuid));
	os.SetItem(PyValue("getgid"), PyValue::Function("getgid", builtin_os_getgid));
	os.SetItem(PyValue("listdir"), PyValue::Function("listdir", builtin_os_listdir));
	os.SetItem(PyValue("remove"), PyValue::Function("remove", builtin_os_remove));
	os.SetItem(PyValue("chdir"), PyValue::Function("chdir", builtin_os_chdir));
	os.SetItem(PyValue("getlogin"), PyValue::Function("getlogin", builtin_os_getlogin));
	os.SetItem(PyValue("getppid"), PyValue::Function("getppid", builtin_os_getppid));
	os.SetItem(PyValue("makedirs"), PyValue::Function("makedirs", builtin_os_makedirs));
	os.SetItem(PyValue("removedirs"), PyValue::Function("removedirs", builtin_os_removedirs));
	os.SetItem(PyValue("replace"), PyValue::Function("replace", builtin_os_replace));
	os.SetItem(PyValue("truncate"), PyValue::Function("truncate", builtin_os_truncate));
	os.SetItem(PyValue("system"), PyValue::Function("system", builtin_os_system));
	os.SetItem(PyValue("cpu_count"), PyValue::Function("cpu_count", builtin_os_cpu_count));
	os.SetItem(PyValue("urandom"), PyValue::Function("urandom", builtin_os_urandom));
	os.SetItem(PyValue("readlink"), PyValue::Function("readlink", builtin_os_readlink));
	os.SetItem(PyValue("symlink"), PyValue::Function("symlink", builtin_os_symlink));
	os.SetItem(PyValue("abort"), PyValue::Function("abort", builtin_os_abort));
	os.SetItem(PyValue("access"), PyValue::Function("access", builtin_os_access));
	os.SetItem(PyValue("chmod"), PyValue::Function("chmod", builtin_os_chmod));
	os.SetItem(PyValue("uname"), PyValue::Function("uname", builtin_os_uname));
	
#ifdef flagWIN32
	os.SetItem(PyValue("name"), PyValue("nt"));
	os.SetItem(PyValue("sep"), PyValue("\\"));
	os.SetItem(PyValue("altsep"), PyValue("/"));
	os.SetItem(PyValue("extsep"), PyValue("."));
	os.SetItem(PyValue("pathsep"), PyValue(";"));
	os.SetItem(PyValue("linesep"), PyValue("\r\n"));
	os.SetItem(PyValue("devnull"), PyValue("nul"));
#else
	os.SetItem(PyValue("name"), PyValue("posix"));
	os.SetItem(PyValue("sep"), PyValue("/"));
	os.SetItem(PyValue("altsep"), PyValue::None());
	os.SetItem(PyValue("extsep"), PyValue("."));
	os.SetItem(PyValue("pathsep"), PyValue(":"));
	os.SetItem(PyValue("linesep"), PyValue("\n"));
	os.SetItem(PyValue("devnull"), PyValue("/dev/null"));
#endif
	
	PyValue environ = PyValue::Dict();
	const auto& env = Environment();
	for(int i = 0; i < env.GetCount(); i++)
		environ.SetItem(PyValue(env.GetKey(i)), PyValue(env[i]));
	os.SetItem(PyValue("environ"), environ);
	
	PyValue os_path = PyValue::Dict();
	os_path.SetItem(PyValue("exists"), PyValue::Function("exists", builtin_os_path_exists));
	os_path.SetItem(PyValue("isdir"), PyValue::Function("isdir", builtin_os_path_isdir));
	os_path.SetItem(PyValue("isfile"), PyValue::Function("isfile", builtin_os_path_isfile));
	os_path.SetItem(PyValue("islink"), PyValue::Function("islink", builtin_os_path_islink));
	os_path.SetItem(PyValue("lexists"), PyValue::Function("lexists", builtin_os_path_lexists));
	os_path.SetItem(PyValue("getsize"), PyValue::Function("getsize", builtin_os_path_getsize));
	os_path.SetItem(PyValue("getatime"), PyValue::Function("getatime", builtin_os_path_getatime));
	os_path.SetItem(PyValue("getmtime"), PyValue::Function("getmtime", builtin_os_path_getmtime));
	os_path.SetItem(PyValue("getctime"), PyValue::Function("getctime", builtin_os_path_getctime));
	os_path.SetItem(PyValue("join"), PyValue::Function("join", builtin_os_path_join));
	os_path.SetItem(PyValue("abspath"), PyValue::Function("abspath", builtin_os_path_abspath));
	os_path.SetItem(PyValue("realpath"), PyValue::Function("realpath", builtin_os_path_realpath));
	os_path.SetItem(PyValue("relpath"), PyValue::Function("relpath", builtin_os_path_relpath));
	os_path.SetItem(PyValue("samefile"), PyValue::Function("samefile", builtin_os_path_samefile));
	os_path.SetItem(PyValue("basename"), PyValue::Function("basename", builtin_os_path_basename));
	os_path.SetItem(PyValue("dirname"), PyValue::Function("dirname", builtin_os_path_dirname));
	os_path.SetItem(PyValue("split"), PyValue::Function("split", builtin_os_path_split));
	os_path.SetItem(PyValue("splitext"), PyValue::Function("splitext", builtin_os_path_splitext));
	os_path.SetItem(PyValue("isabs"), PyValue::Function("isabs", builtin_os_path_isabs));
	os_path.SetItem(PyValue("normpath"), PyValue::Function("normpath", builtin_os_path_normpath));
	os_path.SetItem(PyValue("normcase"), PyValue::Function("normcase", builtin_os_path_normcase));
	
os_path.SetItem(PyValue("sep"), os.GetItem(PyValue("sep")));
	os_path.SetItem(PyValue("altsep"), os.GetItem(PyValue("altsep")));
	os_path.SetItem(PyValue("extsep"), os.GetItem(PyValue("extsep")));
	os_path.SetItem(PyValue("pathsep"), os.GetItem(PyValue("pathsep")));

	os.SetItem(PyValue("path"), os_path);
	globals.GetAdd(PyValue("os")) = os;

	// sys module
	PyValue sys = PyValue::Dict();
	sys.SetItem(PyValue("exit"), PyValue::Function("exit", builtin_sys_exit));
	sys.SetItem(PyValue("executable"), PyValue(GetExeFilePath()));
	
	PyValue argv = PyValue::List();
	const Vector<String>& cmd = CommandLine();
	for(int i = 0; i < cmd.GetCount(); i++)
		argv.Add(PyValue(cmd[i]));
	sys.SetItem(PyValue("argv"), argv);

	sys.SetItem(PyValue("path"), PyValue::List());
	sys.SetItem(PyValue("modules"), PyValue::Dict());
	
#ifdef CPU_LE
	sys.SetItem(PyValue("byteorder"), PyValue("little"));
#else
	sys.SetItem(PyValue("byteorder"), PyValue("big"));
#endif
	
#ifdef flagWIN32
	sys.SetItem(PyValue("platform"), PyValue("win32"));
#elif defined(flagLINUX)
	sys.SetItem(PyValue("platform"), PyValue("linux"));
#elif defined(flagMACOS)
	sys.SetItem(PyValue("platform"), PyValue("darwin"));
#else
	sys.SetItem(PyValue("platform"), PyValue("unknown"));
#endif

	sys.SetItem(PyValue("version"), PyValue("0.1v (Uppy)"));
	
globals.GetAdd(PyValue("sys")) = sys;

	// math module
	PyValue math = PyValue::Dict();
	math.SetItem(PyValue("__name__"), PyValue("math"));
	math.SetItem(PyValue("__doc__"), PyValue("This module provides access to the mathematical functions defined by the C standard."));
	math.SetItem(PyValue("__package__"), PyValue(""));
	math.SetItem(PyValue("__loader__"), PyValue::None());
	math.SetItem(PyValue("__spec__"), PyValue::None());
	math.SetItem(PyValue("__file__"), PyValue("built-in"));

	math.SetItem(PyValue("sqrt"), PyValue::Function("sqrt", builtin_math_sqrt));
	math.SetItem(PyValue("cbrt"), PyValue::Function("cbrt", builtin_math_cbrt));
	math.SetItem(PyValue("exp2"), PyValue::Function("exp2", builtin_math_exp2));
	math.SetItem(PyValue("isclose"), PyValue::Function("isclose", builtin_math_isclose));
	math.SetItem(PyValue("isqrt"), PyValue::Function("isqrt", builtin_math_isqrt));
	math.SetItem(PyValue("lcm"), PyValue::Function("lcm", builtin_math_lcm));
	math.SetItem(PyValue("ldexp"), PyValue::Function("ldexp", builtin_math_ldexp));
	math.SetItem(PyValue("frexp"), PyValue::Function("frexp", builtin_math_frexp));
	math.SetItem(PyValue("modf"), PyValue::Function("modf", builtin_math_modf));
	math.SetItem(PyValue("ulp"), PyValue::Function("ulp", builtin_math_ulp));
	math.SetItem(PyValue("comb"), PyValue::Function("comb", builtin_math_comb));
	math.SetItem(PyValue("perm"), PyValue::Function("perm", builtin_math_perm));
	math.SetItem(PyValue("dist"), PyValue::Function("dist", builtin_math_dist));
	math.SetItem(PyValue("prod"), PyValue::Function("prod", builtin_math_prod));
	math.SetItem(PyValue("sumprod"), PyValue::Function("sumprod", builtin_math_sumprod));
	math.SetItem(PyValue("fabs"), PyValue::Function("fabs", builtin_math_fabs));
	math.SetItem(PyValue("asin"), PyValue::Function("asin", builtin_math_asin));
	math.SetItem(PyValue("acos"), PyValue::Function("acos", builtin_math_acos));
	math.SetItem(PyValue("atan"), PyValue::Function("atan", builtin_math_atan));
	math.SetItem(PyValue("atan2"), PyValue::Function("atan2", builtin_math_atan2));
	math.SetItem(PyValue("asinh"), PyValue::Function("asinh", builtin_math_asinh));
	math.SetItem(PyValue("acosh"), PyValue::Function("acosh", builtin_math_acosh));
	math.SetItem(PyValue("atanh"), PyValue::Function("atanh", builtin_math_atanh));
	math.SetItem(PyValue("sin"), PyValue::Function("sin", builtin_math_sin));
	math.SetItem(PyValue("cos"), PyValue::Function("cos", builtin_math_cos));
	math.SetItem(PyValue("tan"), PyValue::Function("tan", builtin_math_tan));
	math.SetItem(PyValue("sinh"), PyValue::Function("sinh", builtin_math_sinh));
	math.SetItem(PyValue("cosh"), PyValue::Function("cosh", builtin_math_cosh));
	math.SetItem(PyValue("tanh"), PyValue::Function("tanh", builtin_math_tanh));
	math.SetItem(PyValue("radians"), PyValue::Function("radians", builtin_math_radians));
	math.SetItem(PyValue("degrees"), PyValue::Function("degrees", builtin_math_degrees));
	math.SetItem(PyValue("exp"), PyValue::Function("exp", builtin_math_exp));
	math.SetItem(PyValue("expm1"), PyValue::Function("expm1", builtin_math_expm1));
	math.SetItem(PyValue("log"), PyValue::Function("log", builtin_math_log));
	math.SetItem(PyValue("log10"), PyValue::Function("log10", builtin_math_log10));
	math.SetItem(PyValue("log2"), PyValue::Function("log2", builtin_math_log2));
	math.SetItem(PyValue("log1p"), PyValue::Function("log1p", builtin_math_log1p));
	math.SetItem(PyValue("ceil"), PyValue::Function("ceil", builtin_math_ceil));
	math.SetItem(PyValue("floor"), PyValue::Function("floor", builtin_math_floor));
	math.SetItem(PyValue("pow"), PyValue::Function("pow", builtin_math_pow));
	math.SetItem(PyValue("isinf"), PyValue::Function("isinf", builtin_math_isinf));
	math.SetItem(PyValue("isnan"), PyValue::Function("isnan", builtin_math_isnan));
	math.SetItem(PyValue("isfinite"), PyValue::Function("isfinite", builtin_math_isfinite));
	math.SetItem(PyValue("gcd"), PyValue::Function("gcd", builtin_math_gcd));
	math.SetItem(PyValue("factorial"), PyValue::Function("factorial", builtin_math_factorial));
	math.SetItem(PyValue("hypot"), PyValue::Function("hypot", builtin_math_hypot));
	math.SetItem(PyValue("trunc"), PyValue::Function("trunc", builtin_math_trunc));
	math.SetItem(PyValue("fsum"), PyValue::Function("fsum", builtin_math_fsum));
	math.SetItem(PyValue("fmod"), PyValue::Function("fmod", builtin_math_fmod));
	math.SetItem(PyValue("remainder"), PyValue::Function("remainder", builtin_math_remainder));
	math.SetItem(PyValue("copysign"), PyValue::Function("copysign", builtin_math_copysign));
	math.SetItem(PyValue("nextafter"), PyValue::Function("nextafter", builtin_math_nextafter));
	math.SetItem(PyValue("erf"), PyValue::Function("erf", builtin_math_erf));
	math.SetItem(PyValue("erfc"), PyValue::Function("erfc", builtin_math_erfc));
	math.SetItem(PyValue("gamma"), PyValue::Function("gamma", builtin_math_gamma));
	math.SetItem(PyValue("lgamma"), PyValue::Function("lgamma", builtin_math_lgamma));
	math.SetItem(PyValue("pi"), PyValue(M_PI));
	math.SetItem(PyValue("e"), PyValue(M_E));
	math.SetItem(PyValue("tau"), PyValue(2.0 * M_PI));
	math.SetItem(PyValue("inf"), PyValue(std::numeric_limits<double>::infinity()));
	math.SetItem(PyValue("nan"), PyValue(std::numeric_limits<double>::quiet_NaN()));
	globals.GetAdd(PyValue("math")) = math;

	// time module
	PyValue time = PyValue::Dict();
	time.SetItem(PyValue("time"), PyValue::Function("time", builtin_time_time));
	time.SetItem(PyValue("sleep"), PyValue::Function("sleep", builtin_time_sleep));
	time.SetItem(PyValue("ctime"), PyValue::Function("ctime", builtin_time_ctime));
	time.SetItem(PyValue("perf_counter"), PyValue::Function("perf_counter", builtin_time_perf_counter));
	time.SetItem(PyValue("gmtime"), PyValue::Function("gmtime", builtin_time_gmtime));
	time.SetItem(PyValue("localtime"), PyValue::Function("localtime", builtin_time_localtime));
	globals.GetAdd(PyValue("time")) = time;

	// json module
	PyValue json = PyValue::Dict();
	json.SetItem(PyValue("dumps"), PyValue::Function("dumps", builtin_json_dumps));
	json.SetItem(PyValue("loads"), PyValue::Function("loads", builtin_json_loads));
	json.SetItem(PyValue("dump"), PyValue::Function("dump", builtin_json_dump));
	json.SetItem(PyValue("load"), PyValue::Function("load", builtin_json_load));
	globals.GetAdd(PyValue("json")) = json;
	
	// subprocess module
	PyValue subprocess = PyValue::Dict();
	subprocess.SetItem(PyValue("run"), PyValue::Function("run", builtin_subprocess_run));
	globals.GetAdd(PyValue("subprocess")) = subprocess;
}

void PyVM::SetIR(Vector<PyIR>& _ir)
{
	frames.Clear();
	Frame& f = frames.Add();
	f.func = PyValue::Function("__main__");
	f.func.GetLambdaRW().ir = pick(_ir);
	f.ir = &f.func.GetLambda().ir;
	f.pc = 0;
}

PyValue PyVM::Run()
{
	PyValue last_result = PyValue::None();
	while(!frames.IsEmpty()) {
		Frame& frame = TopFrame();
		if(frame.pc >= frame.ir->GetCount()) {
			frames.Drop();
			continue;
		}
		
		const PyIR& instr = (*frame.ir)[frame.pc++];
		
		try {
			switch(instr.code) {
			case PY_NOP: break;
			
			case PY_UNARY_POSITIVE: break; // Identity
			
			case PY_UNARY_NEGATIVE: {
				PyValue v = Pop();
				if (v.IsInt()) Push(PyValue(-v.AsInt64()));
				else if (v.IsFloat()) Push(PyValue(-v.AsDouble()));
				break;
			}
	
			case PY_UNARY_NOT: {
				PyValue v = Pop();
				Push(PyValue(!v.IsTrue()));
				break;
			}
	
			case PY_UNARY_INVERT: {
				PyValue v = Pop();
				if (v.IsInt()) Push(PyValue(~v.AsInt64()));
				break;
			}
	
			case PY_POP_TOP:
				last_result = Pop();
				break;
	
			case PY_LOAD_CONST:
				Push(instr.arg);
				break;
			
			case PY_LOAD_NAME: {
				int q = frame.locals.Find(instr.arg);
				if(q >= 0) Push(frame.locals[q]);
				else {
					q = globals.Find(instr.arg);
					if(q >= 0) Push(globals[q]);
					else throw Exc("NameError: name '" + instr.arg.ToString() + "' is not defined");
				}
				break;
			}
		
			case PY_STORE_NAME:
				if(frames.GetCount() <= 1)
					globals.GetAdd(instr.arg) = Pop();
				else
					frame.locals.GetAdd(instr.arg) = Pop();
				break;
	
			case PY_LOAD_GLOBAL: {
				int q = globals.Find(instr.arg);
				if(q >= 0) Push(globals[q]);
				else throw Exc("NameError: name '" + instr.arg.ToString() + "' is not defined");
				break;
			}
				
			case PY_STORE_GLOBAL:
				globals.GetAdd(instr.arg) = Pop();
				break;

			case PY_IMPORT_NAME: {
				String name = instr.arg.ToString();
				// Support dotted names like os.path
				Vector<String> parts = Split(name, '.');
				PyValue current;
				
				// Check first part in globals
				int q = globals.Find(parts[0]);
				if(q >= 0) {
					current = globals[q];
				}
				else {
					// Check sys.modules
					PyValue sys = globals.Get(PyValue("sys"), PyValue::None());
					if(sys.GetType() == PY_DICT) {
						PyValue modules = sys.GetItem(PyValue("modules"));
						if(modules.GetType() == PY_DICT) {
							current = modules.GetItem(PyValue(parts[0]));
						}
					}
				}
				
				if(!current.IsNone()) {
					for(int i = 1; i < parts.GetCount(); i++) {
						if(current.GetType() == PY_DICT) {
							current = current.GetItem(PyValue(parts[i]));
						} else {
							current = PyValue::None();
							break;
						}
					}
				}
				
				Push(current);
				break;
			}

			case PY_IMPORT_FROM: {
				PyValue name = instr.arg;
				PyValue mod = stack.Top();
				if (mod.GetType() == PY_DICT) {
					Push(mod.GetItem(name));
				} else if (mod.IsUserDataValid()) {
					Push(mod.GetUserData().GetAttr(name.ToString()));
				} else {
					Push(PyValue::None());
				}
				break;
			}

			case PY_IMPORT_STAR: {
				PyValue mod = Pop();
				if(mod.GetType() == PY_DICT) {
					const VectorMap<PyValue, PyValue>& m = mod.GetDict();
					for(int i = 0; i < m.GetCount(); i++) {
						String name = m.GetKey(i).ToString();
						if(!name.StartsWith("_")) {
							globals.GetAdd(m.GetKey(i)) = m[i];
						}
					}
				}
				break;
			}
	
			case PY_LOAD_ATTR: {				PyValue obj = Pop();
				if (obj.GetType() == PY_DICT) {
					Push(obj.GetItem(instr.arg));
				} else if (obj.GetType() == PY_STR) {
					String attr = instr.arg.ToString();
					if(attr == "endswith") {
						Push(PyValue::BoundMethod(PyValue::Function("endswith", builtin_str_endswith), obj));
					} else if(attr == "join") {
						Push(PyValue::BoundMethod(PyValue::Function("join", builtin_str_join), obj));
					} else {
						Push(PyValue::None());
					}
				} else if (obj.IsUserDataValid()) {
					String attr_name = instr.arg.ToString();
					RTLOG("PY_LOAD_ATTR: obj=" << obj.ToString() << " attr=" << attr_name);
					Push(obj.GetUserData().GetAttr(attr_name));
				} else {
					Push(PyValue::None());
				}
				break;
			}
	
			case PY_STORE_ATTR: {
				PyValue val = Pop();
				PyValue obj = Pop();
				if (obj.GetType() == PY_DICT) {
					obj.SetItem(instr.arg, val);
				} // TODO: support UserData store
				break;
			}
				
			case PY_BUILD_LIST: {
				int n = instr.iarg;
				PyValue list = PyValue::List();
				Vector<PyValue> items;
				for(int i = 0; i < n; i++) items.Add(Pop());
				for(int i = n - 1; i >= 0; i--) list.Add(items[i]);
				Push(list);
				break;
			}
			
			case PY_BUILD_TUPLE: {
				int n = instr.iarg;
				PyValue tuple = PyValue::Tuple();
				Vector<PyValue> items;
				for(int i = 0; i < n; i++) items.Add(Pop());
				for(int i = n - 1; i >= 0; i--) tuple.Add(items[i]);
				Push(tuple);
				break;
			}
			
			case PY_BUILD_MAP: {
				int n = instr.iarg;
				PyValue dict = PyValue::Dict();
				Vector<PyKV> items;
				for(int i = 0; i < n; i++) {
					PyValue v = Pop();
					PyValue k = Pop();
					items.Add(PyKV(k, v));
				}
				for(int i = n - 1; i >= 0; i--) dict.SetItem(items[i].k, items[i].v);
				Push(dict);
				break;
			}
				
			case PY_BINARY_ADD: {
				PyValue b = Pop();
				PyValue a = Pop();
				if(a.GetType() == PY_COMPLEX || b.GetType() == PY_COMPLEX)
					Push(PyValue(a.GetComplex() + b.GetComplex()));
				else if(a.IsInt() && b.IsInt()) Push(PyValue(a.AsInt64() + b.AsInt64()));
				else if(a.IsNumber() && b.IsNumber()) Push(PyValue(a.AsDouble() + b.AsDouble()));
				else if(a.GetType() == PY_STR && b.GetType() == PY_STR) Push(PyValue(a.GetStr() + b.GetStr()));
				else if(a.GetType() == PY_LIST && b.GetType() == PY_LIST) {
					PyValue res = PyValue::List();
					const Vector<PyValue>& va = a.GetArray();
					const Vector<PyValue>& vb = b.GetArray();
					for(int i = 0; i < va.GetCount(); i++) res.Add(va[i]);
					for(int i = 0; i < vb.GetCount(); i++) res.Add(vb[i]);
					Push(res);
				}
				break;
			}
	
			case PY_BINARY_SUBTRACT: {
				PyValue b = Pop();
				PyValue a = Pop();
				if(a.GetType() == PY_COMPLEX || b.GetType() == PY_COMPLEX)
					Push(PyValue(a.GetComplex() - b.GetComplex()));
				else if(a.IsInt() && b.IsInt()) Push(PyValue(a.AsInt64() - b.AsInt64()));
				else Push(PyValue(a.AsDouble() - b.AsDouble()));
				break;
			}
	
			case PY_BINARY_MULTIPLY: {
				PyValue b = Pop();
				PyValue a = Pop();
				if(a.GetType() == PY_COMPLEX || b.GetType() == PY_COMPLEX)
					Push(PyValue(a.GetComplex() * b.GetComplex()));
				else if(a.IsInt() && b.IsInt()) Push(PyValue(a.AsInt64() * b.AsInt64()));
				else Push(PyValue(a.AsDouble() * b.AsDouble()));
				break;
			}
			
			case PY_BINARY_MODULO: {
				PyValue b = Pop();
				PyValue a = Pop();
				if(a.GetType() == PY_STR) {
					String fmt = a.ToString();
					if(b.GetType() == PY_TUPLE || b.GetType() == PY_LIST) {
						// Very basic multiple arg formatting
						String res;
						int arg_idx = 0;
						for(int i = 0; i < fmt.GetCount(); i++) {
							if(fmt[i] == '%' && i + 1 < fmt.GetCount() && arg_idx < b.GetCount()) {
								char spec = fmt[i+1];
								if(spec == 's' || spec == 'd' || spec == 'g') {
									res << b.GetItem(arg_idx++).ToString();
									i++;
									continue;
								}
							}
							res.Cat(fmt[i]);
						}
						Push(PyValue(res));
					} else {
						// Single arg formatting
						String res;
						for(int i = 0; i < fmt.GetCount(); i++) {
							if(fmt[i] == '%' && i + 1 < fmt.GetCount()) {
								char spec = fmt[i+1];
								if(spec == 's' || spec == 'd' || spec == 'g') {
									res << b.ToString();
									i++;
									continue;
								}
							}
							res.Cat(fmt[i]);
						}
						Push(PyValue(res));
					}
				} else if(a.IsInt() && b.IsInt()) {
					if(b.AsInt64() == 0) throw Exc("ZeroDivisionError: integer modulo by zero");
					Push(PyValue(a.AsInt64() % b.AsInt64()));
				} else {
					Push(PyValue(fmod(a.AsDouble(), b.AsDouble())));
				}
				break;
			}
			
			case PY_BINARY_TRUE_DIVIDE: {
				PyValue b = Pop();
				PyValue a = Pop();
				if (b.AsDouble() == 0) throw Exc("ZeroDivisionError: division by zero");
				Push(PyValue(a.AsDouble() / b.AsDouble()));
				break;
			}
			case PY_BINARY_SUBSCR: {
				PyValue sub = Pop();
				PyValue container = Pop();
				if(sub.IsInt() && (container.GetType() == PY_LIST || container.GetType() == PY_TUPLE)) {
					int idx = sub.AsInt();
					if (idx < 0 || idx >= container.GetCount()) throw Exc("IndexError: list index out of range");
					Push(container.GetItem(idx));
				}
				else
					Push(container.GetItem(sub));
				break;
			}
	
			case PY_STORE_SUBSCR: {
				PyValue val = Pop();
				PyValue sub = Pop();
				PyValue obj = Pop();
				if(sub.IsInt() && obj.GetType() == PY_LIST)
					obj.SetItem(sub.AsInt(), val);
				else
					obj.SetItem(sub, val);
				break;
			}
	
			case PY_COMPARE_OP: {
				PyValue b = Pop();
				PyValue a = Pop();
				bool res = false;
				switch(instr.iarg) {
				case PY_CMP_EQ: res = (a == b); break;
				case PY_CMP_NE: res = (a != b); break;
				case PY_CMP_LT: res = (a < b); break;
				case PY_CMP_LE: res = (a < b || a == b); break;
				case PY_CMP_GT: res = (b < a); break;
				case PY_CMP_GE: res = (b < a || a == b); break;
				}
				Push(PyValue(res));
				break;
			}
	
			case PY_JUMP_ABSOLUTE:
				frame.pc = instr.iarg;
				break;
				
			case PY_POP_JUMP_IF_FALSE: {
				PyValue v = Pop();
				if(!v.IsTrue()) frame.pc = instr.iarg;
				break;
			}
	
			case PY_JUMP_IF_FALSE_OR_POP: {
				if(!stack.Top().IsTrue()) frame.pc = instr.iarg;
				else Pop();
				break;
			}
	
			case PY_JUMP_IF_TRUE_OR_POP: {
				if(stack.Top().IsTrue()) frame.pc = instr.iarg;
				else Pop();
				break;
			}
	
			case PY_CALL_FUNCTION: {
				int nargs = instr.iarg;
				Vector<PyValue> args;
				for(int i = 0; i < nargs; i++) args.Add(Pop());
				Vector<PyValue> sorted_args;
				for(int i = nargs - 1; i >= 0; i--) sorted_args.Add(args[i]);
				
				PyValue callable = Pop();
				if (callable.IsBoundMethod()) {
					PyValue func = callable.GetBound().func;
					PyValue self = callable.GetBound().self;
					sorted_args.Insert(0, self);
					callable = func;
				}
	
				if(callable.IsFunction()) {
					const PyLambda& l = callable.GetLambda();
					if(l.builtin) {
						Push(l.builtin(sorted_args, l.user_data));
					}
					else {
						Frame& f = frames.Add();
						f.func = callable;
						f.ir = &l.ir;
						f.pc = 0;
						for(int i = 0; i < min(l.arg.GetCount(), sorted_args.GetCount()); i++)
							f.locals.GetAdd(PyValue(l.arg[i])) = sorted_args[i];
					}
				}
				else {
					throw Exc("TypeError: '" + callable.ToString() + "' object is not callable");
				}
				break;
			}
	
			case PY_GET_ITER: {
				PyValue obj = Pop();
				if(obj.GetType() == PY_LIST || obj.GetType() == PY_TUPLE) {
					Push(PyValue(new PyVectorIter(obj.GetArray())));
				}
				else if(obj.IsIterator()) {
					Push(obj);
				}
				else {
					throw Exc("TypeError: '" + obj.ToString() + "' object is not iterable");
				}
				break;
			}
			
			case PY_FOR_ITER: {
				PyValue iterator = Pop();
				if(iterator.IsIterator()) {
					PyValue next_val = iterator.GetIter().Next();
					if(next_val.IsStopIteration()) {
						frame.pc = instr.iarg;
					}
					else {
						Push(iterator);
						Push(next_val);
					}
				}
				else {
					throw Exc("TypeError: '" + iterator.ToString() + "' object is not an iterator");
				}
				break;
			}
	
			case PY_LIST_APPEND: {
				PyValue val = Pop();
				int offset = instr.iarg;
				PyValue& list = stack[stack.GetCount() - offset];
				list.Add(val);
				break;
			}
	
			case PY_RETURN_VALUE: {
				PyValue val = Pop();
				frames.Drop();
				if(!frames.IsEmpty()) {
					Push(val);
				}
				else {
					if(!val.IsNone() || last_result.IsNone())
						last_result = val;
				}
				break;
			}
				
default:
				throw Exc("RuntimeError: Unknown opcode: " + AsString((int)instr.code));
			}
		} catch (Exc& e) {
			String func_name = "<module>";
			if (frame.func.IsFunction()) func_name = frame.func.GetLambda().name;
			String loc = "  File \"<stdin>\", line " + AsString(instr.line) + ", in " + func_name;
			if (e.Find("Traceback") < 0) {
				throw Exc("Traceback (most recent call last):\n" + loc + "\n" + e);
			} else {
				String msg = e;
				int q = msg.Find('\n');
				if (q >= 0) msg.Insert(q + 1, loc + "\n");
				throw Exc(msg);
			}
		}
	}
	return last_result;
}

}