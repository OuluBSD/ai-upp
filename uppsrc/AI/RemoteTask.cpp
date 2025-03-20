#include "AI.h"
#include <plugin/openai/openai.h>
#include <ide/ide.h>

NAMESPACE_UPP

void AiTask::Store(bool force)
{
	if(output.IsEmpty())
		return;
	if(!changed)
		return;
	String dir = ConfigFile("ai-results");
	RealizeDirectory(dir);
	String filename = GetInputHash() + ".txt";
	String file = dir + filename;
	SaveFileBOMUtf8(file, output);
	changed = false;
}

void AiTask::Load()
{
	if(skip_load)
		return;
	String dir = ConfigFile("ai-results");
	RealizeDirectory(dir);
	String filename = GetInputHash() + ".txt";
	String file = dir + filename;
	if(FileExists(file))
		output = LoadFileBOM(file);
	if(IsAnyImageTask()) {
		String dir = ConfigFile("images");
		Vector<String> rel_paths = Split(output, "\n");
		recv_images.Clear();
		bool fail = false;
		for(String& rel_path : rel_paths) {
			String path = AppendFileName(dir, rel_path);
			Image& img = recv_images.Add();
			img = StreamRaster::LoadFileAny(path);
			if(img.IsEmpty())
				fail = true;
		}
		if(fail) {
			recv_images.Clear();
			output.Clear();
		}
	}
}

void AiTask::SetError(String s)
{
	failed = true;
	error = s;
	WhenError();
}

String AiTask::GetInputHash() const
{
	String input = MakeInputString();
	hash_t h = input.GetHashValue();
	if(binary_param.GetCount()) {
		CombineHash c;
		c.Put(h);
		c.Do(binary_param);
		h = c;
	}
	return HexString((void*)&h, sizeof(h));
}

String AiTask::GetOutputHash() const
{
	hash_t h = output.GetHashValue();
	return HexString((void*)&h, sizeof(h));
}

bool AiTask::HasAnyInput() const {
	return !raw_input.IsEmpty() || !input.IsEmpty() || !json_input.IsEmpty();
}

bool AiTask::HasJsonInput() const {
	return raw_input.IsEmpty() && !json_input.IsEmpty();
}

bool AiTask::ForceCompletion() const {
	return json_input.force_completion;
}

String AiTask::MakeInputString(bool pretty) const {
	if (raw_input.GetCount())
		return raw_input;
	else if (!json_input.IsEmpty())
		return json_input.AsJSON(pretty);
	else
		return input.AsString();
}

String AiTask::GetDescription() const
{
	String s;
	s << GetTypeString();
	return s;
}

String AiTask::GetTypeString() const { return TaskRule::name; }

bool AiTask::ProcessInput()
{
	bool ok = true;

	if(raw_input.GetCount()) {
		Load();
	}
	else {
		// Return if this task won't have input function
		if(!TaskRule::input)
			return ok;

		// Create input with given function
		if(TaskRule::input) {
			json_input.Clear();
			input.Clear();
			(this->*TaskRule::input)();
			if(fast_exit)
				return true;

			if(failed)
				return false;

			Load();
		}
	}

	// Remove Win32 uselessness (\r in newline)
	output = TrimBoth(output);

	// Request output from completion-mode AI
	if(HasAnyInput() && output.IsEmpty()) {
		ok = RunOpenAI();
	}

	// Remove Win32 uselessness (\r in newline)
	output.Replace("\r", "");

	if(output.IsEmpty())
		return false;

	return ok;
}

void AiTask::Process()
{

	if(ret_fail) {
		output = "fail";
		try {
			(this->*TaskRule::process)();
		}
		catch(Exc e) {
			LOG("error: " << e);
		}
		ready = true;
		fast_exit = false;
		skip_load = false;
		processing = false;
		WhenDone();
		return;
	}

	// LOG("AiTask::Process: begin of " << TaskRule::name);
	processing = true;

	bool ok = true;

	ok = ok && ProcessInput();

	if(fast_exit) {
		ready = true;
	}
	else if(ok) {
		if(TaskRule::process) {
			try {
				(this->*TaskRule::process)();
			}
			catch(Exc e) {
				LOG("error: " << e);
			}
		}

		if(wait_task) {
			wait_task = false;
		}
		else {
			if(!failed) {
				ready = true;
			}
			changed = true;
		}
	}

	fast_exit = false;
	skip_load = false;
	processing = false;

	if(ready)
		WhenDone();

	// LOG("AiTask::Process: end of " << rule->name);
}

