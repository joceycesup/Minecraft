#include "engine/utils/types.h"
#include "engine/utils/ny_utils.h"
#include "engine/utils/types_3d.h"

#include "gl/glew.h"
#include "gl/freeglut.h"
#include "engine/render/graph/tex_manager.h"

class NYFbo {
public:
	GLuint * _ColorTex;
	int _NbColorTex;
	GLuint _DepthTex;
	GLuint _FBO;
	int _Width;
	int _Height;

	NYFbo (int nbOutTex) {
		_ColorTex = new GLuint[nbOutTex];
		memset (_ColorTex, 0x00, sizeof (GLuint) *nbOutTex);
		_NbColorTex = nbOutTex;
		_DepthTex = 0;
		_FBO = 0;
	}

	~NYFbo () {
		SAFEDELETE_TAB (_ColorTex);
	}

	void init (int width, int height) {
		_Width = width;
		_Height = height;
		createColorTexs (width, height);
		createFBO ();
	}

	void setColorAsShaderInput (int numCol = 0, int location = GL_TEXTURE0, char * texSamplerName = "colorTex1") {
		GLint prog;
		glGetIntegerv (GL_CURRENT_PROGRAM, &prog);

		GLuint texLoc = glGetUniformLocation (prog, texSamplerName);
		checkGlError ("glGetUniformLocation(prog, texSamplerName);");
		glUniform1i (texLoc, location - GL_TEXTURE0);
		checkGlError ("glUniform1i(texLoc, location- GL_TEXTURE0);");

		glActiveTexture (location);
		glBindTexture (GL_TEXTURE_2D, _ColorTex[numCol]);

		//reset
		glActiveTexture (GL_TEXTURE0);
	}

	void setDepthAsShaderInput (int location, char * texSamplerName) {
		GLint prog;
		glGetIntegerv (GL_CURRENT_PROGRAM, &prog);

		GLuint texLoc = glGetUniformLocation (prog, texSamplerName);
		glUniform1i (texLoc, location - GL_TEXTURE0);
		checkGlError ("glUniform1i(texLoc, location);");

		glActiveTexture (location);
		glBindTexture (GL_TEXTURE_2D, _DepthTex);
	}/*

	void loadFromFile (const char * name, int numCol) {
		NYTexManager * texMan = NYTexManager::getInstance ();

		png_image image;
		png_bytep buffer;
		uint32 i;
		memset (&image, 0x00, sizeof (png_image));

		if (texMan->loadImageFile_PNG (std::string (name), &image, &buffer))
			return;

		if (_Width != image.width || _Height != image.height)
			Log::log (Log::USER_ERROR, "Loading tex in FBO with bad height or width");

		uint32 nbPixels = _Width * _Height;
		uint8 * pixelsRgb = new uint8[nbPixels * 3];

		if (!pixelsRgb)
			return;

		//Ajoute la composante alpha
		for (int x = 0; x < min (image.width, _Width); x++) {
			for (int y = 0; y < min (image.height, _Height); y++) {
				int pix1 = (x + y*_Width);
				int pix2 = (x + (image.height - y - 1)*image.width);
				pixelsRgb[(3 * pix1) + 0] = buffer[(4 * pix2) + 0];
				pixelsRgb[(3 * pix1) + 1] = buffer[(4 * pix2) + 1];
				pixelsRgb[(3 * pix1) + 2] = buffer[(4 * pix2) + 2];
			}
		}

		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, _ColorTex[numCol]);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, _Width, _Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelsRgb);

		glBindTexture (GL_TEXTURE_2D, 0);

		SAFEDELETE_TAB (pixelsRgb);
	}//*/

	void readFb (int numCol, uint8 * buff, uint32 bufSize) {
		glReadPixels ((GLint) 0, (GLint) 0,
			(GLint) _Width, (GLint) _Height,
			GL_RGB, GL_UNSIGNED_BYTE, buff);
	}

