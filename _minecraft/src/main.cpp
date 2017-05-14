//Includes application
#include "main.h"
#include <conio.h>
#include <vector>
#include <string>
#include <windows.h>

#include "external/gl/glew.h"
#include "external/gl/freeglut.h"

//Moteur
#include "engine/utils/types_3d.h"
#include "engine/timer.h"
#include "engine/log/log_console.h"
#include "engine/render/renderer.h"
#include "engine/gui/screen.h"
#include "engine/gui/screen_manager.h"

#include "world.h"
#include "cube.h"
#include "avatar.h"
#include "input_handler.h"

#define INTER_FRAME_DELAY 0.01666666666666666666666666666667 // 60th of a second 

NYWorld * g_world = NULL;
NYRenderer * g_renderer = NULL;
GLuint g_program;
NYTimer * g_timer = NULL;
int g_nb_frames = 0;
float g_elapsed_fps = 0;
int g_main_window_id;
int g_mouse_btn_gui_state = 0;
bool g_fullscreen = false;

NYColor skyColor;
float skyColorFactorShader = 0.0f;
float dayTime = 0.0f;
bool cursorHandled = true;
bool warpCalled = true;
NYVert3Df sunPos = NYVert3Df (0, 0, 0);
NYAvatar * avatar = NULL;

//GUI 
GUIScreenManager * g_screen_manager = NULL;
GUIBouton * BtnParams = NULL;
GUIBouton * BtnClose = NULL;
GUILabel * LabelFps = NULL;
GUILabel * LabelTime = NULL;
GUILabel * LabelDebug = NULL;
GUILabel * LabelCam = NULL;
GUIScreen * g_screen_params = NULL;
GUIScreen * g_screen_jeu = NULL;
GUISlider * g_slider;

void printDebug (string s) {
	LabelDebug->Text = s;
}

//////////////////////////////////////////////////////////////////////////
// GESTION APPLICATION
//////////////////////////////////////////////////////////////////////////
void update (void) {
	float elapsed = g_timer->getElapsedSeconds (true);
	if (elapsed > INTER_FRAME_DELAY)
		elapsed = INTER_FRAME_DELAY;

	InputHandler::update (elapsed);
	GetAsyncKeyState (VK_CONTROL);//reset Async state

	avatar->update (elapsed);
	g_world->update (elapsed);

	static float g_eval_elapsed = 0;

	//Calcul des fps
	g_elapsed_fps += elapsed;
	g_nb_frames++;
	if (g_elapsed_fps > 1.0) {
		LabelFps->Text = std::string ("FPS : ") + toString (g_nb_frames);
		g_elapsed_fps -= 1.0f;
		g_nb_frames = 0;
	}

	//Rendu
	g_renderer->render (elapsed);

	if (cursorHandled) {
		cursorHandled = false;
		int centerX = glutGet (GLUT_WINDOW_WIDTH) / 2;
		int centerY = glutGet (GLUT_WINDOW_HEIGHT) / 2;
		warpCalled = true;
		glutWarpPointer (centerX, centerY);
	}
}


void render2d (void) {
	g_screen_manager->render ();
}

