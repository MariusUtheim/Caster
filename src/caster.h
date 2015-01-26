/*
 * caster.h
 * 
 * Created by Marius Utheim on 27.05.2011
 * All rights reserved
 *
 */

#ifndef CASTER_H_
#define CASTER_H_

#if defined(_WIN32) || defined(_WIN64)
#define EXPORT __declspec(dllexport)
#elif defined(__APPLE__)
#define EXPORT __attribute__((visiblility("default")))
#endif

#define GM_NOONE -4
#define GM_ALL -3

#define CASTER_ERROR_NOERROR -1
#define CASTER_ERROR_INITIALIZATION 1
#define CASTER_ERROR_FILE_NOT_FOUND 2
#define CASTER_ERROR_INVALID_FILE 3
#define CASTER_ERROR_NOT_INITIALIZED 4
#define CASTER_ERROR_HANDLE_NOT_FOUND 5
#define CASTER_ERROR_INSTANCE_NOT_FOUND 6
#define CASTER_ERROR_OUT_OF_MEMORY 7
#define CASTER_ERROR_ARGUMENT_OUT_OF_BOUNDS 8
#define CASTER_ERROR_DEINITIALIZATION 9
#define CASTER_ERROR_INVALID_HANDLE 10
#define CASTER_ERROR_INSTANCE_OVERFLOW 11
#define CASTER_ERROR_UNSPECIFIED 999

#ifdef __cplusplus
extern "C"
{
#endif
	EXPORT char  *caster_version();
	EXPORT double caster_initialize();
	EXPORT double caster_deinitialize();
	EXPORT double caster_load(char *file);
	EXPORT double caster_free(double handle);
	EXPORT double caster_play(double handle, double volume, double pitch);
	EXPORT double caster_loop(double handle, double volume, double pitch);
	EXPORT double caster_stop(double handle);
	EXPORT double caster_pause(double handle);
	EXPORT double caster_resume(double handle);
	EXPORT double caster_is_playing(double handle);
	EXPORT double caster_get_volume(double handle);
	EXPORT double caster_set_volume(double handle, double volume);
	EXPORT double caster_get_pitch(double handle);
	EXPORT double caster_set_pitch(double handle, double pitch);
	EXPORT double caster_get_panning(double handle);
	EXPORT double caster_set_panning(double handle, double panning);
	EXPORT double caster_error_occurred();
	EXPORT char  *caster_error_message();
#ifdef __cplusplus
}
#endif

#endif /* CASTER_H_ */