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
#include "Sample.h"
#include "lot.h"

ALCcontext *context;
ALCdevice *device;
lot_t *handles;
bool initialized = false;


#define SOURCE_OFFSET 24			
#define SAMPLE_MASK 0x00FFFFFF		

#define GET_SAMPLE(handle,s,inst) { \
	int __err; \
	if ((__err = GetSample(handle, s, inst))) \
		return __err; }

static int GetSample(double input, Sample **s, unsigned int *inst)
{
	unsigned int in = (unsigned int)input;
	if (input != in)
		return CASTER_INVALID_HANDLE;

	if (inst != 0)
		*inst = in >> SOURCE_OFFSET;

	*s = (Sample *)lot_place(handles, in & SAMPLE_MASK); 

	if (s == 0)
		return CASTER_HANDLE_NOT_FOUND;

	return 0;
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
		return "Windows 1.2.01";
#elif defined(__APPLE__)
		return "Mac 1.2.01";
#else
#error Unsupported platform
#endif
	}

	double caster_initialize()
	{
		float orientation[] = { 0, 1, 0, 0, 0, 1 };
		handles = lot_create();
		if (handles == 0)
			return CASTER_OUT_OF_MEMORY;

		device = alcOpenDevice(0);
		if (device == 0)
		{
			lot_destroy(handles);
			return CASTER_AL_ERROR;
		}

		context = alcCreateContext(device, 0);
		if (context == 0)
		{
			alcCloseDevice(device);
			lot_destroy(handles);
			return CASTER_AL_ERROR;
		}

		alcMakeContextCurrent(context);
		alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
		alListenerfv(AL_ORIENTATION, orientation);

		ALenum err = alGetError();
		if (err != AL_NO_ERROR)
			return err;

		initialized = true;
		return 0;
	}

	double caster_deinitialize()
	{
		if (!initialized)
			return CASTER_NOT_INITIALIZED;
		lot_iterator_t *iter = lot_iterator_create(handles);
		Sample *current;

		while ((current = reinterpret_cast<Sample *>(lot_next(iter, 0))) != 0)
			delete current;

		lot_destroy(handles);
		alcDestroyContext(context);
		alcCloseDevice(device);

		return alGetError();
	}

	double caster_load(char *file)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;

		double result;
		Sample *s = new Sample();
		if (s == 0)
			return CASTER_OUT_OF_MEMORY;

		FILE *f;
		if (fopen_s(&f, file, "r") != 0)
			return CASTER_FILE_NOT_FOUND;
		else
			fclose(f);

		if (is_ogg(file))
			result = s->load_ogg(file);
		else if (is_wav(file))
			result = s->load_wav(file);
		else
			return CASTER_INVALID_FILE;

		if (result < 0)
		{
			delete s;
			if (result == -1)
				return CASTER_UNSPECIFIED; // file not found; this should have been handled above
			else if (result == -2)
				return CASTER_INVALID_FILE;
			else if (result == -3 || result == -4)
				return CASTER_OUT_OF_MEMORY;
			else
				return CASTER_UNSPECIFIED;
		}



		return lot_add(handles, s);
	}

	double caster_free(double handle)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;
		if (handle == GM_NOONE)
			return 0;

		if (handle == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			Sample *s;

			while ((s = (Sample *)lot_next(iter, 0)) != 0)
				s->free();

			lot_destroy(handles);
			handles = lot_create();

			lot_iterator_destroy(iter);
			return 0;
		}

		Sample *s;
		GET_SAMPLE(handle, &s, 0);
		
		lot_remove(handles, s);
		s->free();
		delete s;

		return alGetError();
	}

	double caster_play(double handle, double volume, double pitch)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;
		if (handle == GM_NOONE)
			return 0;
		
		Sample *s;
		GET_SAMPLE(handle, &s, 0);

		int result = s->play(false, (float)volume, (float)pitch) + 1;
		if (result == -1)
			return CASTER_UNSPECIFIED;
		else if (result == -2)
			return CASTER_INSTANCE_OVERFLOW;
		else
			return (((unsigned int)result) << SOURCE_OFFSET)
				   | (unsigned int)handle;
	}

	double caster_loop(double handle, double volume, double pitch)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;
		if (handle == GM_NOONE)
			return 0;

		Sample *s;
		GET_SAMPLE(handle, &s, 0);

		int result = s->play(true, (float)volume, (float)pitch) + 1;
		if (result == -1)
			return CASTER_UNSPECIFIED;
		else if (result == -2)
			return CASTER_INSTANCE_OVERFLOW;
		else
			return (((unsigned int)result) << SOURCE_OFFSET)
				   | (unsigned int)handle;
	}

	double caster_stop(double handle)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;
		if (handle == GM_NOONE)
			return 0;

		if (handle == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			Sample *s;

			while ((s = (Sample *)lot_next(iter, 0)) != 0)
				s->stop_all();

			lot_iterator_destroy(iter);
			return 0;
		}

		Sample *s;
		unsigned int inst;
		GET_SAMPLE(handle, &s, &inst);

		if (inst == 0)
		{
			if (s->stop_all() < 0)
				return CASTER_UNSPECIFIED;
		}
		else
		{
			if (s->stop(inst - 1) < 0)
				return CASTER_UNSPECIFIED;
		}

		return alGetError();
	}

	double caster_pause(double handle)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;
		if (handle == GM_NOONE)
			return 0;

		if (handle == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			Sample *s;

			while ((s = (Sample *)lot_next(iter, 0)) != 0)
				s->pause_all();

			lot_iterator_destroy(iter);
			return 0;
		}

		Sample *s;
		unsigned int inst;
		GET_SAMPLE(handle, &s, &inst);

		if (inst == 0)
		{
			if (s->pause_all() < 0)
				return CASTER_UNSPECIFIED;
		}
		else
		{
			if (s->pause(inst - 1) < 0)
				return CASTER_UNSPECIFIED;
		}
		
		return alGetError();
	}

	double caster_resume(double handle)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;
		if (handle == GM_NOONE)
			return 0;

		if (handle == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			Sample *s;

			while ((s = (Sample *)lot_next(iter, 0)) != 0)
				s->resume_all();

			lot_iterator_destroy(iter);
			return 0;
		}

		Sample *s;
		unsigned int inst;
		GET_SAMPLE(handle, &s, &inst);

		if (inst == 0)
		{
			if (s->resume_all() < 0)
				return CASTER_UNSPECIFIED;
		}
		else
		{
			if (s->resume(inst - 1) < 0)
				return CASTER_UNSPECIFIED;
		}

		return alGetError();
	}

	double caster_is_playing(double handle)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;
		if (handle == GM_NOONE)
			return 0;

		if (handle == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			Sample *s;

			while ((s = (Sample *)lot_next(iter, 0)) != 0)
				if (s->is_playing())
				{
					lot_iterator_destroy(iter);
					return 1;
				}

			lot_iterator_destroy(iter);
			return 0;
		}

		Sample *s;
		unsigned int inst;
		GET_SAMPLE(handle, &s, &inst);

		if (inst == 0)
			return s->is_playing();
		else
			return s->is_playing(inst - 1);
	}

	double caster_get_volume(double instance)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;

		Sample *s;
		unsigned int inst;
		GET_SAMPLE(instance, &s, &inst);
		
		return s->get_volume(inst == 0 ? 0 : inst - 1);
	}
	
	double caster_get_pitch(double instance)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;

		Sample *s;
		unsigned int inst;
		GET_SAMPLE(instance, &s, &inst);
		
		return s->get_pitch(inst == 0 ? 0 : inst - 1);
	}

	double caster_get_panning(double instance)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;

		Sample *s;
		unsigned int inst;
		GET_SAMPLE(instance, &s, &inst);

		return s->get_panning(inst == 0 ? 0 : inst - 1);
	}

	double caster_set_volume(double instance, double volume)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;

		if (instance == GM_NOONE)
			return 0;

		if (instance == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			Sample *s;

			while ((s = (Sample *)lot_next(iter, 0)) != 0)
				s->set_volume((float)volume);

			lot_iterator_destroy(iter);
			return 0;
		}

		Sample *s;
		unsigned int inst;
		GET_SAMPLE(instance, &s, &inst);

		if (inst == 0)
			s->set_volume((float)volume);
		else
			s->set_volume(inst - 1, (float)volume);
		return 0;
	}
	
	double caster_set_pitch(double instance, double pitch)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;

		if (instance == GM_NOONE)
			return 0;

		if (instance == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			Sample *s;

			while ((s = (Sample *)lot_next(iter, 0)) != 0)
				s->set_pitch((float)pitch);

			lot_iterator_destroy(iter);
			return 0;
		}

		Sample *s;
		unsigned int inst;
		GET_SAMPLE(instance, &s, &inst);

		if (inst == 0)
			s->set_pitch((float)pitch);
		else
			s->set_pitch(inst - 1, (float)pitch);
		return 0;
	}

	double caster_set_panning(double instance, double panning)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;

		if (instance == GM_NOONE)
			return 1;

		if (instance == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			Sample *s;

			while ((s = (Sample *)lot_next(iter, 0)) != 0)
				s->set_panning((float)panning);

			lot_iterator_destroy(iter);
			return 0;
		}

		Sample *s;
		unsigned int inst;
		GET_SAMPLE(instance, &s, &inst);

		if (inst == 0)
			s->set_panning((float)panning);
		else
			s->set_panning(inst - 1, (float)panning);
		return 1;
	}
	
	double caster_length(double instance)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;

		Sample *s;
		unsigned int inst;
		GET_SAMPLE(instance, &s, &inst);

		return s->get_length(inst == 0 ? 0 : inst - 1);
	}
	
	double caster_position(double instance)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;

		Sample *s;
		unsigned int inst;
		GET_SAMPLE(instance, &s, &inst);

		return s->get_position(inst == 0 ? 0 : inst - 1);
	}
	
	double caster_seek(double instance, double time)
	{
		if (!initialized) return CASTER_NOT_INITIALIZED;

		if (instance == GM_NOONE)
			return 1;
		
		if (instance == GM_ALL)
		{
			lot_iterator_t *iter = lot_iterator_create(handles);
			Sample *s;

			while ((s = (Sample *)lot_next(iter, 0)) != 0)
				s->seek((float)time);

			lot_iterator_destroy(iter);
			return 0;
		}
		
		Sample *s;
		unsigned int inst;
		GET_SAMPLE(instance, &s, &inst);

		if (inst == 0)
			s->seek((float)time);
		else
			s->seek(inst - 1, (float)time);
		return 1;
	}

#ifdef __cplusplus
} /* extern "C" */
#endif