void renderObjects (void) {
	//Rendu des axes
	glDisable (GL_LIGHTING);

	glBegin (GL_LINES);
	glColor3d (1, 0, 0);
	glVertex3d (0, 0, 0);
	glVertex3d (10000, 0, 0);
	glColor3d (0, 1, 0);
	glVertex3d (0, 0, 0);
	glVertex3d (0, 10000, 0);
	glColor3d (0, 0, 1);
	glVertex3d (0, 0, 0);
	glVertex3d (0, 0, 10000);
	glEnd ();

	glEnable (GL_LIGHTING);

	glUseProgram (g_program);

	GLuint elap = glGetUniformLocation (g_program, "elapsed");
	glUniform1f (elap, NYRenderer::_DeltaTimeCumul);

	GLuint viewdir = glGetUniformLocation (g_program, "pos");
	glUniform3f (viewdir, g_renderer->_Camera->_Position.X, g_renderer->_Camera->_Position.Y, g_renderer->_Camera->_Position.Z);

	GLuint amb = glGetUniformLocation (g_program, "ambientLevel");
	glUniform1f (amb, 0.4);
	//*
	NYFloatMatrix iv;
	iv.createViewMatrix (g_renderer->_Camera->_Position, g_renderer->_Camera->_LookAt, g_renderer->_Camera->_UpVec);
	iv.invert ();//*/
	GLuint invView = glGetUniformLocation (g_program, "invertView");
	glUniformMatrix4fv (invView, 1, true, iv.Mat.t);

	GLuint view = glGetUniformLocation (g_program, "view");
	glUniformMatrix4fv (view, 1, true, g_renderer->_Camera->_ViewMatrix.Mat.t);

	GLuint uw = glGetUniformLocation (g_program, "underWater");
	NYCubeType type = g_world->getCube (g_renderer->_Camera->_Position / NYCube::CUBE_SIZE)->getType ();
	bool underwater = (type == NYCubeType::CUBE_FLOWING_WATER || type == NYCubeType::CUBE_STILL_WATER);
	glUniform1f (uw, underwater ? 1.0 : 0.0);

	GLuint sc = glGetUniformLocation (g_program, "skyColor");
	glUniform3f (sc, skyColor.R, skyColor.V, skyColor.B);

	GLuint scf = glGetUniformLocation (g_program, "skyColorFactor");
	glUniform1f (scf, skyColorFactorShader);

	glPushMatrix ();
	//g_world->render_world_old_school ();
	g_world->render_world_vbo (g_program, underwater);
	glPopMatrix ();
	glPushMatrix ();

	glTranslated (avatar->Position.X, avatar->Position.Y, avatar->Position.Z);
	glDisable (GL_LIGHTING);
	glEnable (GL_BLEND);
	glColor3d (1, 1, 0);
	glutSolidCube (3);
	glDisable (GL_BLEND);
	glEnable (GL_LIGHTING);
	glPopMatrix ();
}

