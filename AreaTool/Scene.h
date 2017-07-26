#pragma once

#include "Define.h"

class Object;
class GizmoManager;

class Scene
{
public:
	Scene();
	~Scene();

	void CreateObjects();
	void ReleaseObjects();

	void OnMouseMove(WPARAM wParam, int x, int y);
	void OnMouseLDown(WPARAM wParam, int x, int y);
	void OnMouseRDown(WPARAM wParam, int x, int y);
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam, float fTimeElapsed);

	bool ProcessInput(float timeElapsed, HWND hwnd);
	void AnimateObjects(float fTimeElapsed);
	void Render(float fTimeElapsed);

private:
	typedef std::shared_ptr<Object> SP_Object;
	std::shared_ptr<GizmoManager> m_spkGizmoManager;
	std::list<SP_Object> m_listObject;
	SP_Object m_spkGrid;
	SP_Object m_spkSelectObject;
	POINT	m_poLastMousePos;
	DEFINE::CHANGE_TYPE m_eChangeType;
};