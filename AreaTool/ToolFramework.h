#pragma once

#include "Timer.h"

class Scene;

class ToolFramework
{
public:
	ToolFramework();
	~ToolFramework();

	void	Init();
	HRESULT InitDevice(HWND hWnd);
	void	CleanupDevice();
	void	InitObject();

	void	OnMouseMove(WPARAM wParam, int x, int y);
	void	OnMouseLDown(WPARAM wParam, int x, int y);
	void	OnMouseRDown(WPARAM wParam, int x, int y);
	void	OnMouseLUp(WPARAM wParam, int x, int y);
	void	OnMouseRUp(WPARAM wParam, int x, int y);
	void	OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void	OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void	Render();

private:
	Timer					m_kTimer;
	Scene*					m_pkScene;
};
