/*
 * sample.cpp
 * 
 * Created by Marius Utheim on 27.05.2011
 * All rights reserved
 *
 */

#include <fstream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>
#include <ogg/ogg.h>
#include "sample.h"
#include "wavefile.h"
#include "lot.h"
using std::ifstream;

#define WORD_SIZE 2

sample::sample()
{
	alGenBuffers(1, &buffer);
	sourcecount = 0;
	sources = 0;
}

sample::~sample()
{
	free();
}

int sample::free()
{
	alDeleteSources(sourcecount, sources);
	alDeleteBuffers(1, &buffer);
	return 1;
}

int sample::load_ogg(char *path)
{
	OggVorbis_File oggFile;
	int size;
	vorbis_info *info;
	char *pcm;
	
	FILE *fileHandle;
	if (fopen_s(&fileHandle, path, "rb") != 0)
	{
		return -1;
	}
	
	if (ov_open(fileHandle, &oggFile, NULL, 0) < 0)
	{
		fclose(fileHandle);
		return -2;
	}
	
	info = ov_info(&oggFile, -1);
	ogg_int64_t actualSize = ov_pcm_total(&oggFile, -1);
	size = (int)actualSize * WORD_SIZE * info->channels;
	ALenum format = info->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	
	//	result = ov_read(&oggStream, pcm + size, BUFFER_SIZE - size, 0, 2, 1, &section);
	pcm = new char[size];
	if (pcm == 0)
	{
		// Handle memory leaks
		return -3;
	}

	
	int section;
	int offset = 0;
	int result;
	
	///TODO/// Can everything be just read outside of the loop?
	while (offset < size)
	{
		result = ov_read(&oggFile, pcm + offset, size - offset, 0, WORD_SIZE, 1, &section);
		
		if (result > 0)
			offset += result;
		else if (result < 0)
			return -5;
		else
			break;
	}
	
	alBufferData(buffer, format, pcm, size, info->rate);
	if (!check())
		return -4;
	ov_clear(&oggFile);
	delete pcm;
	
	return 1;
}

int sample::load_wav(char *path)
{
	wavefile *f = wavefile_open(path);
	if (f == 0)
		return -3;
	ALenum format;

	///TODO/// Handle unsupported formats?
	if (f->NumChannels == 1)
	{
		if (f->BitsPerSample == 8)
			format = AL_FORMAT_MONO8;
		else
			format = AL_FORMAT_MONO16;
	}
	else
	{
		if (f->BitsPerSample == 8)
			format = AL_FORMAT_STEREO8;
		else
			format = AL_FORMAT_STEREO16;
	}
	

	alBufferData(buffer, format, f->data, f->Subchunk2Size, f->SampleRate);
	return check();
}

int sample::play(bool looping, float volume, float pitch)
{
	unsigned int i;
	for (i = 0; i < sourcecount; i++)
	{
		if (!is_playing(i))
			goto startplaying;
	}
	
	if (sources == 0)
	{
		sources = (ALuint *)malloc(sizeof(ALuint));
		alGenSources(1, sources);
		sourcecount++;
		i = 0;
	}
	else
	{
		sources = (ALuint *)realloc(sources, 2 * sourcecount * sizeof(ALuint));
		alGenSources(sourcecount, &sources[sourcecount]);
		ALenum err = alGetError();
		if (err == AL_INVALID_VALUE)
			return -2;
		i = sourcecount;
		sourcecount *= 2;
#ifdef _DEBUG
		if (sourcecount > 256)
			throw "error";
#endif
	}
	
startplaying:
	alSourcei(sources[i], AL_BUFFER, buffer);
	alSourcef(sources[i], AL_GAIN, volume);
	alSourcef(sources[i], AL_PITCH, pitch);
	alSourcei(sources[i], AL_LOOPING, looping);
	alSource3f(sources[i], AL_POSITION, 0, 1, 0);

	alSourcePlay(sources[i]);

	if (check() < 0)
		return -1;
	else
		return i;
}

int sample::stop(unsigned int instance)
{
	if (is_playing(instance))
	{
		alSourceStop(sources[instance]);
		return check();
	}
	else
		return -2;
}

int sample::stop_all()
{
	for (unsigned int i = 0; i < sourcecount; i++)
		if (is_playing(i))
			alSourceStop(sources[i]);

	return check();
}

int sample::pause(unsigned int instance)
{
	if (instance < sourcecount)
	{
		alSourcePause(sources[instance]);
		return check();
	}
	else
		return -2;
}

int sample::pause_all()
{
	for (unsigned int i = 0; i < sourcecount; i++)
		alSourcePause(sources[i]);

	return check();
}

int sample::resume(unsigned int instance)
{
	if (instance < sourcecount)
	{
		ALint state;
		alGetSourcei(sources[instance], AL_SOURCE_STATE, &state);
		if (state == AL_PAUSED)
			alSourcePlay(sources[instance]);
		return check();
	}
	else
		return -2;
}

int sample::resume_all()
{
	ALint state;
	for (unsigned int i = 0; i < sourcecount; i++)
	{
		alGetSourcei(sources[i], AL_SOURCE_STATE, &state);
		if (state == AL_PAUSED)
			alSourcePlay(sources[i]);
	}

	return check();
}

int sample::check()
{
	return (alGetError() == AL_NO_ERROR) ? 1 : -1;
}

int sample::is_playing()
{
	for (unsigned int i = 0; i < sourcecount; i++)
		if (is_playing(i))
			return 1;

	return 0;
}

int sample::is_playing(unsigned int instance)
{
	if (instance < sourcecount)
	{
		ALint state;
		alGetSourcei(sources[instance], AL_SOURCE_STATE, &state);
		return (state == AL_PLAYING || state == AL_PAUSED) ? 1 : 0;
	}
	else
		return 0;
}