	void readFbTex (int numCol, uint8 * buff, uint32 bufSize) {
		int width, height;

		if (!buff) {
			Log::log (Log::ENGINE_ERROR, "Fbo read fail, buf is null");
			return;
		}

		uint32 nbPixels = _Width * _Height;
		if (nbPixels * 3 != bufSize) {
			Log::log (Log::ENGINE_ERROR, "Fbo read fail buffer has bad size");
			return;
		}

		glActiveTexture (GL_TEXTURE0);

		glBindTexture (GL_TEXTURE_2D, _ColorTex[numCol]);
		glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

		if (width != _Width || height != _Height) {
			Log::log (Log::ENGINE_ERROR, "Fbo tex has not buff size, cannot read it");
			return;
		}

		glBindBuffer (GL_PIXEL_PACK_BUFFER, 0);
		glPixelStorei (GL_PACK_ALIGNMENT, 1); //par defaut word aligned et padding deborde ?	

											  /*int val;
											  glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_INTERNAL_FORMAT, &val);
											  glGetIntegerv(GL_PACK_ALIGNMENT, &val);*/

		glGetTexImage (GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buff);
		checkGlError ("glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buff);");

		glBindTexture (GL_TEXTURE_2D, 0);
	}/*

	void saveToFile (const char * name, int numCol, bool useCurrentFb = false) {
		uint32 nbPixels = _Width * _Height;
		uint8 * pixelsRgb = (uint8*) malloc (nbPixels * 3);

		if (!pixelsRgb) {
			Log::log (Log::ENGINE_ERROR, "Fbo read buf alloc failed");
			return;
		}

		if (useCurrentFb)
			readFb (numCol, pixelsRgb, nbPixels * 3);
		else
			readFbTex (numCol, pixelsRgb, nbPixels * 3);

		NYTexManager::writeImage (name, _Width, _Height, pixelsRgb, "fbo save", true);

		free (pixelsRgb);
	}//*/

private:
	void createColorTexs (int width, int height) {
		if (_ColorTex[0] != 0)
			glDeleteTextures (_NbColorTex, _ColorTex);
		glGenTextures (_NbColorTex, _ColorTex);

		for (int i = 0; i < _NbColorTex; i++) {
			glActiveTexture (GL_TEXTURE0);
			glBindTexture (GL_TEXTURE_2D, _ColorTex[i]);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f); //no aniso filtering
			glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		}
	}

	void createDepthTex (int width, int height) {
		if (_DepthTex != 0)
			glDeleteTextures (1, &_DepthTex);
		glGenTextures (1, &_DepthTex);
		glBindTexture (GL_TEXTURE_2D, _DepthTex);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	//// !! Creer les tex avnt de creer les FBO
	void createFBO () {
		if (_FBO != 0)
			glDeleteFramebuffers (1, &_FBO);

		glGenFramebuffers (1, &_FBO);

		//On bind le FBO pour tester si tout est ok
		glBindFramebuffer (GL_FRAMEBUFFER, _FBO);

		//Attach 2D texture to this FBO
		for (int i = 0; i < _NbColorTex; i++)
			glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _ColorTex[i], 0);

		//Attach depth texture to FBO
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _DepthTex, 0);

		//Does the GPU support current FBO configuration?
		GLenum status;
		status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
		switch (status) {
		case GL_FRAMEBUFFER_COMPLETE:
			//_cprintf("GPU ok for VBO with depth and color\n");
			break;
		default:
			Log::log (Log::ENGINE_ERROR, "GPU does not support VBO");
			Log::log (Log::USER_ERROR, "You graphic card is not able to run this software (no VBO support)");
			return;
		}

		//On debind
		glBindFramebuffer (GL_FRAMEBUFFER, 0);
	}

	static void checkGlError (const char * call) {
		GLenum error = glGetError ();

		if (error != 0) {
			switch (error) {
			case GL_INVALID_ENUM: Log::log (Log::ENGINE_ERROR, ("Opengl error (GL_INVALID_ENUM) for call " + toString (call)).c_str ()); break;
			case GL_INVALID_OPERATION: Log::log (Log::ENGINE_ERROR, ("Opengl error (GL_INVALID_OPERATION) for call " + toString (call)).c_str ()); break;
			case GL_STACK_OVERFLOW: Log::log (Log::ENGINE_ERROR, ("Opengl error (GL_STACK_OVERFLOW) for call " + toString (call)).c_str ()); break;
			case GL_STACK_UNDERFLOW: Log::log (Log::ENGINE_ERROR, ("Opengl error (GL_STACK_UNDERFLOW) for call " + toString (call)).c_str ()); break;
			case GL_OUT_OF_MEMORY: Log::log (Log::ENGINE_ERROR, ("Opengl error (GL_OUT_OF_MEMORY) for call " + toString (call)).c_str ()); break;
			case GL_TABLE_TOO_LARGE: Log::log (Log::ENGINE_ERROR, ("Opengl error (GL_TABLE_TOO_LARGE) for call " + toString (call)).c_str ()); break;
			default: Log::log (Log::ENGINE_ERROR, ("Unknown Opengl error for call " + toString (call)).c_str ()); break;
			}
		}
	}
};

//Utilisation
void setOutFBO (NYFbo * fbo = NULL) {

	if (fbo) {
		//On passe en FBO pour pouvoir faire nos effets
		glBindFramebuffer (GL_FRAMEBUFFER, fbo->_FBO);
		checkGlError ("glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_FBO);");

		//Attach 2D texture to this FBO
		for (int i = 0; i < fbo->_NbColorTex; i++)
			glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, fbo->_ColorTex[i], 0);

		//Attach depth texture to FBO
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo->_DepthTex, 0);
	} else {
		//On passe en mode rendu normal
		glBindFramebuffer (GL_FRAMEBUFFER, 0);
	}
}
