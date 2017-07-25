#pragma once

namespace DEFINE
{
	const int SCREEN_WIDTH = 640;
	const int SCREEN_HEIGHT = 480;
	const XMFLOAT4 COLOR_RED(1.0f, 0.f, 0.f, 1.f);
	const XMFLOAT4 COLOR_GREEN(0.f, 1.f, 0.f, 1.f);
	const XMFLOAT4 COLOR_BLUE(0.f, 0.f, 1.f, 1.f);
	const XMFLOAT4 COLOR_YELLOW(1.f, 1.f, 0.f, 1.f);
	const XMFLOAT4 COLOR_BLACK(0.0f, 0.0f, 0.0f, 1.0f);
	const XMFLOAT4 COLOR_GRAY(0.5f, 0.5f, 0.5f, 1.0f);
	const XMFLOAT4 COLOR_WHITE(1.0f, 1.0f, 1.0f, 1.0f);

	enum FIGURE_TYPE
	{
		F_GRID,
		F_BOX,
		F_CYLINDER,
		F_SPHERE,
		MAX_FIGURE_TYPE
	};

	enum CHANGE_TYPE
	{
		C_TRNAS,
		C_ROT,
		C_SCALE,
		MAX_CHANGE_TYPE
	};
}