void sample::set_looping(unsigned int instance, bool looping)
{
	if (instance == 0)
		for (unsigned int i = 0; i < sourcecount; i++)
			alSourcei(sources[i], AL_LOOPING, looping);
	else if (instance < sourcecount)
		alSourcei(sources[instance], AL_LOOPING, looping);
}

void sample::set_pitch(float pitch)
{
	for (unsigned int i = 0; i < sourcecount; i++)
		set_pitch(i, pitch);
}

void sample::set_pitch(unsigned int instance, float pitch)
{
	if (is_playing(instance))
		alSourcef(sources[instance], AL_PITCH, pitch);
}

void sample::set_volume(float volume)
{
	for (unsigned int i = 0; i < sourcecount; i++)
		set_volume(i, volume);
}

void sample::set_volume(unsigned int instance, float volume)
{
	if (is_playing(instance))
		alSourcef(sources[instance], AL_GAIN, volume);
}

bool sample::get_looping(unsigned int instance)
{
	if (!is_playing(instance))
		return false;
	int looping;
	alGetSourcei(sources[instance], AL_LOOPING, &looping);
	return (looping == 1);
}

float sample::get_volume(unsigned int instance)
{
	if (!is_playing(instance))
		return 0;
	float volume;
	alGetSourcef(sources[instance], AL_GAIN, &volume);
	return volume;
}

float sample::get_pitch(unsigned int instance)
{
	if (!is_playing(instance))
		return 0;
	float pitch;
	alGetSourcef(sources[instance], AL_PITCH, &pitch);
	return pitch;
}

void sample::set_panning(float panning)
{
	for (unsigned int i = 0; i < sourcecount; i++)
		set_panning(i, panning);
}

void sample::set_panning(unsigned int instance, float panning)
{
	if (is_playing(instance))
		alSource3f(sources[instance], AL_POSITION, sinf((float)M_PI_2 * panning), cosf((float)M_PI_2 * panning), 0.0f);
}

float sample::get_panning(unsigned int instance)
{
	if (!is_playing(instance))
		return 0;
	float x, y, z;
	alGetSource3f(sources[instance], AL_POSITION, &x, &y, &z);

	return asinf(x) / (float)M_PI_2;
}

float sample::get_length_seconds(unsigned int instance)
{
	if (!is_playing(instance))
		return 0;
	ALint bufferID, sizeInBytes, channels, bits, freq;
	alGetSourcei(sources[instance], AL_BUFFER, &bufferID); 
	alGetBufferi(bufferID, AL_SIZE, &sizeInBytes);
	alGetBufferi(bufferID, AL_CHANNELS, &channels);
	alGetBufferi(bufferID, AL_BITS, &bits);
	alGetBufferi(bufferID, AL_FREQUENCY, &freq);
	alSourcei(bufferID, AL_BUFFER, NULL);
	return sizeInBytes / channels / (bits/8) / (float)freq;
}

float sample::get_length_samples(unsigned int instance)
{
	if (!is_playing(instance))
		return 0;
	ALint bufferID, sizeInBytes, channels, bits;
	alGetSourcei(sources[instance], AL_BUFFER, &bufferID); 
	alGetBufferi(bufferID, AL_SIZE, &sizeInBytes);
	alGetBufferi(bufferID, AL_CHANNELS, &channels);
	alGetBufferi(bufferID, AL_BITS, &bits);
	alSourcei(bufferID, AL_BUFFER, NULL);
	return (float)(sizeInBytes * 8 / (channels * bits));
}

float sample::get_length_bytes(unsigned int instance)
{
	if (!is_playing(instance))
		return 0;
	ALint bufferID, sizeInBytes;
	alGetSourcei(sources[instance], AL_BUFFER, &bufferID); 
	alGetBufferi(bufferID, AL_SIZE, &sizeInBytes);
	alSourcei(bufferID, AL_BUFFER, NULL);
	return (float)sizeInBytes;
}

float sample::get_position_seconds(unsigned int instance) 
{
	if (!is_playing(instance))
		return 0;
	float offset = 0;
	alGetSourcef(sources[instance], AL_SEC_OFFSET, &offset);
	return offset;
}

float sample::get_position_samples(unsigned int instance) 
{
	if (!is_playing(instance))
		return 0;
	float offset = 0;
	alGetSourcef(sources[instance], AL_SAMPLE_OFFSET, &offset);
	return offset;
}

float sample::get_position_bytes(unsigned int instance) 
{
	if (!is_playing(instance))
		return 0;
	float offset = 0;
	alGetSourcef(sources[instance], AL_BYTE_OFFSET, &offset);
	return offset;
}

void sample::set_position_seconds(float value)
{
	for (unsigned int i = 0; i < sourcecount; i++)
		set_position_seconds(i, value);
}

void sample::set_position_seconds(unsigned int instance, float value)
{
	if (is_playing(instance))
		alSourcef(sources[instance], AL_SEC_OFFSET, value);
}

void sample::set_position_samples(float value)
{
	for (unsigned int i = 0; i < sourcecount; i++)
		set_position_samples(i, value);
}

void sample::set_position_samples(unsigned int instance, float value)
{
	if (is_playing(instance))
		alSourcef(sources[instance], AL_SAMPLE_OFFSET, value);
}

void sample::set_position_bytes(float value)
{
	for (unsigned int i = 0; i < sourcecount; i++)
		set_position_bytes(i, value);
}

void sample::set_position_bytes(unsigned int instance, float value)
{
	if (is_playing(instance))
		alSourcef(sources[instance], AL_BYTE_OFFSET, value);
}