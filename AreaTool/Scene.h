#pragma once

#include "Define.h"

class Object;

class Scene
{
public:
	Scene();
	~Scene();

	void OnMouseMove(WPARAM wParam, int x, int y);
	void OnMouseLDown(WPARAM wParam, int x, int y);
	void OnMouseRDown(WPARAM wParam, int x, int y);
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam, float fTimeElapsed);

	void CreateObjects();
	void ReleaseObjects();

	bool ProcessInput(float timeElapsed, HWND hwnd);
	void AnimateObjects(float fTimeElapsed);
	void Render(float fTimeElapsed);

private:
	typedef std::shared_ptr<Object> SP_Object;
	std::shared_ptr<Object> m_spkGrid;
	std::shared_ptr<Object> m_spkGizmo;
	std::list<SP_Object> m_listObject;
	std::shared_ptr<Object> m_spkSelectObject;
	POINT	m_poLastMousePos;
	DEFINE::CHANGE_TYPE m_eChangeType;
};