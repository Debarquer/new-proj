#pragma once
#include "SimpleMath.h"

#include "Defines.h"

using namespace DirectX::SimpleMath;

class Player
{
public:
	Player();
	~Player();

	float m_fallRate;
	float m_jumpHeight;

	Vector3 m_cameraPosition;
	Vector3 m_lookAt;

	float m_rotationSpeed;
	float m_movementSpeed;

	void Update();
	void moveCharacter(int factorX, int factorY, int factorZ);
	void rotateCharacter(int factorX, int factorY);
};

