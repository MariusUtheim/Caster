#define __caster_initialize

/*
    Initializes the extension
*/

begin
    var result;
    result = _caster_initialize();
    if (result == CASTER_OUT_OF_MEMORY)
        show_error("An out of meory error occurred while initializing Caster.", true);
    else if (result == CASTER_AL_ERROR)
        show_error("An OpenAL error occurred while initializing Caster.", true);
    else if (result > 0)
        show_error("An OpenAL error occurred while initializing Caster (error code: " + string(result) + ").", true);
    else if (result != 0)
        show_error("An unspecified error occurred while initializing Caster.", true);
end

#define __caster_deinitialize

/* 
    Deinitializes the extension
*/

begin
    var result;
    result = _caster_deinitialize();
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster tried to deinitialize before it was initialized.", true);
    else if (result > 0)
        show_error("OpenAL caused an error while deinitializing (error code: " + string(result) + ").", true);
end

#define caster_version

/*
    Gets the version of the native extension
        returns - A string representing the version of the extension
*/

return _caster_version();

#define caster_load

/*
    Loads a .wav or .ogg file
    
    arg0 - the file path
    returns - a handle to the loaded file
*/

begin
    var result;
    result = _caster_load(argument0);
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_OUT_OF_MEMORY)
        show_error("Out of memory", true);
    else if (result == CASTER_FILE_NOT_FOUND)
        show_error("Could not open '" + argument0 + "'.", false);
    else if (result == CASTER_INVALID_FILE)
        show_error("The file '" + argument0 + "' is not a valid .ogg or .wav file.", false);
    else if (result == CASTER_UNSPECIFIED)
        show_error("An unspecified error occurred in Caster when loading a file.", true);
    return result;
end

#define caster_free

/*
    Frees the specified sample
        arg0 - the sample handle
*/

begin
    if (is_string(argument0))
        show_error("Sample handles should not be text strings.", true);
    var result;
    result = _caster_free(argument0);
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);
    else if (result > 0)
        show_error("OpenAl caused an error (error code: " + string(result) + ").", false);
end

#define caster_play

/*
    Plays the specified sample once, with the specified volume and pitch
        arg0 - handle
        arg1 - volume
        arg2 - pitch
        returns - a handle for the playing sound instance
*/

begin
    if (is_string(argument0))
        show_error("Sample handles should not be text strings.", true);
    if (argument1 < 0)
        show_error("Argument 'volume' must be greater than or equal to 0 when playing a sound.", false);
    if (argument2 <= 0)
        show_error("Argument 'pitch' must be greater than 0 when playing a sound.", false);
    var result;
    result = _caster_play(argument0, argument1, argument2);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);
    else if (result == CASTER_INSTANCE_OVERFLOW)
        show_error("Too many simulatneous instances.", false);
    else if (result == CASTER_UNSPECIFIED)
        show_error("An unspecified error occurred in Caster.", false);

    return result;
end

#define caster_loop

/*
    Plays the specified sample with looping, with the specified volume and pitch
        arg0 - handle
        arg1 - volume
        arg2 - pitch
        returns - a handle for the playing sound instance
*/

begin
    if (is_string(argument0))
        show_error("Sample handles should not be text strings.", true);
    if (argument1 < 0)
        show_error("Argument 'volume' must be greater than or equal to 0 when playing a sound.", false);
    if (argument2 <= 0)
        show_error("Argument 'pitch' must be greater than 0 when playing a sound.", false);
    var result;
    result = _caster_loop(argument0, argument1, argument2);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);
    else if (result == CASTER_INSTANCE_OVERFLOW)
        show_error("Too many simulatneous instances.", false);
    else if (result == CASTER_UNSPECIFIED)
        show_error("An unspecified error occurred in Caster.", false);

    return result;
end

#define caster_stop
/*
    Stops the specified instance or sample.
        arg0 - the instance or sample to stop. Can be noone or all.
*/

begin
    if (is_string(argument0))
        show_error("Sample handles should not be text strings.", true);
    
    var result;
    result = _caster_stop(argument0);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);
    else if (result == CASTER_UNSPECIFIED)
        show_error("An unspecified error occurred in Caster.", false);
    else if (result > 0)
        show_error("OpenAl caused an error (error code: " + string(result) + ").", false);
end

#define caster_pause
/*
    Pauses the specified instance or sample.
        arg0 - the instance or sample to pause. Can be noone or all.
*/

begin
    if (is_string(argument0))
        show_error("Sample handles should not be text strings.", true);
    
    var result;
    result = _caster_pause(argument0);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);
    else if (result == CASTER_UNSPECIFIED)
        show_error("An unspecified error occurred in Caster.", false);
    else if (result > 0)
        show_error("OpenAl caused an error (error code: " + string(result) + ").", false);
end

#define caster_resume
/*
    Resumes the specified instance or sample.
        arg0 - the instance or sample to resume. Can be noone or all.
*/

begin
    if (is_string(argument0))
        show_error("Sample handles should not be text strings.", true);
    
    var result;
    result = _caster_resume(argument0);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);
    else if (result == CASTER_UNSPECIFIED)
        show_error("An unspecified error occurred in Caster.", false);
    else if (result > 0)
        show_error("OpenAl caused an error (error code: " + string(result) + ").", false);
end

#define caster_is_playing
/*
    Returns whether the specified instance or sample is playing.
        arg0 - the instance or sample. Can be noone or all.
        returns - 1 if the instance is playing; or 1 if any instance of the sample is playing; or 1 if arg0 is all and any sample is playing; or 0 if arg0 is noone or otherwise.
*/

begin
    if (is_string(argument0))
        show_error("Sample handles should not be text strings.", true);
    
    var result;
    result = _caster_is_playing(argument0);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);
    else if (result == CASTER_UNSPECIFIED)
        show_error("An unspecified error occurred in Caster.", false);

    return result;
end

#define caster_get_volume


begin
    var result;
    result = _caster_get_volume(argument0);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);

    return result;
end

#define caster_get_pitch


begin
    var result;
    result = _caster_get_pitch(argument0);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);

    return result;
end

#define caster_get_panning


begin
    var result;
    result = _caster_get_panning(argument0);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);

    return result;
end

#define caster_get_length


begin
    var result;
    result = _caster_get_length(argument0,argument1);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);

    return result;
end

#define caster_set_volume


begin
    if (argument1 < 0)
        show_error("Argument 'volume' must be greater than or equal to zero.", false);
    
    var result;
    result = _caster_set_volume(argument0, argument1);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);
end

#define caster_set_pitch


begin
    if (argument1 <= 0)
        show_error("Argument 'pitch' must be greater than zero.", false);
    
    var result;
    result = _caster_set_pitch(argument0, argument1);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);
end

#define caster_set_panning


begin
    if (argument1 < -1 || argument1 > 1)
        show_error("Argument 'panning' must be in the interval [-1, 1].", false);
    
    var result;
    result = _caster_set_panning(argument0, argument1);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);
end

#define caster_get_position


begin
    var result;
    result = _caster_get_position(argument0, argument1);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);

    return result;
end

#define caster_set_position


begin
    var result;
    result = _caster_set_position(argument0, argument1, argument2);
    
    if (result == CASTER_NOT_INITIALIZED)
        show_error("Caster is not initialized.", true);
    else if (result == CASTER_INVALID_HANDLE)
        show_error("Invalid sample handle.", false);
    else if (result == CASTER_HANDLE_NOT_FOUND)
        show_error("Handle not found.", false);
end
