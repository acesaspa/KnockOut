//Soruces: https://gist.github.com/mudream4869/34541dfbd12a747b027e, Stephen Dios

#include "SoundManager.h"

SoundManager::SoundManager() {};
SoundManager::SoundManager(ALuint* buffer, int id)
{
	this->id = id;
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, *buffer);
}

SoundManager::~SoundManager()
{
	this->cleanup();
}

void SoundManager::initialize() 
{
	alSourcef(source, AL_PITCH, 1.0f);
	alSourcef(source, AL_GAIN, 1);
	alSource3f(source, AL_POSITION, 0, 0, 0);
	alSource3f(source, AL_VELOCITY, 0, 0, 0);
	alSourcei(source, AL_LOOPING, AL_FALSE);
}

int SoundManager::getId() 
{
	return this->id;
}

void SoundManager::playSound() 
{
	alSourcePlay(source);
}

void SoundManager::pauseSound()
{
	alSourcePause(source);
}

void SoundManager::stopSound() 
{
	alSourceStop(source);
}

void SoundManager::loopSound(bool loop) 
{
	alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
}

void SoundManager::setVolume(float volume)
{
	alSourcef(source, AL_GAIN, volume);
}

void SoundManager::updateSourcePosition(float x, float y, float z) 
{
	alSource3f(source, AL_POSITION, x, y, z);
}

bool SoundManager::soundPlaying() 
{
	alGetSourcei(source, AL_SOURCE_STATE, &source_state);
	return source_state == AL_PLAYING;
}

void SoundManager::cleanup() 
{
	this->stopSound();
	alDeleteSources(1, &source);
}
