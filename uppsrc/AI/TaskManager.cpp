#include "AI.h"
#include <ide/ide.h>
#include <plugin/openai/openai.h>

NAMESPACE_UPP

struct TaskMgrConfig {
	::Ide* ide = 0;
	bool running = false, stopped = true;
	int max_tries = 3;
	openai::OpenAI* instance = 0;

	typedef TaskMgrConfig CLASSNAME;
	void Realize()
	{
		if (!running) Start();
	}
	void Start()
	{
		running = true;
		stopped = false;
		Thread::Start(THISBACK(Process));
	}
	void Stop()
	{
		running = false;
		while(!stopped)
			Sleep(100);
	}
	void Process();

	void SetupInstance(::Ide* ide);

	static TaskMgrConfig& Single()
	{
		static TaskMgrConfig m;
		return m;
	}
};

void TaskMgrConfig::Process()
{
	while(running && !Thread::IsShutdownThreads()) {
		AiTaskManager().Process();

		Sleep(10);
	}

	stopped = true;
}

void TaskMgr::Setup(::Ide* ide) { TaskMgrConfig::Single().SetupInstance(ide); }

template <class T>
String TaskMgr::MakeName(T& o, const char* name)
{
	String s;
	s << name;
	s << " #" << (int)o.fn;
	return s;
}

AiTask& TaskMgr::AddTask()
{
	task_lock.Enter();
	AiTask& t = tasks.Add();
	task_lock.Leave();
	return t;
}

void TaskMgr::Process()
{
	const TaskMgrConfig& mgr = TaskMgrConfig::Single();

	iters++;

	int ready = 0, got_ready = 0;

	this->total = tasks.GetCount();

	for(int i = 0; i < tasks.GetCount() && mgr.running && !Thread::IsShutdownThreads(); i++) {
		AiTask& t = tasks[i];

		if(!t.ready) {
			ProcessSingle(i);

			if(t.ready) {
				actual++;
				ready++;
				got_ready++;
				task_i++;
			}
		}
		else
			ready++;
	}
	this->actual = ready;

	if(!got_ready) {
		int tried_retry = 0;
		for(AiTask& t : tasks) {
			if(t.fatal_error)
				continue;
			if(t.failed && !t.ready && t.tries < mgr.max_tries) {
				t.tries++;
				t.Retry(false, false);
				tried_retry++;
			}
		}
	}
}

void TaskMgr::ProcessSingle(int task_i)
{
	task_lock.Enter();

	AiTask& t = tasks[task_i];

	Index<AiTask*> seen;

	// Skip ready, failed and those with non-ready dependencies
	if(t.ready || t.failed)
		;
	else {
		status = "Processing #" + IntStr(task_i);
		t.Process();
		status = "";
	}

	task_lock.Leave();
	
	TaskMgrConfig().Single().Realize();
}

void TaskMgr::Translate(String orig_lang, String orig_txt, String trans_lang,
                        Event<String> WhenResult, bool slightly_dialect)
{
	const TaskMgrConfig& mgr = TaskMgrConfig::Single();
	TaskMgr& p = *this;

	AiTask& t = AddTask();

	t.SetRule("translate")
		.Input(&AiTask::CreateInput_Translate)
		.Process(&AiTask::Process_Default);

	t.args << orig_lang << orig_txt << trans_lang << IntStr(slightly_dialect);
	t.WhenResult << WhenResult;
	
	TaskMgrConfig().Single().Realize();
}

void TaskMgr::RawCompletion(String prompt, Event<String> WhenResult)
{
	const TaskMgrConfig& mgr = TaskMgrConfig::Single();
	TaskMgr& p = *this;

	AiTask& t = AddTask();

	t.SetRule("raw prompt completion").Process(&AiTask::Process_Default);

	t.raw_input = prompt;
	t.WhenResult << WhenResult;
	
	TaskMgrConfig().Single().Realize();
}

void TaskMgr::CreateImage(String prompt, int count, Event<Array<Image>&> WhenResult,
                          int reduce_size_mode, Event<> WhenError)
{
	const TaskMgrConfig& mgr = TaskMgrConfig::Single();
	TaskMgr& p = *this;

	AiTask& t = AddTask();

	t.SetRule("create image")
		.ImageTask()
		.Input(&AiTask::CreateInput_CreateImage)
		.Process(&AiTask::Process_CreateImage);

	t.args << prompt << IntStr(count) << IntStr(reduce_size_mode);
	t.WhenResultImages << WhenResult;
	t.WhenError << WhenError;
	
	TaskMgrConfig().Single().Realize();
}

void TaskMgr::GetEditImage(Image orig, Image mask, String prompt, int count,
                           Event<Array<Image>&> WhenResult, Event<> WhenError)
{
	const TaskMgrConfig& mgr = TaskMgrConfig::Single();
	TaskMgr& p = *this;

	if(orig.GetSize() != mask.GetSize()) {
		WhenError();
		return;
	}

	{
		Size sz = orig.GetSize();
		ImageBuffer ib(sz);
		RGBA* dst = ib.Begin();
		const RGBA* orig_it = orig.Begin();
		const RGBA* mask_it = mask.Begin();
		const RGBA* mask_end = mask.End();
		RGBA dark;
		dark.r = 0;
		dark.g = 0;
		dark.b = 0;
		dark.a = 0;
		while(mask_it != mask_end) {
			if(mask_it->a == 0)
				*dst = *orig_it;
			else
				*dst = dark;
			mask_it++;
			orig_it++;
			dst++;
		}
		orig = ib;
	}

	AiTask& t = AddTask();

	t.SetRule("edit image")
		.ImageTask()
		.ImageEditTask()
		.Input(&AiTask::CreateInput_EditImage)
		.Process(&AiTask::Process_EditImage);

	t.send_images << orig;
	t.args << prompt << IntStr(count);
	t.WhenResultImages << WhenResult;
	t.WhenError << WhenError;
	
	TaskMgrConfig().Single().Realize();
}

