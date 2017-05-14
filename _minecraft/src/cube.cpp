//*
#include "cube.h"

float NYCube::_CubeUV[16 * 16 * 2 * 4 * 6];//*/
/*
void NYCube::initUV () {
	for (int i = 0; i < 256; i++) {
		for (int face = 0; face < 6; face++) {
			_CubeUV[i * 48 + face * 8] = 0.0f;
			_CubeUV[i * 48 + face * 8 + 1] = 0.0f;

			_CubeUV[i * 48 + face * 8 + 2] = 0.0f;
			_CubeUV[i * 48 + face * 8 + 3] = 0.0625f;

			_CubeUV[i * 48 + face * 8 + 4] = 0.0625f;
			_CubeUV[i * 48 + face * 8 + 5] = 0.0625f;

			_CubeUV[i * 48 + face * 8 + 6] = 0.0625f;
			_CubeUV[i * 48 + face * 8 + 7] = 0.0f;
		}
	}
}//*/