void setLights (void) {
	dayTime += NYRenderer::_DeltaTime;
	if (dayTime >= 24.0f)
		dayTime -= 24.0f;
	float dayNightFactor = dayTime / 24.0f;
	double sunDist = 25.0;
	sunPos = NYVert3Df (cos (dayNightFactor*2.0*M_PI)*sunDist, sin (dayNightFactor*2.0*M_PI)*sunDist, 0.0f);
	sunPos.Z = sunPos.X*-0.33f;

	NYColor night = NYColor (0.025f, 0.1f, 0.165f, 1.0f);
	NYColor colors[] = {
		night,
		night,
		NYColor (0.827f, 0.475f, 0.322f, 1.0f),//dawn
		NYColor (0.827f, 0.475f, 0.322f, 1.0f),//dawn
		NYColor (0.447f, 0.8f, 0.898f, 1.0f), //day
		NYColor (0.133f, 0.729f, 0.91f, 1.0f),//twilight
		night
	};
	float skyColorFactorsShader[] = {0.8f, 0.8f, 0.5f, 0.4f, 0.0f, 0.4f, 0.8f};
	float positions[] = {0.0f, 0.21f, 0.29f, 0.3f, 0.36f, 0.86f, 1.00005f};
	int i = -1;
	while (dayNightFactor > positions[i + 1]) i++;

	//LabelDebug->Text = toString (((float) ((int) (dayTime*10.0f))) / 10.0f);
	{
		int hour = (int)(dayNightFactor*24.0f);
		int min = ((int)(dayNightFactor*1440.0f)) % 60;
		LabelTime->Text = (hour < 10 ? "0" : "") + toString (hour) + (min < 10 ? ":0" : ":") + toString (((int)(dayTime*60.0f)) % 60);
	}
	float colorFactor = (dayNightFactor - positions[i]) / (positions[i + 1] - positions[i]);
	skyColorFactorShader = colorFactor*skyColorFactorsShader[i + 1] + (1.0f - colorFactor)*skyColorFactorsShader[i];

	skyColor = NYColor (
		(colorFactor*colors[i + 1].R + (1.0f - colorFactor)*colors[i].R),
		(colorFactor*colors[i + 1].V + (1.0f - colorFactor)*colors[i].V),
		(colorFactor*colors[i + 1].B + (1.0f - colorFactor)*colors[i].B), 1.0f);
	g_renderer->setBackgroundColor (skyColor);

	float sunLuminosity = 1.0f;
	float changeDuration = 0.2f;
	float changeHalfDuration = changeDuration / 2.0f;

	if (dayNightFactor <= 0.25f - changeHalfDuration) {
		sunLuminosity = 0.0f;
	}
	else if (dayNightFactor <= 0.25f + changeHalfDuration) {
		float factor = (dayNightFactor - 0.25f + changeHalfDuration) / changeDuration;
		sunLuminosity = factor*factor*(3.0f - 2.0f*factor);
	}
	else if (dayNightFactor <= 0.75f - changeHalfDuration) {
		//sunLuminosity = 1.0f;
	}
	else if (dayNightFactor <= 0.75f + changeHalfDuration) {
		float factor = 1.0f - (dayNightFactor - 0.75f + changeHalfDuration) / changeDuration;
		sunLuminosity = factor*factor*(3.0f - 2.0f*factor);
	}
	else {
		sunLuminosity = 0.0f;
	}
	float moonLuminosity = 1.0f - sunLuminosity;
	//LabelDebug->Text = toString (((float) ((int) (sunLuminosity*10.0f))) / 10.0f);

	//On active l'illumination
	glDisable (GL_TEXTURE_2D);
	glEnable (GL_LIGHTING);
	{
		//On active la light 0
		glEnable (GL_LIGHT0);

		//On définit une lumière
		float position[4] = {sunPos.X, sunPos.Y, sunPos.Z, 0}; // w = 1 donc c'est une point light (w=0 -> directionelle, point à l'infini)
		glLightfv (GL_LIGHT0, GL_POSITION, position);
		float diffuse[4] = {0.5f*sunLuminosity, 0.5f*sunLuminosity, 0.5f*sunLuminosity};
		glLightfv (GL_LIGHT0, GL_DIFFUSE, diffuse);
		float specular[4] = {0.1f*sunLuminosity, 0.1f*sunLuminosity, 0.1f*sunLuminosity};
		glLightfv (GL_LIGHT0, GL_SPECULAR, specular);
		float ambient[4] = {0.3f*sunLuminosity, 0.3f*sunLuminosity, 0.3f*sunLuminosity};
		glLightfv (GL_LIGHT0, GL_AMBIENT, ambient);
	}
	{
		//On active la light 1
		glEnable (GL_LIGHT1);

		//On définit une lumière
		float position[4] = {-sunPos.X, -sunPos.Y, -sunPos.Z, 0}; // w = 1 donc c'est une point light (w=0 -> directionelle, point à l'infini)
		glLightfv (GL_LIGHT1, GL_POSITION, position);
		float diffuse[4] = {0.1f*moonLuminosity, 0.1f*moonLuminosity, 0.5f*moonLuminosity};
		glLightfv (GL_LIGHT1, GL_DIFFUSE, diffuse);
		float specular[4] = {0.04f*moonLuminosity, 0.04f*moonLuminosity, 0.1f*moonLuminosity};
		glLightfv (GL_LIGHT1, GL_SPECULAR, specular);
		float ambient[4] = {0.05f*moonLuminosity, 0.1f*moonLuminosity, 0.3f*moonLuminosity};
		glLightfv (GL_LIGHT1, GL_AMBIENT, ambient);
	}
	//Diffuse
	GLfloat diffuseValue = 0.7f;
	GLfloat sunMaterialDiffuse[] = {diffuseValue, diffuseValue / 2.0f, 0.0f, 1.0f};
	GLfloat moonMaterialDiffuse[] = {diffuseValue, diffuseValue, diffuseValue, 1.0f};

	//Speculaire
	GLfloat whiteSpecularMaterial[] = {0.3, 0.3, 0.3, 1.0};
	glMaterialfv (GL_FRONT, GL_SPECULAR, whiteSpecularMaterial);
	GLfloat mShininess = 100;
	glMaterialf (GL_FRONT, GL_SHININESS, mShininess);

	//Ambient
	GLfloat ambientValue = 0.2f;
	GLfloat sunMaterialAmbient[] = {ambientValue, ambientValue / 2.0f, 0.0f, 1.0f};
	GLfloat moonMaterialAmbient[] = {ambientValue, ambientValue, ambientValue, 1.0f};

	glPushMatrix ();
	glTranslated (g_renderer->_Camera->_Position.X, g_renderer->_Camera->_Position.Y, g_renderer->_Camera->_Position.Z);
	//Emissive
	GLfloat emissiveSun[] = {1.0f, 0.5f, 0.0f, 1.0f};
	GLfloat emissiveMoon[] = {0.8f, 0.8f, 0.8f, 1.0f};
	glPushMatrix ();
	glTranslated (sunPos.X, sunPos.Y, sunPos.Z);
	glRotated (dayNightFactor*360.0, 0, 0, 1);
	glMaterialfv (GL_FRONT, GL_EMISSION, emissiveSun);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, sunMaterialDiffuse);
	glMaterialfv (GL_FRONT, GL_AMBIENT, sunMaterialAmbient);
	glutSolidCube (2.0);
	glPopMatrix ();
	glTranslated (-sunPos.X, -sunPos.Y, -sunPos.Z);
	glRotated (dayNightFactor*360.0, 0, 0, 1);
	glMaterialfv (GL_FRONT, GL_EMISSION, emissiveMoon);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, moonMaterialDiffuse);
	glMaterialfv (GL_FRONT, GL_AMBIENT, moonMaterialAmbient);
	glutSolidCube (1.4);
	glPopMatrix ();
}

