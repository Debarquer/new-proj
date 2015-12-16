#pragma once
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

class Player
{
public:
	Player();
	~Player();

	Vector3 cameraPosition;
	Vector3 lookAt;

	float rotationSpeed;
	float movementSpeed;

	void moveCharacter(int factorX, int factorY, int factorZ);
	void rotateCharacter(bool x, int factorX, bool y, int factorY);
};

