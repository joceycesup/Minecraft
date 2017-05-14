#ifndef __AVATAR__
#define __AVATAR__

#include "engine/utils/types_3d.h"
#include "engine/render/camera.h"
#include "main.h"
#include "input_handler.h"
#include "world.h"

#define nysign(x) ((x<0)?(-1.0f):(1.0f))
//*
class NYAvatar {
private:
	NYVert3Df oldPosition;

public:
	NYVert3Df Position;
	NYVert3Df Speed;

	NYTimer _TimerGrounded;

	bool physicsEnabled = false;

	float walkSpeed;
	float sprintSpeed;
	float sneakSpeed;
	float swimSpeed;

	NYVert3Df MoveDir;
	bool Move;
	bool Jump;
	float Height;
	float Width;
	bool Grounded;
	bool Swimming;
	//float jumpZ = 0.0f;
	//float jumpHeight = 0.0f;

	NYCamera * Cam;
	NYWorld * World;

	NYAvatar (NYCamera * cam, NYWorld * world) {
		float pos = MAT_SIZE_CUBES*NYCube::CUBE_SIZE / 2.0f;
		Position = NYVert3Df (pos, pos, MAT_HEIGHT_CUBES*NYCube::CUBE_SIZE);
		oldPosition = Position;
		Height = NYCube::CUBE_SIZE;
		Width = 0.3f*NYCube::CUBE_SIZE;
		walkSpeed = 4.317f*NYCube::CUBE_SIZE;
		sprintSpeed = 5.612f*NYCube::CUBE_SIZE;
		sneakSpeed = 1.31f*NYCube::CUBE_SIZE;;
		swimSpeed = 5.0f*NYCube::CUBE_SIZE;
		Cam = cam;
		Grounded = false;
		Swimming = false;
		Jump = false;
		World = world;
	}

	void render (void) {
		glutSolidCube (Width / 2);
	}

	void update (float elapsed) {
		if (InputHandler::GetButtonDown (BUTTON_PHYSICS)) {
			physicsEnabled = !physicsEnabled;
		}
		//printDebug ("action : " + toString (InputHandler::GetButton (BUTTON_ACTION)));

		Cam->rotate (-InputHandler::GetAxis (AXIS_X_VIEW));
		Cam->rotateUp (InputHandler::GetAxis (AXIS_Y_VIEW));

		float forward = InputHandler::GetAxis (AXIS_Y_MOVE), right = InputHandler::GetAxis (AXIS_X_MOVE);

		MoveDir = NYVert3Df (0.0f, 0.0f, 1.0f).vecProd (Cam->_NormVec);
		//printDebug (toString (Position.X) + "/" + toString (Position.Y) + "/" + toString ((int) Position.Z));
		float speedZ = Speed.Z;
		Speed = MoveDir*forward + Cam->_NormVec*right;

		if (physicsEnabled) {
			oldPosition = Position;
			bool noHorizontalSpeed = (Speed.X == 0.0f && Speed.Y == 0.0f);
			// ----- Check Collisions -----
			if (_TimerGrounded.getElapsedSeconds () > 0.01f)
				Grounded = false;

			for (int pass = 0; pass < 2; pass++) {
				for (int i = 0; i < 6; i++) {
					float valueColMin = 0;
					NYAxis axis = World->getMinCol (Position, Speed, Width, Height, valueColMin, i < 3);
					//Log::log(Log::ENGINE_INFO,"check");
					if (axis != 0) {
						//valueColMin = nymax(nyabs(valueColMin), 0.0001f) * (valueColMin > 0 ? 1.0f : -1.0f);
						if (axis & NY_AXIS_X) {
							Position.X += valueColMin + 0.001*nysign (valueColMin);
							Speed.X = 0;
						}
						if (axis & NY_AXIS_Y) {
							Position.Y += valueColMin + 0.001*nysign (valueColMin);
							Speed.Y = 0;
						}
						if (axis & NY_AXIS_Z) {
							Speed.Z = 0;
							Position.Z += valueColMin + 0.001*nysign (valueColMin);
							Grounded = true;
							if (valueColMin > 0) {
								Jump = false;
							}
							_TimerGrounded.start ();
						}
					}
				}
				if (noHorizontalSpeed) {
					Position.X = oldPosition.X;
					Position.Y = oldPosition.Y;
				}
			}

			if ((Grounded || Swimming) && !Jump && InputHandler::GetButton (BUTTON_JUMP)) {
				Jump = true;
				if (Grounded) {
					Grounded = false;
					speedZ = 47.0f;
				}
			}
			// ----- Check Collisions -----
			if (Grounded) {
				Speed *= InputHandler::GetButton (BUTTON_SNEAK) ? sneakSpeed : walkSpeed;
				Speed.Z = 0.0f;
			} else {
				Speed *= InputHandler::GetButton (BUTTON_SNEAK) ? sneakSpeed : walkSpeed;//TO BE REMOVED
				Speed.Z = speedZ - 9.81*elapsed*NYCube::CUBE_SIZE;
			}
		} else {
			Speed.Z = 0.0f;
			if (InputHandler::GetButton (BUTTON_SNEAK) != InputHandler::GetButton (BUTTON_JUMP)) {
				Speed.Z = InputHandler::GetButton (BUTTON_SNEAK) ? -1.0f : 1.0f;
			}
			Speed *= walkSpeed;
		}

		Position += Speed*elapsed;
		if (Position.Z < NYCube::CUBE_SIZE)
			Position.Z = NYCube::CUBE_SIZE;

		Cam->moveTo (Position + NYVert3Df (0.0f, 0.0f, Height));
	}
};

