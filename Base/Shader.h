#include "Helpers.h"
#include <SFML/OpenGL.hpp>

class Shader
{
public:
	Shader() = default;
	~Shader() = default;

	bool Compile(const char* pSource);
private:
	GLint m_Shader;
};