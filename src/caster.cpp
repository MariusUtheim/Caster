/*
 * caster.cpp
 * 
 * Created by Marius Utheim on 27.05.2011
 * All rights reserved
 *
 */

#include <AL/al.h>
#include <AL/alc.h>
#include <stdio.h>
#include <string.h>
#include "caster.h"
#include "sample.h"
#include "lot.h"

ALCcontext *context;
ALCdevice *device;
lot_t *handles;
bool initialized = false;
bool error_occurred = false;

int errornum;

#define SetError(error) { \
	error_occurred = true; \
	errornum = error; \
	return -1; }
#define InitializedTest() \
	if (!initialized) SetError(CASTER_ERROR_NOT_INITIALIZED)

// source_id = input >> SOURCE_OFFSET,
// sample_id = input & SAMPLE_MASK
// output = sample_id | (source_id << SOURCE_OFFSET)
#define SOURCE_OFFSET 24			
#define SAMPLE_MASK 0x00FFFFFF		

static bool GetSample(double input, sample **s, unsigned int *inst)
{
	if (!initialized)
	{
		error_occurred = true;
		errornum = CASTER_ERROR_NOT_INITIALIZED;
		return false;
	}

	unsigned int in = (unsigned int)input;
	if (input != in)
	{
		error_occurred = true;
		errornum = CASTER_ERROR_INVALID_HANDLE;
		return false;
	}

	if (inst != 0)
		*inst = in >> SOURCE_OFFSET;

	*s = (sample *)lot_place(handles, in & SAMPLE_MASK); 

	if (s == 0)
	{
		error_occurred = true;
		errornum = CASTER_ERROR_HANDLE_NOT_FOUND;
		return false;
	}

	return true;
}

// is_ogg and is_wav should be extended to actually check the header
// in the file itself
static bool is_ogg(char *file)
{
	int i;
	for (i = 0; file[i] != 0; i++);
	if (i < 4)
		return false;
	
	return strcmp(file + i - 4, ".ogg") == 0;
}

static bool is_wav(char *file)
{
	int i;
	for (i = 0; file[i] != 0; i++);
	if (i < 4)
		return false;
	
	return strcmp(file + i - 4, ".wav") == 0;
}

