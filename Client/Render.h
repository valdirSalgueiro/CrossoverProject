#pragma once
#include "Metrics.h"

class Render
{
public:
	Render();
	~Render();

	void draw(HDC hdc, Metrics* metrics);

private:
	std::wstring clientGUID;
};