bool AiTask::RunOpenAI()
{
	switch (type) {
		case TYPE_COMPLETION:		return RunOpenAI_Completion();
		case TYPE_IMAGE_GENERATION:	return RunOpenAI_Image();
		case TYPE_IMAGE_EDIT:		return RunOpenAI_Image();
		case TYPE_IMAGE_VARIATE:	return RunOpenAI_Image();
		case TYPE_VISION:			return RunOpenAI_Vision();
		case TYPE_TRANSCRIPTION:	return RunOpenAI_Transcription();
		default: TODO; return false;
	}
}

String AiTask::FixInvalidChars(const String& s)
{
	WString ws = s.ToWString();
	WString out;
	for(int i = 0; i < ws.GetCount(); i++) {
		int chr = ws[i];

		// ascii
		if(chr < 32 && chr != '\n') {
			// pass
		}
		else {
			out.Cat(chr);
		}
	}
	return out.ToString();
}

void AiTask::EscapeString(String& s)
{
	s = ToCharset(CHARSET_ISO8859_15, s, CHARSET_UTF8);
	s = ToCharset(CHARSET_UTF8, s, CHARSET_ISO8859_15);
	s = StoreAsJson(s);
	RemoveQuotes(s);
}

void AiTask::RemoveQuotes(String& s)
{
	if(s.GetCount() > 0 && s[0] == '\"')
		s = s.Mid(1);
	int c = s.GetCount();
	if(c > 0 && s[c - 1] == '\"')
		s = s.Left(c - 1);
}

void AiTask::RemoveQuotes2(String& s_)
{
	WString ws = s_.ToWString();
	if(ws.GetCount() > 0 && (ws[0] == '\"' || ws[0] == L"“"[0]))
		ws = ws.Mid(1);
	int c = ws.GetCount();
	if(c > 0 && (ws[c - 1] == '\"' || ws[c - 1] == L"”"[0]))
		ws = ws.Left(c - 1);
	s_ = ws.ToString();
}

void AiTask::RemoveParenthesis(String& s)
{
	if(s.GetCount() > 0 && s[0] == '(')
		s = s.Mid(1);
	int c = s.GetCount();
	if(c > 0 && s[c - 1] == ')')
		s = s.Left(c - 1);
}

