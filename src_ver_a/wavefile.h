/*
 * wavefile.h
 * 
 * Created by Marius Utheim on 14.06.2011
 * All rights reserved
 *
 */

#ifndef WAVEFILE_H_
#define WAVEFILE_H_

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
 *	little	34		BitsPerSample	2		Number of bits per sample (e.g. 8, 16)
 *	big		36		Subchunk2ID		4		Contains "data" (0x64617461)
 *	little	40		Subchunk2Size	4		NumSamples * NumChannels * BitsPerSample / 8
 *	little	44		data					The sound data
 */

typedef struct wavefile {
	int			ChunkID;
	int			ChunkSize;
	int			Format;
	int			Subchunk1ID;
	int			Subchunk1Size;
	short int	AudioFormat;
	short int	NumChannels;
	int			SampleRate;
	int			ByteRate;
	short int	BlockAlign;
	short int	BitsPerSample;
	int			Subchunk2ID;
	int			Subchunk2Size;
	char	   *data;
} wavefile_t;

wavefile_t *wavefile_open(char *path);
void wavefile_free(wavefile_t *wavefile);
void wavefile_buffer(wavefile_t *wavefile, char **buffer);


#endif