#include "AudioEngine.h"
#include <cmath>

namespace Upp {

AudioEngine::AudioEngine() {
}

AudioEngine::~AudioEngine() {
	Shutdown();
}

bool AudioEngine::Init() {
	PaError err = Pa_Initialize();
	if (err != paNoError) return false;
	
	err = Pa_OpenDefaultStream(&stream,
	                           1,          // mono input
	                           1,          // mono output
	                           paFloat32,  // 32 bit floating point output
	                           44100,
	                           256,        // frames per buffer
	                           Callback,
	                           this);
	
	if (err != paNoError) return false;
	
	err = Pa_StartStream(stream);
	if (err != paNoError) return false;
	
	return true;
}

void AudioEngine::Shutdown() {
	if (stream) {
		Pa_StopStream(stream);
		Pa_CloseStream(stream);
		stream = nullptr;
	}
	Pa_Terminate();
}

void AudioEngine::PlayPitch(double freq, double duration_secs) {
	frequency = freq;
	playing = true;
	if (duration_secs > 0) {
		PostCallback([=] {
			Sleep((int)(duration_secs * 1000));
			StopPitch();
		});
	}
}

void AudioEngine::StopPitch() {
	playing = false;
}

void AudioEngine::GetInputData(Vector<float>& out) {
	Mutex::Lock __(input_mutex);
	out = pick(input_buffer);
	input_buffer.Clear();
}

int AudioEngine::Callback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData) {
	AudioEngine *engine = (AudioEngine*)userData;
	float *out = (float*)outputBuffer;
	const float *in = (const float*)inputBuffer;
	
	// Handle input
	if (in) {
		Mutex::Lock __(engine->input_mutex);
		for (unsigned int i = 0; i < framesPerBuffer; i++) {
			engine->input_buffer.Add(in[i]);
		}
		// Keep buffer at reasonable size (approx 1 sec)
		if (engine->input_buffer.GetCount() > 44100) {
			engine->input_buffer.Remove(0, engine->input_buffer.GetCount() - 44100);
		}
	}
	
	// Handle output (Sine wave)
	for (unsigned int i = 0; i < framesPerBuffer; i++) {
		if (engine->playing) {
			*out++ = (float)(engine->amplitude * sin(engine->phase));
			engine->phase += 2.0 * M_PI * engine->frequency / 44100.0;
			if (engine->phase > 2.0 * M_PI) engine->phase -= 2.0 * M_PI;
		} else {
			*out++ = 0;
		}
	}
	
	return paContinue;
}

}