bool AiTask::RunOpenAI_Image()
{
	output.Clear();

	String prompt = MakeInputString();

	prompt.Replace("\\", "\\\\");
	prompt.Replace("\n", " ");
	prompt.Replace("\t", " ");
	prompt.Replace("\"", "\\\"");
	prompt = TrimBoth(prompt);

	prompt = FixInvalidChars(prompt); // NOTE: warning: might break something

	// Don't even try offensive language
	prompt.Replace(" fuck ", " f**k ");
	prompt.Replace(" bitch ", " b***h ");
	prompt.Replace(" whore ", " w***e ");
	prompt.Replace(" nigga ", " n***a ");

	if(image_n.IsEmpty() || image_sz.IsEmpty()) {
		SetError("No image arguments set");
		return false;
	}

	String recv;
	try {
		if(TaskRule::type == TYPE_IMAGE_EDIT) {
			if(send_images.GetCount() != 1) {
				SetError("expected sendable images");
				return false;
			}
			String file_path0 = ConfigFile("tmp0.png");
			PNGEncoder().SaveFile(file_path0, send_images[0]);

			openai::Json json({{"image", file_path0.Begin()},
			                   {"prompt", prompt.Begin()},
			                   {"n", StrInt(image_n)},
			                   {"size", image_sz},
			                   {"response_format", "b64_json"}});
			auto img = openai::image().edit(json);
			recv = String(img.dump(2));
		}
		else if(TaskRule::type == TYPE_IMAGE_VARIATE) {
			if(send_images.GetCount() != 1) {
				SetError("expected sendable images");
				return false;
			}
			String file_path0 = ConfigFile("tmp0.png");
			PNGEncoder().SaveFile(file_path0, send_images[0]);

			openai::Json json({{"image", file_path0.Begin()},
			                   {"n", StrInt(image_n)},
			                   {"size", image_sz},
			                   {"response_format", "b64_json"}});
			auto img = openai::image().variation(json);
			recv = String(img.dump(2));
		}
		else {
			openai::Json json({{"prompt", prompt.Begin()},
			                   {"n", StrInt(image_n)},
			                   {"size", image_sz},
			                   {"response_format", "b64_json"}});
			auto img = openai::image().create(json);
			recv = String(img.dump(2));
		}
	}
	catch(std::runtime_error e) {
		if(keep_going) {
			output = " ";
			GetTaskMgr().keep_going_counter++;
			return true;
		}
		LOG(prompt);
		fatal_error = true;
		SetError(e.what());
		Array<Image> res;
		WhenResultImages(res);
		if(auto_ret_fail)
			ReturnFail();
		return false;
	}
	catch(std::string e) {
		if(keep_going) {
			output = " ";
			GetTaskMgr().keep_going_counter++;
			return true;
		}
		LOG(prompt);
		fatal_error = true;
		SetError(e.c_str());
		Array<Image> res;
		WhenResultImages(res);
		if(auto_ret_fail)
			ReturnFail();
		return false;
	}
	catch(NLOHMANN_JSON_NAMESPACE::detail::parse_error e) {
		if(keep_going) {
			output = " ";
			GetTaskMgr().keep_going_counter++;
			return true;
		}
		LOG(prompt);
		LOG(e.what());
		fatal_error = true;
		SetError(e.what());
		Array<Image> res;
		WhenResultImages(res);
		if(auto_ret_fail)
			ReturnFail();
		return false;
	}
	catch(std::exception e) {
		if(keep_going) {
			output = " ";
			GetTaskMgr().keep_going_counter++;
			return true;
		}
		LOG(prompt);
		SetError(e.what());
		fatal_error = true;
		Array<Image> res;
		WhenResultImages(res);
		if(auto_ret_fail)
			ReturnFail();
		return false;
	}
	DalleResponse response;
	LoadFromJson(response, recv);

	output.Clear();
	recv_images.Clear();

	for(int i = 0; i < response.data.GetCount(); i++) {
		String img_str = Base64Decode(response.data[i].b64_json);

		Image& in = recv_images.Add();
		in = StreamRaster::LoadStringAny(img_str);

		// Get file path
		String part_str = " " + IntStr(i + 1) + "/" + IntStr(response.data.GetCount());
		if(TaskRule::type == TYPE_IMAGE_EDIT || TaskRule::type == TYPE_IMAGE_VARIATE)
			part_str << " "
					 << IntStr64(Random64()); // add never-matching random number to name for
			                                  // editing and variation creation purposes
		String dir = ConfigFile("images");
		String filename = Base64Encode(prompt + part_str) + ".png";
		String rel_path = AppendFileName(image_sz, filename);
		String path = AppendFileName(dir, rel_path);
		RealizeDirectory(GetFileDirectory(path));

		// Store to file
		PNGEncoder enc;
		enc.SaveFile(path, in);

		// Add file path to output
		output << rel_path << "\n";
	}

	changed = true;
	Store();
	return output.GetCount() > 0;
}

