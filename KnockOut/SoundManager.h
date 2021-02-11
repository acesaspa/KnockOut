#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <AL/al.h>
#include <AL/alc.h>

class SoundManager
{
	public:
		SoundManager();
		SoundManager(ALuint* buffer, int id);
		~SoundManager();
		void initialize();
		void playSound();
		void stopSound();
		void loopSound(bool loop);
		void pauseSound();
		void setVolume(float newVol);
		void updateSourcePosition(float x, float y, float z);
		bool soundPlaying();
		void cleanup();
		int getId();

		ALuint source;
		ALint source_state;
		int id;
};