void resizeFunction (int width, int height) {
	glViewport (0, 0, width, height);
	g_renderer->resize (width, height);
}

//////////////////////////////////////////////////////////////////////////
// GESTION CLAVIER SOURIS
//////////////////////////////////////////////////////////////////////////

void specialDownFunction (int key, int p1, int p2) {
	//On change de mode de camera
	if (key == GLUT_KEY_LEFT) {
	}
}

void specialUpFunction (int key, int p1, int p2) {
}

void keyboardDownFunction (unsigned char key, int p1, int p2) {
	if (key == VK_ESCAPE) {
		glutDestroyWindow (g_main_window_id);
		exit (0);
	}

	if (key == 'f') {
		if (g_fullscreen) {
			glutLeaveGameMode ();
			glutLeaveFullScreen ();
			glutReshapeWindow (g_renderer->_ScreenWidth, g_renderer->_ScreenWidth);
			glutPositionWindow (0, 0);
		}
		else {
			glutFullScreen ();
		}
		g_fullscreen = !g_fullscreen;
	}
	if (key == 'm') {
		g_program = g_renderer->createProgram ("shaders/psbase.glsl", "shaders/vsbase.glsl");
	}
}

void keyboardUpFunction (unsigned char key, int p1, int p2) {
}

void mouseWheelFunction (int wheel, int dir, int x, int y) {
	//g_renderer->_Camera->move (NYVert3Df (0, 0, dir*5.0f));
}