bool AiTask::RunOpenAI_Completion()
{
	output.Clear();

	if(!input.response_length) {
		LOG("warning: no response length set");
		input.response_length = 1024;
	}
	String prompt = MakeInputString(true);

	prompt = FixInvalidChars(prompt); // NOTE: warning: might break something

	// Cache prompts too (for crash debugging)
	if(1) {
		String prompt_cache_dir = ConfigFile("prompt-cache");
		String fname = IntStr64(prompt.GetHashValue()) + ".txt";
		// DUMP(fname);
		String path = prompt_cache_dir + DIR_SEPS + fname;
		if(!FileExists(path)) {
			RealizeDirectory(prompt_cache_dir);
			FileOut fout(path);
			fout << prompt;
			fout.Flush();
			fout.Close();
		}
	}

#ifdef flagLLAMACPP
	if(1) {
		LlamaCppResponse response;
		response.SetPrompt(prompt);
		response.SetMaxTokens(input.response_length);
		response.Process();
		output = response.GetOutput();
	}
	else
#endif
	{
		EscapeString(prompt);

		if(GetDefaultCharset() != CHARSET_UTF8)
			prompt = ToCharset(CHARSET_UTF8, prompt, CHARSET_DEFAULT);

		String txt;
		bool is_chat_json_input = HasJsonInput() && !ForceCompletion();
		bool chat_completion = is_chat_json_input || quality == 1;
		if (is_chat_json_input) {
			txt = R"_({
			    "model": ")_" + String(quality == 1 ? "gpt-4-turbo" : "gpt-3.5-turbo") + R"_(",
			    "messages":[)_";
			for(int i = 0; i < json_input.messages.GetCount(); i++) {
				const auto& msg = json_input.messages[i];
				String part_content = msg.GetContentString();
				EscapeString(part_content);
				txt << "{\"role\": \""
					<< msg.GetTypeString() << "\", \"content\": [{\"type\": \"text\", \"text\": \""
					<< part_content
					<< "\"}]}";
				if (i+1 < json_input.messages.GetCount())
					txt << ",\n";
			}
			txt << R"_(],
			    "max_tokens": )_" + IntStr(input.response_length) + R"_(,
			    "temperature": 1
			})_";
			//LOG(txt);
		}
		else {
			if(quality == 1) {
				txt = R"_({
				    "model": "gpt-4-turbo",
				    "messages":[
						{"role": "system", "content": "This is an impersonal text completion system like gpt-3.5-turbo. This is not chat mode. Do not repeat the last part of the input text in your result."},
						{"role":"user", "content": ")_" +
							      prompt + R"_("}
					],
				    "max_tokens": )_" +
							      IntStr(input.response_length) + R"_(,
				    "temperature": 1
				})_";
			}
			else {
				txt = R"_({
				    "model": "gpt-3.5-turbo-instruct",
				    "stop": "<|endoftext|>",
				    "prompt": ")_" +
							      prompt + R"_(",
				    "max_tokens": )_" +
							      IntStr(input.response_length) + R"_(,
				    "temperature": 1
				})_";
			}
		}
		// LOG(txt);
		return TryOpenAI(prompt, txt, [this,txt,chat_completion]{
			nlohmann::json json = nlohmann::json::parse(txt.Begin(), txt.End());
			OpenAiResponse response;

			if(chat_completion) {
				auto completion = openai::completion().create_chat(json);
				LoadFromJson(response, String(completion.dump(2)));
			}
			else {
				auto completion = openai::completion().create(json);
				// LOG("Response is:\n" << completion.dump(2));
				LoadFromJson(response, String(completion.dump(2)));
				// LOG(response.ToString());
			}

			if(response.choices.GetCount())
				output = response.choices[0].GetText();
			else {
				SetError("invalid output");
				output.Clear();
			}
		});
	}
}

bool AiTask::RunOpenAI_Vision()
{
	output.Clear();

	if(!input.response_length) {
		LOG("warning: no response length set");
		input.response_length = 1024;
	}
	String prompt = MakeInputString();

	String base64 = Base64Encode(this->binary_param);

	{
		EscapeString(prompt);

		if(GetDefaultCharset() != CHARSET_UTF8)
			prompt = ToCharset(CHARSET_UTF8, prompt, CHARSET_DEFAULT);

		String txt;
		if(quality == 1) {
			txt = R"_({
    "model": "gpt-4o",
    "messages":[
		{"role":"user", "content": [
			{
				"type":"text",
				"text":")_" +
			      prompt + R"_("
			},
			{
				"type":"image_url",
				"image_url": {
					"url": "data:image/jpeg;base64,)_" +
			      base64 + R"_("
				}
			}
		]}
	],
    "max_tokens": )_" +
			      IntStr(input.response_length) + R"_(,
    "temperature": 1
})_";
		}
		else {
			txt = R"_({
    "model": "gpt-4-turbo",
    "messages":[
		{"role":"user", "content": [
			{
				"type":"text",
				"text":")_" +
			      prompt + R"_("
			},
			{
				"type":"image_url",
				"image_url": {
					"url": "data:image/jpeg;base64,)_" +
			      base64 + R"_("
				}
			}
		]}
	],
    "max_tokens": )_" +
			      IntStr(input.response_length) + R"_(,
    "temperature": 1
})_";
		}
		LOG(txt);

		return TryOpenAI(prompt, txt, [this,txt]{
			nlohmann::json json = nlohmann::json::parse(txt.Begin(), txt.End());
			OpenAiResponse response;

			auto completion = openai::completion().create_chat(json);
			LoadFromJson(response, String(completion.dump(2)));

			if(response.choices.GetCount())
				output = response.choices[0].GetText();
			else {
				SetError("invalid output");
				output.Clear();
			}
		});
	}
}

