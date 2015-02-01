/*
 * wavefile.cpp
 * 
 * Created by Marius Utheim on 14.06.2011
 * All rights reserved
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "wavefile.h"

static void read_little_4(FILE *src, int *dest)
{
	fread(dest, 4, 1, src);
}

static void read_little_2(FILE *src, short int *dest)
{
	fread(dest, 2, 1, src);
}

static void read_big_4(FILE *src, int *dest)
{
	char buffer;
	*dest = 0;
	for (int i = 1; i <= 4; i++)
	{
		*dest <<= 8;
		fread(&buffer, 1, 1, src);
		*dest |= buffer;
	}
}

static void read_big_2(FILE *src, int *dest)
{
	char *buffer;
	*dest = 0;
	for (int i = 1; i <= 2; i++)
	{
		*dest <<= 8;
		fread(buffer, 1, 1, src);
		*dest |= *buffer;
	}
}

wavefile_t *wavefile_open(char *path)
{
	wavefile_t *wavefile = 0;
	FILE *f;
	if (fopen_s(&f, path, "rb") != 0)
		goto error;
	
	wavefile = (wavefile_t *)malloc(sizeof(wavefile_t));
/*
 *	The WAVE file format
 *	--------------------
 *	endian	offset	field			size	description
 *	big		0		ChunkID			4		Contains "RIFF" (0x52494646)
 *	little	4		ChunkSize		4		36 + SubChunk2Size
 *	big		8		Format			4		Contains "WAVE" (0x57415645)
 *	big		12		Subchunk1ID		4		Contains "fmt " (0x666D7420)
 *	little	16		Subchunk1Size	4		16
 *	little	20		AudioFormat		2		1 if not compressed
 *	little	22		NumChannels		2		Number of channels (e.g. 2 for stereo)
 *	little	24		SampleRate		4		Sample rate (e.g. 44100)
 *	little	28		ByteRate		4		SampleRate * NumChannels * BitsPerSample / 8
 *	little	32		BlockAlign		2		NumChannels * BitsPerSample / 8
 *	little	34		BitsPerSample	2		Number of bits per sample
 *	big		36		Subchunk2ID		4		Contains "data" (0x64617461)
 *	little	40		Subchunk2Size	4		NumSamples * NumChannels * BitsPerSample / 8
 *	little	44		data					The sound data (size is Subchunk2Size)
 */
	read_big_4	 (f, &wavefile->ChunkID		 );
	read_little_4(f, &wavefile->ChunkSize	 );
	read_big_4	 (f, &wavefile->Format		 );
	read_big_4	 (f, &wavefile->Subchunk1ID	 );
	read_little_4(f, &wavefile->Subchunk1Size);
	read_little_2(f, &wavefile->AudioFormat	 );
	read_little_2(f, &wavefile->NumChannels	 );
	read_little_4(f, &wavefile->SampleRate	 );
	read_little_4(f, &wavefile->ByteRate	 );
	read_little_2(f, &wavefile->BlockAlign	 );
	read_little_2(f, &wavefile->BitsPerSample);
	read_big_4	 (f, &wavefile->Subchunk2ID	 );
	read_little_4(f, &wavefile->Subchunk2Size);

	wavefile->data = (char *)malloc(wavefile->Subchunk2Size);
	if (wavefile->data == 0)
		goto error;
	fread(wavefile->data, sizeof(char), wavefile->Subchunk2Size, f);

	fclose(f);
	
	return wavefile;

error:
	if (f != NULL)
		fclose(f);
	if (wavefile != 0)
	{
		if (wavefile->data != 0)
			free(wavefile->data);
		free(wavefile);
	}

	return NULL;
}

void wavefile_free(wavefile_t *wavefile)
{
	free(wavefile->data);
	free(wavefile);
}