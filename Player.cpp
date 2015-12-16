#include "Player.h"



Player::Player()
{
	m_fallRate = 0;
	m_jumpHeight = 0.02;

	m_movementSpeed = 0.1f;
	m_rotationSpeed = 0.01f;

	m_cameraPosition = Vector3(0.0f, 0.0f, -2.0f);
	m_lookAt = Vector3(0.0f, 0.0f, 0.0f);
}


Player::~Player()
{
}

void Player::Update()
{
	if (m_cameraPosition.y > Defines::groundPlane.y)
	{
		m_fallRate += Defines::g;
		m_cameraPosition.y -= m_fallRate;
		m_lookAt.y -= m_fallRate;
		
	}
	if (m_cameraPosition.y <= Defines::groundPlane.y)
	{
		m_fallRate = 0;
		m_cameraPosition.y = Defines::groundPlane.y;
		m_lookAt.y = Defines::groundPlane.y;
	}
}

void Player::moveCharacter(int factorX, int factorY, int factorZ)
{
	m_cameraPosition.x += factorX * m_movementSpeed;
	m_lookAt.x += factorX * m_movementSpeed;
		
	m_cameraPosition.y += factorY *  1;//factorY * m_jumpHeight;
	m_lookAt.y += factorY * 1;//factorY * m_jumpHeight;
	m_fallRate += -factorY * m_jumpHeight;

	m_cameraPosition.z += factorZ * m_movementSpeed;
	m_lookAt.z += factorZ * m_movementSpeed;
}

void Player::rotateCharacter(int factorX, int factorY)
{
	m_lookAt.x += factorX * m_rotationSpeed;

	m_lookAt.y += factorY * m_rotationSpeed;
}