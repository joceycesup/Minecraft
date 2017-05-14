#include "chunk.h"

float NYChunk::_WorldVert[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 3 * 4 * 6];
float NYChunk::_WorldCols[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 3 * 4 * 6];
float NYChunk::_WorldNorm[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 3 * 4 * 6];
float NYChunk::_WorldUV[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 2 * 4 * 6];

float NYChunk::_WorldVertTransparent[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 3 * 4 * 6];
float NYChunk::_WorldColsTransparent[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 3 * 4 * 6];
float NYChunk::_WorldNormTransparent[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 3 * 4 * 6];
float NYChunk::_WorldUVTransparent[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 2 * 4 * 6];
