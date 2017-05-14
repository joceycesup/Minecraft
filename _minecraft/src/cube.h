#pragma once

#include <stdio.h>
#include "engine/utils/types.h"

#define FACES_MASK 0x003f
#define TRANSPARENCY_MASK 0x0040
#define SOLID_MASK 0x0080
#define BREAKABLE_MASK 0x0100
#define DIRTY_MASK 0x0200

#define FACE_NONE 0x0000
#define FACE_X_MINUS 0x0001
#define FACE_Y_MINUS 0x0002
#define FACE_Z_MINUS 0x0004
#define FACE_X_PLUS 0x0008
#define FACE_Y_PLUS 0x0010
#define FACE_Z_PLUS 0x0020

#define UV_1_16 0.0625f
#define UV_GAP 0.0001f

enum NYCubeType {
	CUBE_AIR = 0,
	CUBE_STONE = 1,
	CUBE_GRASS = 2,
	CUBE_DIRT = 3,
	CUBE_COBBLESTONE = 4,
	CUBE_SAPLING = 5,
	CUBE_WOOD_PLANK = 6,
	CUBE_BEDROCK = 7,
	CUBE_FLOWING_WATER = 8,
	CUBE_STILL_WATER = 9,
	CUBE_FLOWING_LAVA = 10,
	CUBE_STILL_LAVA = 11,
	CUBE_SAND = 12,
	CUBE_GRAVEL = 13,
	CUBE_WOOD = 17,
	CUBE_LEAVES = 18
};

class NYCube {
private:
	uint16 _Infos;
	NYCubeType _Type;

public:
	static float _CubeUV[16 * 16 * 2 * 4 * 6]; ///< Buffer pour positions d'uv

	static void initUV () {//south (x+), north (x-), east (y+), west (y-), top (z+), bottom (z-)
		float xs[6], ys[6];
		for (int i = 0; i < 32; i++) {
			switch ((NYCubeType) i) {
			case CUBE_BEDROCK:
				for (int i = 0; i < 6; i++) {
					xs[i] = 1.0f; ys[i] = 1.0f;
				}
				break;
			case CUBE_STONE:
				for (int i = 0; i < 6; i++) {
					xs[i] = 1.0f; ys[i] = 0.0f;
				}
				break;
			case CUBE_GRASS:
				for (int i = 0; i < 4; i++) {
					xs[i] = 3.0f; ys[i] = 0.0f;
				}
				xs[4] = 0.0f; ys[4] = 0.0f;
				xs[5] = 2.0f; ys[5] = 0.0f;
				break;
			case CUBE_DIRT:
				for (int i = 0; i < 6; i++) {
					xs[i] = 2.0f; ys[i] = 0.0f;
				}
				break;
			case CUBE_COBBLESTONE:
				for (int i = 0; i < 6; i++) {
					xs[i] = 0.0f; ys[i] = 1.0f;
				}
				break;
			case CUBE_STILL_WATER:
				for (int i = 0; i < 6; i++) {
					xs[i] = 13.0f; ys[i] = 12.0f;
				}
				break;
			}
			for (int i = 0; i < 6; i++) {
				xs[i] *= UV_1_16; ys[i] *= UV_1_16;
			}

			for (int face = 0; face < 6; face++) {
				_CubeUV[i * 48 + face * 8] = xs[face] + UV_1_16 - UV_GAP;
				_CubeUV[i * 48 + face * 8 + 1] = ys[face] + UV_1_16 - UV_GAP;

				_CubeUV[i * 48 + face * 8 + 2] = xs[face] + UV_1_16 - UV_GAP;
				_CubeUV[i * 48 + face * 8 + 3] = ys[face] + UV_GAP;

				_CubeUV[i * 48 + face * 8 + 4] = xs[face] + UV_GAP;
				_CubeUV[i * 48 + face * 8 + 5] = ys[face] + UV_GAP;

				_CubeUV[i * 48 + face * 8 + 6] = xs[face] + UV_GAP;
				_CubeUV[i * 48 + face * 8 + 7] = ys[face] + UV_1_16 - UV_GAP;
			}
		}
	}

	//bool _Draw;
	static const int CUBE_SIZE = 10;

	NYCube () {
		//_Draw = true;
		setType (CUBE_AIR);
	}

	void setType (NYCubeType type) {
		_Type = type;
		_Infos = 0x0000;
		switch (type) {
		case CUBE_DIRT:
		case CUBE_GRASS:
		case CUBE_STONE:
		case CUBE_WOOD:
			_Infos = SOLID_MASK | BREAKABLE_MASK;
			break;
		case CUBE_LEAVES:
			_Infos = TRANSPARENCY_MASK | SOLID_MASK | BREAKABLE_MASK;
			break;
		case CUBE_AIR:
		case CUBE_STILL_WATER:
		case CUBE_FLOWING_WATER:
			_Infos = TRANSPARENCY_MASK;
			break;
		case CUBE_BEDROCK:
			_Infos = SOLID_MASK | FACES_MASK;
			break;
		default:
			break;
		}
		if (type != CUBE_AIR) _Infos |= DIRTY_MASK;
	}

	/*
	return value :
		number of frames until the next update
		-1 if no more update is needed
	*/
	int update (int x, int y, int z) {
		if (isSolid ()) return -1;

		return -1;
	}

	NYCubeType getType (void) {
		return _Type;
	}

	bool isSolid (void) {
		return (_Infos & SOLID_MASK);
	}

	bool isDirty (void) {
		return (_Infos & DIRTY_MASK);
	}

	bool isTransparent (void) {
		return (_Infos & TRANSPARENCY_MASK);
	}

	bool isBreakable (void) {
		return (_Infos & BREAKABLE_MASK);
	}

	void setFace (uint16 side) {
		_Infos = (_Infos & ~FACES_MASK) | (side & FACES_MASK);
	}

	void hideFace (uint16 side) {
		_Infos &= ((~side) & FACES_MASK);
	}

	void showFace (uint16 side) {
		_Infos |= (side & FACES_MASK);
	}

	bool showFace (NYCube * other) {
		if (!other) return true;
		if (other->isTransparent ()) {
			return (_Type != other->_Type);
		}
		return false;
	}

	bool checkFace (uint16 side) {
		return (_Infos & side & FACES_MASK);
	}

	void saveToFile (FILE * fs) {
		//fputc (_Draw ? 1 : 0, fs);
		fputc (_Type, fs);
	}

	void loadFromFile (FILE * fe) {
		//_Draw = fgetc (fe) ? true : false;
		_Type = (NYCubeType) fgetc (fe);
	}
};