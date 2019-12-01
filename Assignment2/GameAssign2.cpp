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
	//texture = txtrManager.LoadTGAFile("meme.tga");
    //Memory manager test
	
	FILE* pFile = fopen("meme.tga", "rb");

	fseek(pFile, 0, SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);

	//allocate memory
	void* buffer = (unsigned char*)malloc(sizeof(char)*lSize);
	memset(buffer, 0, lSize);
	//write to memory
	fread(buffer, 1, lSize, pFile);
	texture = txtrManager.LoadTGAFile(buffer);
	//Archiver tests
	//txtrManager.LoadTGAFile("Phone.tga");

#ifndef MACOS
	//Archiver tests
    DummyStruct object1(1, 3, 3, 7, 6, 9);
    DummyStruct2D object2(23, 34, 45);

    Archiver& archiver = Archiver::GetInstance();

    char testString[] =
        "Dynamic allocation of memory is always an expensive operation but most software doesn't really need to care."
        "However, if it is a realtime low latency software, for example, a game, the time allocations take does actually matter."
        "When allocating dynamic memory, i.e. malloc() or C++'s global new-operator,"
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

    char testString2[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Praesent tristique magna sit amet purus gravida quis blandit turpis. Euismod in pellentesque massa placerat duis ultricies lacus sed. Ac auctor augue mauris augue neque. Pellentesque adipiscing commodo elit at. Mi sit amet mauris commodo quis imperdiet. Praesent tristique magna sit amet purus gravida quis blandit turpis. Varius morbi enim nunc faucibus a pellentesque sit amet porttitor. Cras tincidunt lobortis feugiat vivamus at augue eget. Cras ornare arcu dui vivamus arcu felis bibendum ut tristique. Quisque non tellus orci ac auctor augue. Pulvinar pellentesque habitant morbi tristique senectus et. Orci sagittis eu volutpat odio facilisis mauris sit. Sed blandit libero volutpat sed cras ornare arcu. Tincidunt nunc pulvinar sapien et ligula ullamcorper. Bibendum at varius vel pharetra vel. Tristique senectus et netus et malesuada fames. Adipiscing at in tellus integer feugiat scelerisque varius. Vitae elementum curabitur vitae nunc sed velit. Facilisis mauris sit amet massa vitae tortor condimentum lacinia. Sit amet porttitor eget dolor morbi non arcu risus quis. Tellus mauris a diam maecenas sed enim ut. Vel fringilla est ullamcorper eget. Viverra aliquet eget sit amet tellus cras adipiscing enim. In dictum non consectetur a erat nam at lectus urna. Quis vel eros donec ac odio tempor orci. Leo a diam sollicitudin tempor id eu nisl. Elit pellentesque habitant morbi tristique senectus et netus et. Lorem donec massa sapien faucibus et molestie ac. Risus commodo viverra maecenas accumsan lacus vel facilisis volutpat est. Rhoncus urna neque viverra justo nec. Faucibus turpis in eu mi. Metus aliquam eleifend mi in nulla posuere sollicitudin aliquam ultrices. Imperdiet proin fermentum leo vel orci porta non. Vitae tempus quam pellentesque nec nam aliquam sem et. Viverra nam libero justo laoreet. Ultricies mi quis hendrerit dolor magna. Ac turpis egestas integer eget aliquet nibh praesent. Sit amet luctus venenatis lectus magna fringilla. Purus in mollis nunc sed id semper risus in. Felis eget nunc lobortis mattis. Morbi tristique senectus et netus et malesuada fames. Gravida arcu ac tortor dignissim. Nisl nisi scelerisque eu ultrices vitae auctor eu augue ut. Turpis egestas sed tempus urna. Proin nibh nisl condimentum id. Pretium viverra suspendisse potenti nullam ac tortor vitae purus. Eget nunc scelerisque viverra mauris in aliquam sem fringilla ut. Tortor id aliquet lectus proin nibh nisl condimentum id venenatis. Orci dapibus ultrices in iaculis. Commodo sed egestas egestas fringilla phasellus faucibus. Quis blandit turpis cursus in hac habitasse platea. Quam pellentesque nec nam aliquam sem et tortor. Pharetra magna ac placerat vestibulum lectus mauris. Nunc mi ipsum faucibus vitae aliquet. Amet tellus cras adipiscing enim eu turpis egestas. At augue eget arcu dictum varius duis at. Sagittis id consectetur purus ut faucibus pulvinar elementum. At erat pellentesque adipiscing commodo elit at imperdiet dui accumsan. In massa tempor nec feugiat nisl pretium fusce id velit. Faucibus turpis in eu mi bibendum neque. Quis blandit turpis cursus in hac. Dui vivamus arcu felis bibendum ut tristique et egestas quis. Posuere urna nec tincidunt praesent semper feugiat nibh sed pulvinar. Sit amet mauris commodo quis imperdiet massa tincidunt nunc pulvinar. Phasellus vestibulum lorem sed risus ultricies. Blandit libero volutpat sed cras ornare arcu dui vivamus. Nibh tellus molestie nunc non blandit massa enim. Nunc mi ipsum faucibus vitae aliquet nec ullamcorper. Donec ultrices tincidunt arcu non sodales neque sodales ut etiam. Massa massa ultricies mi quis hendrerit dolor magna eget. Morbi tincidunt augue interdum velit euismod in pellentesque massa placerat. Tristique sollicitudin nibh sit amet commodo nulla facilisi nullam. Sed odio morbi quis commodo odio. Arcu dictum varius duis at consectetur. Nunc mi ipsum faucibus vitae aliquet. Platea dictumst quisque sagittis purus sit. Morbi leo urna molestie at elementum eu facilisis sed. Ultricies integer quis auctor elit sed. Vitae congue mauris rhoncus aenean vel elit scelerisque. Malesuada fames ac turpis egestas integer eget aliquet nibh praesent. Risus at ultrices mi tempus. Leo vel orci porta non. Lobortis scelerisque fermentum dui faucibus in. Nunc scelerisque viverra mauris in aliquam sem. Euismod quis viverra nibh cras pulvinar. Amet cursus sit amet dictum sit amet justo donec enim. Auctor neque vitae tempus quam pellentesque nec. Pharetra diam sit amet nisl suscipit adipiscing bibendum. Tempus egestas sed sed risus. Dolor sit amet consectetur adipiscing elit pellentesque. Enim ut sem viverra aliquet eget sit. Pulvinar elementum integer enim neque volutpat ac tincidunt vitae. Enim blandit volutpat maecenas volutpat blandit. Non sodales neque sodales ut etiam sit amet nisl. In massa tempor nec feugiat nisl pretium fusce. Egestas maecenas pharetra convallis posuere morbi leo urna molestie. Arcu ac tortor dignissim convallis aenean et. A erat nam at lectus urna duis. Dolor purus non enim praesent.";

    archiver.CreateUncompressedPackage();
	archiver.AddToUncompressedPackage(HashString("our texture"), sizeof(char)*lSize, buffer);
    archiver.AddToUncompressedPackage(HashString("Object 1"), sizeof(DummyStruct), &object1);
    archiver.AddToUncompressedPackage(HashString("Object 2"), sizeof(DummyStruct2D), &object2);
    archiver.AddToUncompressedPackage(HashString("Test String"), sizeof(testString), testString);
    archiver.AddToUncompressedPackage(HashString("Test String 2"), sizeof(testString2), testString2);
    archiver.SaveUncompressedPackage("package");
    archiver.CloseUncompressedPackage();

    archiver.OpenCompressedPackage("package", Archiver::LOAD_AND_PREPARE);

    size_t testStringHash = HashString("Test String");

    size_t testStringSize = archiver.ReadRequiredSizeForPackageData(testStringHash);

    void* decompressedTestStringData = MemoryManager::GetInstance().Allocate(testStringSize, 1, "Test String Decompressed");
    archiver.ReadPackageData(testStringHash, decompressedTestStringData, testStringSize);

    std::string decompressedTestString = reinterpret_cast<char*>(decompressedTestStringData);

	//size_t testTextureHash = HashString("our texture");
	//size_t testTextureSize = archiver.ReadRequiredSizeForPackageData(testTextureHash);

	////test texture
	//void* decompressedTestTextureData = MemoryManager::GetInstance().Allocate(testTextureHash, 1, "Test Texture Decompressed");
	//archiver.ReadPackageData(testTextureHash, decompressedTestTextureData, testTextureSize);
	//texture = txtrManager.LoadTGAFile(decompressedTestStringData);

    std::cout << "Decompressed String: " << decompressedTestString << std::endl;
#endif

	//Construct mesh
	//m_pCube = Mesh::CreateCube();
	//m_pCube->Construct();
}

void GameAssign2::Update(const sf::Time& deltaTime)
{
}

void GameAssign2::Render()
{
	m_MeshShader.setUniform("our_Texture", *texture);
	m_pCube->Draw();
}

void GameAssign2::RenderImGui()
{
}

void GameAssign2::Release()
{
	//delete m_pCube;
	//m_pCube = nullptr;
}
