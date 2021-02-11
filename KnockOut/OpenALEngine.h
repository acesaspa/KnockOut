#define _CRT_SECURE_NO_WARNINGS
#include "SoundManager.h"
#include <iostream>
#include <vector>
#include <glm\ext\vector_float3.hpp>

class AudioEngine
{
	public:
		AudioEngine();
		~AudioEngine();
		void initialize();
		void updateListenerPosition(float x, float y, float z);
		void updateListenerOrientatation(glm::vec3 front, glm::vec3 up);
		void initializeBuffers();
		SoundManager& createBoomBox(int soundFile);
		void killSource(SoundManager* boombox);
		void killSources();
		void pauseAllActiveSources();
		void resumeAllActiveSources();
		std::vector<int> sourcesPaused;
		bool allHaveBeenPaused;

	private:
		void CheckError();
		bool _strcmp(const char* base, const char* cp);
		bool loadWavFile(const char* filename, ALuint* buffer);

		static const int ARR_SIZE = 1;
		const char* soundFiles[ARR_SIZE];

		int sourcesMade;

		ALuint bufferArray[ARR_SIZE];
		std::vector<std::unique_ptr<SoundManager>> listOfSources;

		ALCdevice *device;
		ALCcontext *context;
};