void mouseMoveFunction (int x, int y, bool pressed) {
	if (warpCalled) {
		warpCalled = false;
		return;
	}
	int centerX = glutGet (GLUT_WINDOW_WIDTH) / 2;
	int centerY = glutGet (GLUT_WINDOW_HEIGHT) / 2;

	int dx = x - centerX;
	int dy = y - centerY;

	/*
	g_renderer->_Camera->rotate ((float) -dx / 300.0f);
	g_renderer->_Camera->rotateUp ((float) -dy / 300.0f);/*/
	InputHandler::SetMouseDeltaPos (dx, -dy);
	//*/
	cursorHandled = true;
}

void mouseFunction (int button, int state, int x, int y) {
	//Gestion de la roulette de la souris
	if ((button & 0x07) == 3 && state)
		mouseWheelFunction (button, 1, x, y);
	if ((button & 0x07) == 4 && state)
		mouseWheelFunction (button, -1, x, y);

	/*
	//GUI
	g_mouse_btn_gui_state = 0;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		g_mouse_btn_gui_state |= GUI_MLBUTTON;

	bool mouseTraite = false;
	mouseTraite = g_screen_manager->mouseCallback (x, y, g_mouse_btn_gui_state, 0, 0);/*/

	//Gestion clic souris
	g_mouse_btn_gui_state = 0;
	if (button == GLUT_LEFT_BUTTON) {
		InputHandler::SetMouseButtonClicked (state == GLUT_DOWN);
	}
	//*/
}

void mouseMoveActiveFunction (int x, int y) {
	mouseMoveFunction (x, y, true);
}
void mouseMovePassiveFunction (int x, int y) {
	mouseMoveFunction (x, y, false);
}


void clickBtnParams (GUIBouton * bouton) {
	g_screen_manager->setActiveScreen (g_screen_params);
}

void clickBtnCloseParam (GUIBouton * bouton) {
	g_screen_manager->setActiveScreen (g_screen_jeu);
}

/**
  * POINT D'ENTREE PRINCIPAL
  **/