void TaskMgr::VariateImage(Image orig, int count, Event<Array<Image>&> WhenResult,
                           Event<> WhenError)
{
	const TaskMgrConfig& mgr = TaskMgrConfig::Single();
	TaskMgr& p = *this;

	AiTask& t = AddTask();

	t.SetRule("variate image")
		.ImageTask()
		.ImageVariateTask()
		.Input(&AiTask::CreateInput_VariateImage)
		.Process(&AiTask::Process_VariateImage);

	t.send_images << orig;
	t.args << IntStr(count);
	t.WhenResultImages << WhenResult;
	t.WhenError << WhenError;
	
	TaskMgrConfig().Single().Realize();
}

void TaskMgr::GetGenericPrompt(const GenericPromptArgs& args, Event<String> WhenResult)
{
	const TaskMgrConfig& mgr = TaskMgrConfig::Single();
	TaskMgr& p = *this;

	String s = args.Get();

	task_lock.Enter();
	AiTask& t = tasks.Add();
	t.SetRule(MakeName(args, "generic prompt"))
		.Input(&AiTask::CreateInput_GenericPrompt)
		.Process(&AiTask::Process_Default);

	t.args << s;
	t.WhenResult << WhenResult;
	task_lock.Leave();
	
	TaskMgrConfig().Single().Realize();
}

void TaskMgr::GetVision(const String& jpeg, const VisionArgs& args, Event<String> WhenResult)
{
	const TaskMgrConfig& mgr = TaskMgrConfig::Single();
	TaskMgr& p = *this;

	String s = args.Get();

	task_lock.Enter();
	AiTask& t = tasks.Add();
	t.SetRule(MakeName(args, "vision"))
		.Input(&AiTask::CreateInput_Vision)
		.Process(&AiTask::Process_Default);

	t.args << s;
	t.jpeg = jpeg;
	t.WhenResult << WhenResult;
	task_lock.Leave();
	
	TaskMgrConfig().Single().Realize();
}

void TaskMgr::GetCode(const CodeArgs& args, Event<String> WhenResult)
{
	const TaskMgrConfig& mgr = TaskMgrConfig::Single();
	TaskMgr& p = *this;

	String s = args.Get();

	task_lock.Enter();
	AiTask& t = tasks.Add();
	t.SetRule(MakeName(args, "generic prompt"))
		.Input(&AiTask::CreateInput_Code)
		.Process(&AiTask::Process_Default);

	t.args << s;
	t.WhenResult << WhenResult;
	task_lock.Leave();
	
	TaskMgrConfig().Single().Realize();
}

void TaskMgr::GetTokenData(const TokenArgs& args, Event<String> WhenResult) {
	const TaskMgrConfig& mgr = TaskMgrConfig::Single();
	TaskMgr& p = *this;
	
	String s = args.Get();
	
	task_lock.Enter();
	AiTask& t = tasks.Add();
	
	t.SetRule(MakeName(args, "get token data"))
		.Input(&AiTask::CreateInput_GetTokenData)
		.Process(&AiTask::Process_Default);
	
	t.args << s;
	t.WhenResult << WhenResult;
	task_lock.Leave();
}

TaskRule& TaskRule::SetRule(const String& name)
{
	this->name = name;
	return *this;
}

TaskRule& TaskRule::Input(void (AiTask::*fn)())
{
	this->input = fn;
	return *this;
}

TaskRule& TaskRule::Process(void (AiTask::*fn)())
{
	process = fn;
	return *this;
}

TaskRule& TaskRule::Spawnable(bool b)
{
	spawnable = b;
	return *this;
}

TaskRule& TaskRule::MultiSpawnable(bool b)
{
	spawnable = b;
	multi_spawnable = b;
	return *this;
}

TaskRule& TaskRule::CrossMode(bool b)
{
	allow_cross_mode = b;
	return *this;
}

TaskRule& TaskRule::SeparateItems(bool b)
{
	separate_items = b;
	return *this;
}

TaskRule& TaskRule::DebugInput(bool b)
{
	debug_input = b;
	return *this;
}

TaskRule& TaskRule::ImageTask(bool b)
{
	image_task = b;
	return *this;
}

TaskRule& TaskRule::ImageEditTask(bool b)
{
	imageedit_task = b;
	return *this;
}

TaskRule& TaskRule::ImageVariateTask(bool b)
{
	imagevariate_task = b;
	return *this;
}

void TaskMgrConfig::SetupInstance(::Ide* ide)
{
	this->ide = ide;

	if(!instance && !ide->openai_token.IsEmpty())
		instance = &openai::start(ide->openai_token.Begin());

	if(instance) {
		if(!ide->openai_proxy.IsEmpty())
			instance->setProxy(ide->openai_proxy.Begin());
		else
			instance->setProxy("");
	}
}

TaskMgr& AiTaskManager()
{
	static TaskMgr tm;
	return tm;
}

END_UPP_NAMESPACE

