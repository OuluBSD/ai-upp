#include "ByteVM.h"
#include <Core/Core.h>

namespace Upp {

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

static PyValue builtin_abs(const Vector<PyValue>& args, void*) {
	if(args.GetCount() == 0) return PyValue(0);
	PyValue v = args[0];
	if(v.IsInt()) return PyValue(std::abs(v.AsInt64()));
	if(v.IsFloat()) return PyValue(std::abs(v.AsDouble()));
	if(v.GetType() == PY_COMPLEX) return PyValue(std::abs(v.GetComplex()));
	return v;
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
	return PyValue((path.GetCount() >= 2 && IsAlpha(path[0]) && path[1] == ':') || path.StartsWith("\\\\") || path.StartsWith("/"));
#else
	return PyValue(path.StartsWith("/"));
#endif
}

static PyValue builtin_os_getlogin(const Vector<PyValue>& args, void*) {
	return PyValue(GetUserName());
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
	t.Add(PyValue(GetHostName()));
	t.Add(PyValue("")); // release
	t.Add(PyValue("")); // version
	t.Add(PyValue("")); // machine
	return t;
}

static PyValue builtin_sys_exit(const Vector<PyValue>& args, void*) {
	int code = 0;
	if(args.GetCount() >= 1) code = args[0].AsInt();
	exit(code);
	return PyValue::None();
}

static PyValue builtin_sys_executable(const Vector<PyValue>& args, void*) {
	return PyValue(GetExeFilePath());
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

static int64 py_gcd(int64 a, int64 b) {
	while(b) { a %= b; Swap(a, b); }
	return a;
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

static PyValue builtin_json_dumps(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue("");
	return PyValue(AsJSON(args[0].ToValue()));
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

	PyValue p_iter = PyValue::Function("iter");
	p_iter.GetLambdaRW().builtin = builtin_iter;
	globals.GetAdd(PyValue("iter")) = p_iter;

	PyValue p_next = PyValue::Function("next");
	p_next.GetLambdaRW().builtin = builtin_next;
	globals.GetAdd(PyValue("next")) = p_next;

	PyValue p_abs = PyValue::Function("abs");
	p_abs.GetLambdaRW().builtin = builtin_abs;
	globals.GetAdd(PyValue("abs")) = p_abs;

	PyValue p_min = PyValue::Function("min");
	p_min.GetLambdaRW().builtin = builtin_min;
	globals.GetAdd(PyValue("min")) = p_min;

	PyValue p_max = PyValue::Function("max");
	p_max.GetLambdaRW().builtin = builtin_max;
	globals.GetAdd(PyValue("max")) = p_max;

	PyValue p_sum = PyValue::Function("sum");
	p_sum.GetLambdaRW().builtin = builtin_sum;
	globals.GetAdd(PyValue("sum")) = p_sum;

	// os module
	PyValue os = PyValue::Dict();
	os.SetItem(PyValue("getcwd"), PyValue::Function("getcwd", builtin_os_getcwd));
	os.SetItem(PyValue("mkdir"), PyValue::Function("mkdir", builtin_os_mkdir));
	os.SetItem(PyValue("rmdir"), PyValue::Function("rmdir", builtin_os_rmdir));
	os.SetItem(PyValue("rename"), PyValue::Function("rename", builtin_os_rename));
	os.SetItem(PyValue("getenv"), PyValue::Function("getenv", builtin_os_getenv));
	os.SetItem(PyValue("putenv"), PyValue::Function("putenv", builtin_os_putenv));
	os.SetItem(PyValue("getpid"), PyValue::Function("getpid", builtin_os_getpid));
	os.SetItem(PyValue("listdir"), PyValue::Function("listdir", builtin_os_listdir));
	os.SetItem(PyValue("remove"), PyValue::Function("remove", builtin_os_remove));
	os.SetItem(PyValue("chdir"), PyValue::Function("chdir", builtin_os_chdir));
	
	PyValue environ = PyValue::Dict();
	const auto& env = Environment();
	for(int i = 0; i < env.GetCount(); i++)
		environ.SetItem(PyValue(env.GetKey(i)), PyValue(env[i]));
	os.SetItem(PyValue("environ"), environ);
	
	PyValue os_path = PyValue::Dict();
	os_path.SetItem(PyValue("exists"), PyValue::Function("exists", builtin_os_path_exists));
	os_path.SetItem(PyValue("isdir"), PyValue::Function("isdir", builtin_os_path_isdir));
	os_path.SetItem(PyValue("isfile"), PyValue::Function("isfile", builtin_os_path_isfile));
	os_path.SetItem(PyValue("getsize"), PyValue::Function("getsize", builtin_os_path_getsize));
	os_path.SetItem(PyValue("join"), PyValue::Function("join", builtin_os_path_join));
	os_path.SetItem(PyValue("abspath"), PyValue::Function("abspath", builtin_os_path_abspath));
	os_path.SetItem(PyValue("basename"), PyValue::Function("basename", builtin_os_path_basename));
	os_path.SetItem(PyValue("dirname"), PyValue::Function("dirname", builtin_os_path_dirname));
	os_path.SetItem(PyValue("split"), PyValue::Function("split", builtin_os_path_split));
	os_path.SetItem(PyValue("splitext"), PyValue::Function("splitext", builtin_os_path_splitext));
	os_path.SetItem(PyValue("path"), os_path); // recursive path reference is common in python
	
	os.SetItem(PyValue("path"), os_path);
	globals.GetAdd(PyValue("os")) = os;

	// sys module
	PyValue sys = PyValue::Dict();
	sys.SetItem(PyValue("exit"), PyValue::Function("exit", builtin_sys_exit));
	sys.SetItem(PyValue("executable"), PyValue::Function("executable", builtin_sys_executable));
	
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
	
	PyValue argv = PyValue::List();
	const Vector<String>& cmd = CommandLine();
	for(const String& s : cmd) argv.Add(PyValue(s));
	sys.SetItem(PyValue("argv"), argv);
	
	globals.GetAdd(PyValue("sys")) = sys;

	// math module
	PyValue math = PyValue::Dict();
	math.SetItem(PyValue("sqrt"), PyValue::Function("sqrt", builtin_math_sqrt));
	math.SetItem(PyValue("asin"), PyValue::Function("asin", builtin_math_asin));
	math.SetItem(PyValue("acos"), PyValue::Function("acos", builtin_math_acos));
	math.SetItem(PyValue("atan"), PyValue::Function("atan", builtin_math_atan));
	math.SetItem(PyValue("atan2"), PyValue::Function("atan2", builtin_math_atan2));
	math.SetItem(PyValue("sin"), PyValue::Function("sin", builtin_math_sin));
	math.SetItem(PyValue("cos"), PyValue::Function("cos", builtin_math_cos));
	math.SetItem(PyValue("tan"), PyValue::Function("tan", builtin_math_tan));
	math.SetItem(PyValue("radians"), PyValue::Function("radians", builtin_math_radians));
	math.SetItem(PyValue("degrees"), PyValue::Function("degrees", builtin_math_degrees));
	math.SetItem(PyValue("exp"), PyValue::Function("exp", builtin_math_exp));
	math.SetItem(PyValue("log"), PyValue::Function("log", builtin_math_log));
	math.SetItem(PyValue("ceil"), PyValue::Function("ceil", builtin_math_ceil));
	math.SetItem(PyValue("floor"), PyValue::Function("floor", builtin_math_floor));
	math.SetItem(PyValue("pi"), PyValue(M_PI));
	math.SetItem(PyValue("e"), PyValue(2.718281828459045));
	globals.GetAdd(PyValue("math")) = math;

	// time module
	PyValue time = PyValue::Dict();
	time.SetItem(PyValue("time"), PyValue::Function("time", builtin_time_time));
	time.SetItem(PyValue("sleep"), PyValue::Function("sleep", builtin_time_sleep));
	time.SetItem(PyValue("ctime"), PyValue::Function("ctime", builtin_time_ctime));
	globals.GetAdd(PyValue("time")) = time;

	// json module
	PyValue json = PyValue::Dict();
	json.SetItem(PyValue("dumps"), PyValue::Function("dumps", builtin_json_dumps));
	json.SetItem(PyValue("loads"), PyValue::Function("loads", builtin_json_loads));
	json.SetItem(PyValue("dump"), PyValue::Function("dump", builtin_json_dump));
	json.SetItem(PyValue("load"), PyValue::Function("load", builtin_json_load));
	globals.GetAdd(PyValue("json")) = json;
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

void PyVM::Run()
{
	while(!frames.IsEmpty()) {
		Frame& frame = TopFrame();
		if(frame.pc >= frame.ir->GetCount()) {
			frames.Drop();
			continue;
		}
		
		const PyIR& instr = (*frame.ir)[frame.pc++];
		
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
			Pop();
			break;

		case PY_LOAD_CONST:
			Push(instr.arg);
			break;
			
		case PY_LOAD_NAME: {
			PyValue v = frame.locals.Get(instr.arg, PyValue::None());
			if(v.IsNone()) v = globals.Get(instr.arg, PyValue::None());
			Push(v);
			break;
		}
		
		case PY_STORE_NAME:
			if(frames.GetCount() <= 1)
				globals.GetAdd(instr.arg) = Pop();
			else
				frame.locals.GetAdd(instr.arg) = Pop();
			break;

		case PY_LOAD_GLOBAL:
			Push(globals.Get(instr.arg, PyValue::None()));
			break;
			
		case PY_STORE_GLOBAL:
			globals.GetAdd(instr.arg) = Pop();
			break;

		case PY_LOAD_ATTR: {
			PyValue obj = Pop();
			if (obj.GetType() == PY_DICT) {
				Push(obj.GetItem(instr.arg));
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
		
		        case PY_BINARY_TRUE_DIVIDE: {
		            PyValue b = Pop();
		            PyValue a = Pop();
		            Push(PyValue(a.AsDouble() / b.AsDouble()));
		            break;
		        }
		case PY_BINARY_SUBSCR: {
			PyValue sub = Pop();
			PyValue container = Pop();
			if(sub.IsInt() && (container.GetType() == PY_LIST || container.GetType() == PY_TUPLE))
				Push(container.GetItem(sub.AsInt()));
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
				Push(PyValue::None());
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
				Push(PyValue::None());
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
				Push(PyValue::None());
			}
			break;
		}

		case PY_RETURN_VALUE: {
			PyValue ret = Pop();
			frames.Drop();
			if(!frames.IsEmpty()) Push(ret);
			break;
		}
			
		default:
			
			Cout() << "Unknown opcode: " << (int)instr.code << " at PC " << frame.pc - 1 << "\n";
			
			return;
			
		}
			

	}
}

}