#ifdef __cplusplus
extern "C"
{
#endif

	char *caster_version()
	{
#if defined(_WIN32) || defined(_WIN64)
		return "Windows 1.1b";
#elif defined(__APPLE__)
		return "Mac 1.1b";
#else
#error Unsupported platform
#endif
	}

	double caster_initialize()
	{
		handles = lot_create();
		float orientation[] = { 0, 1, 0, 0, 0, 1 };
		if (handles == 0)
			goto error;

		device = alcOpenDevice(0);
		if (device == 0)
			goto error;

		context = alcCreateContext(device, 0);
		if (context == 0)
			goto error;

		alcMakeContextCurrent(context);
		alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
		alListenerfv(AL_ORIENTATION, orientation);

		initialized = true;
		error_occurred = false;
		errornum = CASTER_ERROR_NOERROR;
		return 1;

error:
		if (handles != 0)
			lot_destroy(handles);
		if (device != 0)
			alcCloseDevice(device);
		if (context != 0)
			alcDestroyContext(context);
		
		error_occurred = true;
		errornum = CASTER_ERROR_INITIALIZATION;
		return -1;
	}

	double caster_deinitialize()
	{
		if (!initialized)
			SetError(CASTER_ERROR_NOT_INITIALIZED);
		lot_iterator_t *iter = lot_iterator_create(handles);
		sample *current;

		while ((current = reinterpret_cast<sample *>(lot_next(iter, 0))) != 0)
			delete current;

		lot_destroy(handles);
		alcDestroyContext(context);
		alcCloseDevice(device);

		if (alGetError() != AL_NO_ERROR)
			SetError(CASTER_ERROR_DEINITIALIZATION);

		return 1;
	}

	double caster_load(char *file)
	{
		InitializedTest();
		double result;
		sample *s = new sample();
		if (s == 0)
			SetError(CASTER_ERROR_OUT_OF_MEMORY);

		FILE *f;
		if (fopen_s(&f, file, "r") != 0)
			SetError(CASTER_ERROR_FILE_NOT_FOUND)
		else
			fclose(f);

		if (is_ogg(file))
			result = s->load_ogg(file);
		else if (is_wav(file))
			result = s->load_wav(file);
		else
			SetError(CASTER_ERROR_INVALID_FILE);

		if (result < 0)
		{
			delete s;
			if (result == -1)
				errornum = CASTER_ERROR_UNSPECIFIED; // file not found; this should have been handled above
			else if (result == -2)
				errornum = CASTER_ERROR_INVALID_FILE;
			else if (result == -3 || result == -4)
				errornum = CASTER_ERROR_OUT_OF_MEMORY;
			else
				errornum = CASTER_ERROR_UNSPECIFIED;
			error_occurred = true;
			return -1;
		}

		return lot_add(handles, s);
	}

	double caster_free(double handle)
	{
		InitializedTest();

		if (handle == GM_NOONE)
			return 1;

		if (handle == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			sample *s;

			while ((s = (sample *)lot_next(iter, 0)) != 0)
				s->free();

			lot_destroy(handles);
			handles = lot_create();

			lot_iterator_destroy(iter);
			return 1;
		}

		sample *s;
		if (!GetSample(handle, &s, 0))
			return -1;

		lot_remove(handles, s);
		s->free();
		delete s;

		if (alGetError() != AL_NO_ERROR)
			SetError(CASTER_ERROR_UNSPECIFIED);

		return 1;
	}

	double caster_play(double handle, double volume, double pitch)
	{
		InitializedTest();
		if (handle == GM_NOONE)
			return 1;
		
		sample *s;
		if (!GetSample(handle, &s, 0))
			return -1;

		int result = s->play(false, (float)volume, (float)pitch) + 1;
		if (result == -1)
			SetError(CASTER_ERROR_UNSPECIFIED)
		else if (result == -2)
			SetError(CASTER_ERROR_INSTANCE_OVERFLOW)
		else
			return (((unsigned int)result) << SOURCE_OFFSET)
				   | (unsigned int)handle;
	}

	double caster_loop(double handle, double volume, double pitch)
	{
		InitializedTest();
		if (handle == GM_NOONE)
			return 1;

		sample *s;
		if (!GetSample(handle, &s, 0))
			return -1;

		int result = s->play(true, (float)volume, (float)pitch) + 1;
		if (result == -1)
			SetError(CASTER_ERROR_UNSPECIFIED)
		else if (result == -2)
			SetError(CASTER_ERROR_INSTANCE_OVERFLOW)
		else
			return (((unsigned int)result) << SOURCE_OFFSET)
				   | (unsigned int)handle;
	}

	double caster_stop(double handle)
	{
		InitializedTest();
		if (handle == GM_NOONE)
			return 1;

		if (handle == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			sample *s;

			while ((s = (sample *)lot_next(iter, 0)) != 0)
				s->stop_all();

			lot_iterator_destroy(iter);
			return 1;
		}

		sample *s;
		unsigned int inst;
		if (!GetSample(handle, &s, &inst))
			return -1;

		if (inst == 0)
		{
			if (s->stop_all() < 0)
				SetError(CASTER_ERROR_UNSPECIFIED)
		}
		else
		{
			if (s->stop(inst - 1) < 0)
				SetError(CASTER_ERROR_UNSPECIFIED)
		}

		return 1;
	}

	double caster_pause(double handle)
	{
		InitializedTest();
		if (handle == GM_NOONE)
			return 1;

		if (handle == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			sample *s;

			while ((s = (sample *)lot_next(iter, 0)) != 0)
				s->pause_all();

			lot_iterator_destroy(iter);
			return 1;
		}

		sample *s;
		unsigned int inst;
		if (!GetSample(handle, &s, &inst))
			return -1;

		if (inst == 0)
		{
			if (s->pause_all() < 0)
				SetError(CASTER_ERROR_UNSPECIFIED)
		}
		else
		{
			if (s->pause(inst - 1) < 0)
				SetError(CASTER_ERROR_UNSPECIFIED)
		}
		
		return 1;
	}

	double caster_resume(double handle)
	{
		InitializedTest();
		if (handle == GM_NOONE)
			return 1;

		if (handle == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			sample *s;

			while ((s = (sample *)lot_next(iter, 0)) != 0)
				s->resume_all();

			lot_iterator_destroy(iter);
			return 1;
		}

		sample *s;
		unsigned int inst;
		if (!GetSample(handle, &s, &inst))
			return -1;

		if (inst == 0)
		{
			if (s->resume_all() < 0)
				SetError(CASTER_ERROR_UNSPECIFIED);
		}
		else
		{
			if (s->resume(inst - 1) < 0)
				SetError(CASTER_ERROR_UNSPECIFIED);
		}

		return 1;
	}

	double caster_is_playing(double handle)
	{
		InitializedTest();
		if (handle == GM_NOONE)
			return 0;

		if (handle == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			sample *s;

			while ((s = (sample *)lot_next(iter, 0)) != 0)
				if (s->is_playing())
				{
					lot_iterator_destroy(iter);
					return 1;
				}

			lot_iterator_destroy(iter);
			return 0;
		}

		sample *s;
		unsigned int inst;
		if (!GetSample(handle, &s, &inst))
			return -1;

		if (inst == 0)
			return s->is_playing();
		else
			return s->is_playing(inst - 1);
	}

	double caster_get_volume(double instance)
	{
		InitializedTest();

		sample *s;
		unsigned int inst;
		if (!GetSample(instance, &s, &inst))
			return -1;
		
		return s->get_volume(inst == 0 ? 0 : inst - 1);
	}
	
	double caster_get_pitch(double instance)
	{
		InitializedTest();

		sample *s;
		unsigned int inst;
		if (!GetSample(instance, &s, &inst))
			return -1;
		
		return s->get_pitch(inst == 0 ? 0 : inst - 1);
	}

	double caster_get_panning(double instance)
	{
		InitializedTest();

		sample *s;
		unsigned int inst;
		if (!GetSample(instance, &s, &inst))
			return -1;

		return s->get_panning(inst == 0 ? 0 : inst - 1);
	}

	double caster_set_volume(double instance, double volume)
	{
		InitializedTest();

		if (instance == GM_NOONE)
			return 1;

		if (instance == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			sample *s;

			while ((s = (sample *)lot_next(iter, 0)) != 0)
				s->set_volume((float)volume);

			lot_iterator_destroy(iter);
			return 0;
		}

		sample *s;
		unsigned int inst;
		if (!GetSample(instance, &s, &inst))
			return -1;

		if (volume < 0)
			SetError(CASTER_ERROR_ARGUMENT_OUT_OF_BOUNDS);

		if (inst == 0)
			s->set_volume((float)volume);
		else
			s->set_volume(inst - 1, (float)volume);
		return 1;
	}
	
	double caster_set_pitch(double instance, double pitch)
	{
		InitializedTest();

		if (instance == GM_NOONE)
			return 1;

		if (instance == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			sample *s;

			while ((s = (sample *)lot_next(iter, 0)) != 0)
				s->set_pitch((float)pitch);

			lot_iterator_destroy(iter);
			return 0;
		}

		sample *s;
		unsigned int inst;
		if (!GetSample(instance, &s, &inst))
			return -1;

		if (pitch <= 0)
			SetError(CASTER_ERROR_ARGUMENT_OUT_OF_BOUNDS);

		if (inst == 0)
			s->set_pitch((float)pitch);
		else
			s->set_pitch(inst - 1, (float)pitch);
		return 1;
	}

	double caster_set_panning(double instance, double panning)
	{
		InitializedTest();

		if (instance == GM_NOONE)
			return 1;

		if (instance == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			sample *s;

			while ((s = (sample *)lot_next(iter, 0)) != 0)
				s->set_panning((float)panning);

			lot_iterator_destroy(iter);
			return 0;
		}

		sample *s;
		unsigned int inst;
		if (!GetSample(instance, &s, &inst))
			return -1;

		if (panning < -1 || panning > 1)
			SetError(CASTER_ERROR_ARGUMENT_OUT_OF_BOUNDS);

		if (inst == 0)
			s->set_panning((float)panning);
		else
			s->set_panning(inst - 1, (float)panning);
		return 1;
	}

	double caster_error_occurred()
	{
		InitializedTest();

		bool result = error_occurred;
		error_occurred = false;
		return result;
	}

	char *caster_error_message()
	{
		if (!initialized)
			return "Library is not initialized"; 

		switch (errornum)
		{
		case CASTER_ERROR_NOERROR:
			return "No error";

		case CASTER_ERROR_INITIALIZATION:
			return "Error in initialization";

		case CASTER_ERROR_DEINITIALIZATION:
			return "Unexpected error while deinitializing";

		case CASTER_ERROR_FILE_NOT_FOUND:
			return "File not found";

		case CASTER_ERROR_INVALID_FILE:
			return "Invalid file";

		case CASTER_ERROR_NOT_INITIALIZED:
			return "Library is not initialized";

		case CASTER_ERROR_HANDLE_NOT_FOUND:
			return "Handle not found";

		case CASTER_ERROR_INSTANCE_NOT_FOUND:
			return "Instance not found";

		case CASTER_ERROR_OUT_OF_MEMORY:
			return "Out of memory";

		case CASTER_ERROR_ARGUMENT_OUT_OF_BOUNDS:
			return "Argument is out of bounds";

		case CASTER_ERROR_INVALID_HANDLE:
			return "Invalid handle index";

		case CASTER_ERROR_INSTANCE_OVERFLOW:
			return "Too many sound instances; no more than 256 instances can be played simultaneously";

		case CASTER_ERROR_UNSPECIFIED:
		default:
			return "Internal error";
		}
	}

#ifdef __cplusplus
} /* extern "C" */
#endif