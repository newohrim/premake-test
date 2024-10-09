#include "core/Game.h"

#ifdef _WIN32

#include <windows.h>

int CALLBACK WinMain(
	HINSTANCE   hInstance,
	HINSTANCE   hPrevInstance,
	LPSTR       lpCmdLine,
	int         nCmdShow
) 
{
	Game game;
	bool success = game.Initialize("E1", 800, 800);
	if (success)
	{
		game.RunLoop();
	}
	game.Shutdown();

	return 0;
}

#else

int main(int argc, char* argv[])
{
	Game game;
	bool success = game.Initialize("E1", 800, 800);
	if (success)
	{
		game.RunLoop();
	}
	game.Shutdown();

	return 0;
}

#endif // _WIN32
