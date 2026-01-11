#include <jni.h>
#include <map>
#include <vector>
#include <sstream>
#include <string>

// Include Core headers in the correct order for Android
#include <Core/Core.h>

#include "MemoryManager.h"

using namespace Upp;

// Example class that uses Upp::Core functionality
class CoreStringProcessor {
public:
    CoreStringProcessor(const String& input) : data(input) {}

    String Process() {
        return data + " processed at " + AsString(Time());
    }

    String GetReversed() {
        String reversed;
        for(int i = data.GetLength() - 1; i >= 0; i--) {
            reversed.Cat(data[i]);
        }
        return reversed;
    }

    int GetLength() {
        return data.GetLength();
    }

private:
    String data;
};

static MemoryManager<CoreStringProcessor> mm;

extern "C" {

	JNIEXPORT void JNICALL Java_org_upp_AndroidCore_CoreStringProcessor_construct(
		JNIEnv *env,
		jobject obj,
		jstring input)
	{
		const char *inputStr = env->GetStringUTFChars(input, 0);
		String uppInput = String(inputStr);
		env->ReleaseStringUTFChars(input, inputStr);

		mm.Insert(env, obj, new CoreStringProcessor(uppInput));
	}

	JNIEXPORT void JNICALL Java_org_upp_AndroidCore_CoreStringProcessor_nativeFinalize(
		JNIEnv *env,
		jobject obj)
	{
		mm.Erase(env, obj);
	}

	JNIEXPORT jstring JNICALL Java_org_upp_AndroidCore_CoreStringProcessor_process(
		JNIEnv *env,
		jobject obj)
	{
		CoreStringProcessor* processor = mm.Get(env, obj);
		if (!processor) return env->NewStringUTF("Error: processor is null");

		String result = processor->Process();
		return env->NewStringUTF(result);
	}

	JNIEXPORT jstring JNICALL Java_org_upp_AndroidCore_CoreStringProcessor_getReversed(
		JNIEnv *env,
		jobject obj)
	{
		CoreStringProcessor* processor = mm.Get(env, obj);
		if (!processor) return env->NewStringUTF("Error: processor is null");

		String result = processor->GetReversed();
		return env->NewStringUTF(result);
	}

	JNIEXPORT jint JNICALL Java_org_upp_AndroidCore_CoreStringProcessor_getLength(
		JNIEnv *env,
		jobject obj)
	{
		CoreStringProcessor* processor = mm.Get(env, obj);
		if (!processor) return -1;

		return processor->GetLength();
	}

} // extern "C"