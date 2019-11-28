#include "GameAssign2.h"

#ifdef VISUAL_STUDIO
	#pragma warning(disable : 4100)		//Disable: "unreferenced formal parameter"-warning
#endif

struct DummyStruct
{
	DummyStruct(float x, float y, float z, float rotationX, float rotationY, float rotationZ)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->rotationX = rotationX;
		this->rotationY = rotationY;
		this->rotationZ = rotationZ;
	}

	float x;
	float y;
	float z;
	float rotationX;
	float rotationY;
	float rotationZ;
};

struct DummyStruct2D
{
	DummyStruct2D(float x, float y, float rotation)
	{
		this->x = x;
		this->y = y;
		this->rotation = rotation;
	}

	float x;
	float y;
	float rotation;
};

void GameAssign2::Init()
{
	DummyStruct object1(1, 3, 3, 7, 6, 9);
	DummyStruct2D object2(23, 34, 45);

	Archiver& archiver = Archiver::GetInstance();

	char testString[] = 
		"Dynamic allocation of memory is always an expensive operation but most software doesn’t really need to care."
		"However, if it is a realtime low latency software, for example, a game, the time allocations take does actually matter."
		"When allocating dynamic memory, i.e. malloc() or C++’s global new-operator,"
		"a request to the operating system is sent with a desired number of bytes."
		"This means that a system call has to be made and an interrupt request is sent to the CPU."
		"The interrupt will stall the executing thread and the operating system will allocate memory in system mode."
		"When done, the execution goes back to user mode and the waiting thread will be invoked."
		"Now it is clear that there is a lot of things happening when asking for memory."
		"To reduce the time it takes to allocate memory for a program, a custom allocator can be made."
		"The idea is to allocate a lot of memory, i.e. using malloc() or new,  at the beginning of execution,"
		"and use this already allocated memory to allocate and instantiate objects at runtime."
		"Implementing it this way we will only assign memory instead of allocating from the operating system,"
		"which means no context-switching into the operating system has to be made. The same applies for freeing memory."
		"We do not actually have to give the memory back to the operating system,"
		"we can just mark it as free available memory to be used again later.";

	archiver.CreateUncompressedPackage();
	//archiver.AddToUncompressedPackage(HashString("Object 1"), sizeof(DummyStruct), &object1);
	//archiver.AddToUncompressedPackage(HashString("Object 2"), sizeof(DummyStruct2D), &object2);
	archiver.AddToUncompressedPackage(HashString("Test String"), sizeof(testString), testString);
	archiver.SaveUncompressedPackage("package", true);

	archiver.OpenCompressedPackage("package");
	size_t testStringSize;
	void* decompressedTestStringData = archiver.ReadPackageData(HashString("Test String"), testStringSize);
	std::string decompressedTestString = reinterpret_cast<char*>(decompressedTestStringData);
	std::cout << "Decompressed String: " << decompressedTestString << std::endl;
}

void GameAssign2::Update(const sf::Time& deltaTime)
{
}

void GameAssign2::Render()
{
}

void GameAssign2::RenderImGui()
{
}

void GameAssign2::Release()
{
}