#endif
/*/
class NYAvatar {
public:
	NYVert3Df Position;
	NYVert3Df Speed;

	bool Move;
	bool Jump;
	float Height;
	float CurrentHeight;
	float Width;
	bool avance;
	bool recule;
	bool gauche;
	bool droite;
	bool Standing;
	bool InWater;
	bool Crouch;
	bool Run;

	NYCamera * Cam;
	NYWorld * World;

	NYTimer _TimerStanding;

	NYAvatar (NYCamera * cam, NYWorld * world) {
		Position = NYVert3Df ((MAT_SIZE_CUBES*NYCube::CUBE_SIZE) / 2, (MAT_SIZE_CUBES*NYCube::CUBE_SIZE) / 2, (MAT_HEIGHT_CUBES*NYCube::CUBE_SIZE) * 2);
		Height = 15;
		CurrentHeight = Height;
		Width = 3;
		Cam = cam;
		avance = false;
		recule = false;
		gauche = false;
		droite = false;
		Standing = false;
		Jump = false;
		World = world;
		InWater = false;
		Crouch = false;
		Run = false;
	}


	void render (void) {
		//NYRenderer::getInstance()->drawSolidCube(Width / 2);
	}

	void update (float elapsed) {
		avance = GetAsyncKeyState (VK_Z);
		recule = GetAsyncKeyState (VK_S);
		gauche = GetAsyncKeyState (VK_Q);
		droite = GetAsyncKeyState (VK_D);

		//Par defaut, on applique la gravité (-100 sur Z), la moitie si dans l eau
		NYVert3Df force = NYVert3Df (0, 0, -1) * 100.0f;
		if (InWater)
			force = NYVert3Df (0, 0, -1) * 5.0f;

		float lastheight = CurrentHeight;
		CurrentHeight = Height;
		if (Crouch)
			CurrentHeight = Height / 2;

		//Pour ne pas s'enfoncer dans le sol en une frame quand on se releve
		if (CurrentHeight > lastheight)
			Position.Z += Height / 4;

		//Si l'avatar n'est pas au sol, alors il ne peut pas sauter

		float accel = 400;
		if (Crouch)
			accel = 200;
		if (!Standing)
			accel = 50;
		if (Run)
			accel = 800;

		NYVert3Df forward (Cam->_Direction.X, Cam->_Direction.Y, 0);
		forward.normalize ();
		NYVert3Df right (Cam->_NormVec.X, Cam->_NormVec.Y, 0);
		right.normalize ();

		//On applique les controles en fonction de l'accélération
		if (avance)
			force += forward * accel;
		if (recule)
			force += forward * -accel;
		if (gauche)
			force += right * -accel;
		if (droite)
			force += right * accel;


		//On applique le jump
		if (Jump) {
			force += NYVert3Df (0, 0, 1) * 55.0f / elapsed; //(impulsion, pas fonction du temps)
			Jump = false;
		}

		//On applique les forces en fonction du temps écoulé
		Speed += force * elapsed;

		//On met une limite a sa vitesse horizontale
		float speedmax = 70;
		if (Crouch)
			speedmax = 45;
		if (!Standing)
			speedmax = 70;
		if (Run)
			speedmax = 140;

		NYVert3Df horSpeed = Speed;
		horSpeed.Z = 0;
		if (horSpeed.getSize () > speedmax) {
			horSpeed.normalize ();
			horSpeed *= speedmax;
			Speed.X = horSpeed.X;
			Speed.Y = horSpeed.Y;
		}

		//On le déplace, en sauvegardant son ancienne position
		NYVert3Df oldPosition = Position;
		Position += (Speed * elapsed);

		//Log::log(Log::ENGINE_INFO, ("zS " + toString(Speed.Z)).c_str());

		if (_TimerStanding.getElapsedSeconds () > 0.01)
			Standing = false;

		for (int pass = 0; pass < 2; pass++) {
			for (int i = 0; i < 6; i++) {
				float valueColMin = 0;
				NYAxis axis = World->getMinCol (Position, Speed, Width, CurrentHeight, valueColMin, i < 3);
				//Log::log(Log::ENGINE_INFO,"check");
				if (axis != 0) {
					//valueColMin = nymax(nyabs(valueColMin), 0.0001f) * (valueColMin > 0 ? 1.0f : -1.0f);
					if (axis & NY_AXIS_X) {
						//Log::log(Log::ENGINE_INFO,("x " + toString(valueColMin)).c_str());
						Position.X += valueColMin + 0.001*nysign (valueColMin);
						Speed.X = 0;
					}
					if (axis & NY_AXIS_Y) {
						//Log::log(Log::ENGINE_INFO, ("y " + toString(valueColMin)).c_str());
						Position.Y += valueColMin + 0.001*nysign (valueColMin);
						Speed.Y = 0;
					}
					if (axis & NY_AXIS_Z) {
						Log::log(Log::ENGINE_INFO, ("z " + toString(valueColMin)).c_str());
						Speed.Z = 0;
						Position.Z += valueColMin + 0.001*nysign (valueColMin);
						Standing = true;
						_TimerStanding.start ();
					}
				}
			}
		}
		Log::log(Log::ENGINE_INFO, ("z final " + toString(Position.Z)).c_str());
		printDebug (toString (Position.Z));


		int x = (int) (Position.X / NYCube::CUBE_SIZE);
		int y = (int) (Position.Y / NYCube::CUBE_SIZE);
		int z = (int) (Position.Z / NYCube::CUBE_SIZE);


		//Escaliers
		float floatheight = 1.0f;
		float zpied = Position.Z - (Height / 2.0f);
		float zfloatpied = zpied - floatheight;
		int izCubeDessousFloat = (int) ((zfloatpied) / NYCube::CUBE_SIZE);
		float zCubeDessous2Float = zfloatpied - NYCube::CUBE_SIZE;
		int izCubeDessous2Float = (int) (zCubeDessous2Float / NYCube::CUBE_SIZE);


		//Si on est dans l'eau
		InWater = false;
		if (World->getCube (x, y, z)->_Type == CUBE_STILL_WATER)
			InWater = true;

		if (InWater) {
			//Standing = true;
			Speed *= pow (0.01f, elapsed);
		}

		if (Standing)
			Speed *= pow (0.01f, elapsed);

		Cam->moveTo (Position + NYVert3Df (0.0f, 0.0f, CurrentHeight));
	}
};

#endif
//*/