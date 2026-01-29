#ifndef _SingerTrainer_AudioEngine_h_
#define _SingerTrainer_AudioEngine_h_

#include <Core/Core.h>
#include <portaudio.h>

namespace Upp {

class AudioEngine {
	PaStream *stream = nullptr;
	double phase = 0;
	double frequency = 0;
	double amplitude = 0.5;
	bool playing = false;
	
	Vector<float> input_buffer;
	Mutex input_mutex;
	
public:
	AudioEngine();
	~AudioEngine();
	
	bool Init();
	void Shutdown();
	
	void PlayPitch(double freq, double duration_secs);
	void StopPitch();
	
	// Thread-safe access to input data
	void GetInputData(Vector<float>& out);
	
	static int Callback(const void *inputBuffer, void *outputBuffer,
	                    unsigned long framesPerBuffer,
	                    const PaStreamCallbackTimeInfo* timeInfo,
	                    PaStreamCallbackFlags statusFlags,
	                    void *userData);
};

}

#endif