bool AiTask::RunOpenAI_Transcription()
{
	TranscriptionArgs args;
	args.Put(this->args[0]);
	
	output.Clear();

	if(!input.response_length) {
		LOG("warning: no response length set");
		input.response_length = 1024;
	}
	String prompt = MakeInputString();
	
	{
		EscapeString(prompt);

		if(GetDefaultCharset() != CHARSET_UTF8)
			prompt = ToCharset(CHARSET_UTF8, prompt, CHARSET_DEFAULT);

		auto& ai_mgr = TheIde()->ai_manager;
		ASSERT(args.ai_provider_idx >= 0 && args.ai_provider_idx < ai_mgr.GetCount());
		const auto& prov = ai_mgr[args.ai_provider_idx];
		
		if (prov.type == AiServiceProvider::OPENAI) {
			String txt =
			R"_({
			    "model": "whisper-1",
			    "response_format": "verbose_json",
			    "language": ")_" + args.language + R"_("
			})_";
			return TryOpenAI(prompt, txt, [this,txt,&args]{
				nlohmann::json json = nlohmann::json::parse(txt.Begin(), txt.End());
				
				json.push_back({"file", args.file});
				
				//OpenAiResponse response;
	
				auto transcription = openai::audio().transcribe(json);
				String str = String(transcription.dump(2));
				//LOG(str);
				//LoadFromJson(response, str);
	
				if (!str.IsEmpty())
					output = str;
				else {
					SetError("invalid output");
					output.Clear();
				}
			});
		}
		else if (prov.type == AiServiceProvider::API_WHISPERFILE_TRANSCRIPT) {
			#ifdef flagWIN32
			TODO
			#else
			String output = ConfigFile("curl-output.txt");
			// TODO use api
			String cmd =
			"curl " + prov.url + "/inference"
			   " -H \"Content-Type: multipart/form-data\""
			   " -F file=\"@" + args.file + "\""
			   " -F language=\"" + args.language + "\""
			   " -F temperature=\"0.0\""
			   " -F temperature_inc=\"0.2\""
			   " -F response_format=\"verbose_json\""
			   " -o \"" + output + "\"";
			LOG(cmd);
			String out;
			if (!Sys(cmd, out)) {
				this->output = LoadFile(output);
				return true;
			}
			else {
				SetError("executing curl failed");
				output.Clear();
				return false;
			}
			#endif
		}
	}
	return false;
}

bool AiTask::TryOpenAI(String prompt, String txt, Event<> cb) {
	{
		try {
			cb();
		}
		catch(std::runtime_error e) {
			if(keep_going) {
				output = " ";
				return true;
			}
			LOG(prompt);
			LOG(txt);
			fatal_error = true;
			SetError(e.what());
			if(auto_ret_fail)
				ReturnFail();
			return false;
		}
		catch(std::string e) {
			if(keep_going) {
				output = " ";
				return true;
			}
			LOG(prompt);
			LOG(txt);
			fatal_error = true;
			SetError(e.c_str());
			if(auto_ret_fail)
				ReturnFail();
			return false;
		}
		catch(NLOHMANN_JSON_NAMESPACE::detail::parse_error e) {
			if(keep_going) {
				output = " ";
				return true;
			}
			LOG(prompt);
			LOG(txt);
			LOG(e.what());
			fatal_error = true;
			SetError(e.what());
			if(auto_ret_fail)
				ReturnFail();
			return false;
		}
		catch(std::exception e) {
			if(keep_going) {
				output = " ";
				return true;
			}
			LOG(prompt);
			LOG(txt);
			SetError(e.what());
			fatal_error = true;
			if(auto_ret_fail)
				ReturnFail();
			return false;
		}
		/*catch (...) {
		    SetError("unknown error");
		    return false;
		}*/

		// LOG(IntStr64(input.AsString().GetHashValue()));

		// Fix unicode formatting
		output = ToUnicode(output, CHARSET_UTF8).ToString();
	}

	changed = true;
	Store();
	return output.GetCount() > 0;
}

void AiTask::Retry(bool skip_prompt, bool skip_cache)
{
	if(!skip_prompt) {
		json_input.Clear();
		input.Clear();
		output.Clear();
	}
	skip_load = skip_cache;
	failed = false;
	fatal_error = false;
	ready = false;
	error.Clear();
	changed = true;
	tries = 0;
}

void AiTask::ReturnFail()
{
	output = "fail";
	tmp_str = "";

	ret_fail = true;
	failed = false;
	fatal_error = false;
	ready = false;
	error.Clear();
	changed = true;
	tries = 1;
}

TaskMgr& AiTask::GetTaskMgr() { return AiTaskManager(); }

void AiTask::Process_CreateImage() { WhenResultImages(recv_images); }

void AiTask::Process_EditImage() { WhenResultImages(recv_images); }

void AiTask::Process_VariateImage() { WhenResultImages(recv_images); }

void AiTask::Process_Default() { WhenResult(tmp_str + output); }

END_UPP_NAMESPACE

