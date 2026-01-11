#include <string.h>
#include <jni.h>

#include <Core/Core.h>
using namespace Upp;

extern "C" {

	JNIEXPORT jstring JNICALL Java_org_upp_AndroidCore_AndroidCore_processString(
		JNIEnv *env,
		jclass,
		jstring input)
	{
		// Convert Java string to Upp string
		const char *inputStr = env->GetStringUTFChars(input, 0);
		String uppInput = String(inputStr);
		env->ReleaseStringUTFChars(input, inputStr);

		// Process with Upp::Core functionality
		String processed = uppInput + " processed by Upp::Core";
		String result = "Input: " + uppInput + " | Length: " + IntStr(uppInput.GetLength()) + " | Result: " + processed;

		// Convert back to Java string
		return env->NewStringUTF(result);
	}

	JNIEXPORT jstring JNICALL Java_org_upp_AndroidCore_AndroidCore_stringOperations(
		JNIEnv *env,
		jclass)
	{
		// Demonstrate Upp::Core string operations
		String str1 = String("Hello");
		String str2 = String("World");
		String combined = str1 + String(" ") + str2 + String("!");

		String result = "String operations: " + (String)combined +
		               " | Length: " + IntStr(combined.GetLength()) +
		               " | Upper: " + ToUpper((String)combined);

		return env->NewStringUTF(result);
	}

	JNIEXPORT jstring JNICALL Java_org_upp_AndroidCore_AndroidCore_formatOperations(
		JNIEnv *env,
		jclass)
	{
		// Demonstrate Upp::Core formatting operations
		String formatted = Format("Current time: %s", Upp::GetSysTime());
		String numberFormatted = Format("Pi approximation: %.6f", M_PI);
		String intFormatted = Format("Random number: %d", (int)Upp::Random(1000));

		String result = formatted + "\n" + numberFormatted + "\n" + intFormatted;

		return env->NewStringUTF(result);
	}

} // extern "C"