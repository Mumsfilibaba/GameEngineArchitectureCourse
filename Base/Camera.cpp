#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

constexpr glm::vec3 FORWARD = glm::vec3(0.0f, 0.0f, 1.0f);
constexpr glm::vec3 RIGHT = glm::vec3(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);


Camera::Camera()
	: m_View(1.0f),
	m_Projection(1.0f),
	m_Position(0.0f, 0.0f, 0.0f),
	m_Rotation(0.0f, 0.0f, 0.0f),
	m_Forward(FORWARD),
	m_Right(RIGHT),
	m_Up(UP),
	m_Aspect(16.0f / 9.0f)
{
	CreateView();
	CreateProjection();
}


void Camera::Rotate(const glm::vec3& rotation)
{
	//Add to rotation
	m_Rotation += rotation;

	//Create rotation matrix
	glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), UP);
	rotationMat = glm::rotate(rotationMat, glm::radians(rotation.x), m_Right);
	rotationMat = glm::rotate(rotationMat, glm::radians(rotation.z), m_Forward);

	//Rotate forward vector
	m_Forward = rotationMat * glm::vec4(m_Forward, 0.0f);
	//Rotate right vector
	m_Right = rotationMat * glm::vec4(m_Right, 0.0f);
	//Rotate up vector
	m_Up = rotationMat * glm::vec4(m_Up, 0.0f);
}


void Camera::Translate(const glm::vec3& translation)
{
	//Move camera
	m_Position += translation.x * m_Right + translation.y * m_Up + translation.z * m_Forward;
}


void Camera::SetPosition(const glm::vec3& position)
{
	m_Position = position;
}


void Camera::SetRotation(const glm::vec3& rotation)
{
	m_Rotation = rotation;

	//Rotate forward vector
	glm::mat3 rotationMat = glm::eulerAngleYXZ(glm::radians(m_Rotation.y), glm::radians(m_Rotation.x), glm::radians(m_Rotation.z));
	m_Forward = rotationMat * FORWARD;

	//Rotate right vector
	m_Right = rotationMat * RIGHT;
}


void Camera::SetAspect(float width, float height)
{
	m_Aspect = width / height;
}


void Camera::CreateView()
{
	//Calculate new lookat
	glm::vec3 lookAt = m_Position + m_Forward;
	m_View = glm::lookAt(m_Position, lookAt, m_Up);
}


void Camera::CreateProjection()
{
	m_Projection = glm::perspective(glm::radians(90.0f), m_Aspect, 0.01f, 10000.0f);
}