#include "olcConsoleGameEngine.h"
using namespace std;
#include <fstream>
#include <strstream>


class PerlinNoise : public olcConsoleGameEngine
{
public:
	PerlinNoise() {
		m_sAppName = L"Perlin Noise";
	}

private:
	virtual bool OnUserCreate() {
		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime) {
		Fill(0, 0, ScreenHeight(), ScreenHeight(), L' ');
		return true;
	}


};

int main() {
	PerlinNoise game;
	game.ConstructConsole(256, 256, 3, 3);
	game.Start();
	return 0;
}