int main (int argc, char* argv[]) {
	LogConsole::createInstance ();

	int screen_width = 800;
	int screen_height = 600;

	glutInit (&argc, argv);
	glutInitContextVersion (3, 0);
	glutSetOption (
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
	);

	glutInitWindowSize (screen_width, screen_height);
	glutInitWindowPosition (0, 0);
	glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glEnable (GL_MULTISAMPLE);

	Log::log (Log::ENGINE_INFO, (toString (argc) + " arguments en ligne de commande.").c_str ());
	bool gameMode = true;
	for (int i = 0; i < argc; i++) {
		if (argv[i][0] == 'w') {
			Log::log (Log::ENGINE_INFO, "Arg w mode fenetre.\n");
			gameMode = false;
		}
	}

	if (gameMode) {
		int width = glutGet (GLUT_SCREEN_WIDTH);
		int height = glutGet (GLUT_SCREEN_HEIGHT);

		char gameModeStr[200];
		sprintf (gameModeStr, "%dx%d:32@60", width, height);
		glutGameModeString (gameModeStr);
		g_main_window_id = glutEnterGameMode ();
	}
	else {
		g_main_window_id = glutCreateWindow ("MyNecraft");
		glutReshapeWindow (screen_width, screen_height);
	}

	if (g_main_window_id < 1) {
		Log::log (Log::ENGINE_ERROR, "Erreur creation de la fenetre.");
		exit (EXIT_FAILURE);
	}

	GLenum glewInitResult = glewInit ();

	if (glewInitResult != GLEW_OK) {
		Log::log (Log::ENGINE_ERROR, ("Erreur init glew " + std::string ((char*)glewGetErrorString (glewInitResult))).c_str ());
		_cprintf ("ERROR : %s", glewGetErrorString (glewInitResult));
		exit (EXIT_FAILURE);
	}

	//Affichage des capacités du système
	Log::log (Log::ENGINE_INFO, ("OpenGL Version : " + std::string ((char*)glGetString (GL_VERSION))).c_str ());

	glutDisplayFunc (update);
	glutReshapeFunc (resizeFunction);
	glutKeyboardFunc (keyboardDownFunction);
	glutKeyboardUpFunc (keyboardUpFunction);
	glutSpecialFunc (specialDownFunction);
	glutSpecialUpFunc (specialUpFunction);
	glutMouseFunc (mouseFunction);
	glutMotionFunc (mouseMoveActiveFunction);
	glutPassiveMotionFunc (mouseMovePassiveFunction);
	glutIgnoreKeyRepeat (1);

	//Initialisation du renderer
	g_renderer = NYRenderer::getInstance ();
	g_renderer->setRenderObjectFun (renderObjects);
	g_renderer->setRender2DFun (render2d);
	g_renderer->setLightsFun (setLights);
	g_renderer->setBackgroundColor (NYColor ());
	g_renderer->initialise (true);

	//Creation d'un programme de shader, avec vertex et fragment shaders
	g_program = g_renderer->createProgram ("shaders/psbase.glsl", "shaders/vsbase.glsl");

	//On applique la config du renderer
	glViewport (0, 0, g_renderer->_ScreenWidth, g_renderer->_ScreenHeight);
	g_renderer->resize (g_renderer->_ScreenWidth, g_renderer->_ScreenHeight);

	//Ecran de jeu
	uint16 x = 10;
	uint16 y = 10;
	g_screen_jeu = new GUIScreen ();

	g_screen_manager = new GUIScreenManager ();

	//Bouton pour afficher les params
	BtnParams = new GUIBouton ();
	BtnParams->Titre = std::string ("Params");
	BtnParams->X = x;
	BtnParams->setOnClick (clickBtnParams);
	g_screen_jeu->addElement (BtnParams);

	y += BtnParams->Height + 1;

	LabelFps = new GUILabel ();
	LabelFps->Text = "FPS";
	LabelFps->X = x;
	LabelFps->Y = y;
	LabelFps->Visible = true;
	g_screen_jeu->addElement (LabelFps);

	LabelTime = new GUILabel ();
	LabelTime->Text = "Time";
	LabelTime->X = x;
	LabelTime->Y = y + 20;
	LabelTime->Visible = true;
	g_screen_jeu->addElement (LabelTime);

	LabelDebug = new GUILabel ();
	LabelDebug->Text = "Debug";
	LabelDebug->X = x;
	LabelDebug->Y = y + 40;
	LabelDebug->Visible = true;
	g_screen_jeu->addElement (LabelDebug);

	//Ecran de parametrage
	x = 10;
	y = 10;
	g_screen_params = new GUIScreen ();

	GUIBouton * btnClose = new GUIBouton ();
	btnClose->Titre = std::string ("Close");
	btnClose->X = x;
	btnClose->setOnClick (clickBtnCloseParam);
	g_screen_params->addElement (btnClose);

	y += btnClose->Height + 1;
	y += 10;
	x += 10;

	GUILabel * label = new GUILabel ();
	label->X = x;
	label->Y = y;
	label->Text = "Param :";
	g_screen_params->addElement (label);

	y += label->Height + 1;

	g_slider = new GUISlider ();
	g_slider->setPos (x, y);
	g_slider->setMaxMin (1, 0);
	g_slider->Visible = true;
	g_screen_params->addElement (g_slider);

	y += g_slider->Height + 1;
	y += 10;

	//Ecran a rendre
	g_screen_manager->setActiveScreen (g_screen_jeu);

	//Init Camera
	g_renderer->_Camera->setPosition (NYVert3Df (10, 10, 10));
	g_renderer->_Camera->setLookAt (NYVert3Df (0, 0, 0));

	glutSetCursor (GLUT_CURSOR_NONE);

	//Fin init moteur

	//Init application

	NYCube::initUV ();
	InputHandler::init ();

	g_world = new NYWorld ();
	g_world->_FacteurGeneration = 5;
	g_world->init_world ();

	avatar = new NYAvatar (g_renderer->_Camera, g_world);

	//Init Timer
	g_timer = new NYTimer ();

	//On start
	g_timer->start ();

	glutMainLoop ();

	return 0;
}

