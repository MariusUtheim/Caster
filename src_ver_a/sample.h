/*
 * sample.h
 * 
 * Created by Marius Utheim on 27.05.2011
 * All rights reserved
 *
 */

#ifndef SAMPLE_H_
#define SAMPLE_H_

#if defined(_WIN32)
#include <al/al.h>
#include <al/alc.h>
#elif defined(__APPLE__)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif
#include "lot.h"


class sample
{
private:
	ALuint buffer;
	ALuint *sources;
	unsigned int sourcecount;
	void newsources(int n, int offset);
	int check();

public:
	sample();
	~sample();
	int load_ogg(char *path);
	int load_wav(char *path);
	int free();
	int play(bool looping, float volume, float pitch);
	int stop(unsigned int instance);
	int stop_all();
	int pause(unsigned int instance);
	int pause_all();
	int resume(unsigned int instance);
	int resume_all();

	int is_playing();
	int is_playing(unsigned int instance);
	int seek(double time);

	void set_looping(unsigned int instance, bool loop);
	bool get_looping(unsigned int instance);
	void set_volume(float volume);
	void set_volume(unsigned int instance, float volume);
	float get_volume(unsigned int instance);
	void set_pitch(float pitch);
	void set_pitch(unsigned int instance, float pitch);
	float get_pitch(unsigned int instance);
	void set_panning(float panning);
	void set_panning(unsigned int instance, float panning);
	float get_panning(unsigned int instance);
	float get_length_seconds(unsigned int instance);
	float get_length_samples(unsigned int instance);
	float get_length_bytes(unsigned int instance);
	float get_position_seconds(unsigned int instance);
	float get_position_samples(unsigned int instance);
	float get_position_bytes(unsigned int instance);
	void set_position_seconds(float value);
	void set_position_samples(float value);
	void set_position_bytes(float value);
	void set_position_seconds(unsigned int instance, float value);
	void set_position_samples(unsigned int instance, float value);
	void set_position_bytes(unsigned int instance, float value);

	int instance_count();

	/*
	void pause(unsigned int instance);
	void pause_all();
	void set_panning(unsigned int instance, float panning);
	float get_panning(unsigned int instance);
	*/
};

#endif /* SAMPLE_H_ */