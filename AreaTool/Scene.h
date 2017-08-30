#pragma once

#include "Define.h"

enum SELECT_EXIS
{
	SEL_X,
	SEL_Y,
	SEL_Z
};

class Object;
class GizmoManager;

class Scene
{
public:
	Scene();
	~Scene();

	void CreateObjects();
	void ReleaseObjects();

	void GetRayPosAndDir(int x, int y, XMVECTOR& rayPos, XMVECTOR& rayDir);
	bool PickTransGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir);
	bool PickRotGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir);
	bool PickScaleGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir);
	bool PickGizmo(XMVECTOR& rayPos, XMVECTOR& rayDir);
	void PickObject(XMVECTOR& rayPos, XMVECTOR& rayDir);
	void DeleteSelectObject();

	void OnMouseMoveRightBtn(int x, int y);
	void OnMouseMoveLeftBtn(int x, int y);
	void OnMouseMove(WPARAM wParam, int x, int y);
	void OnMouseLDown(WPARAM wParam, int x, int y);
	void OnMouseRDown(WPARAM wParam, int x, int y);
	void OnMouseLUp(WPARAM wParam, int x, int y);
	void OnMouseRUp(WPARAM wParam, int x, int y);
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam, float fTimeElapsed);

	void AnimateObjects(float fTimeElapsed);
	void Render(float fTimeElapsed);

private:
	typedef std::shared_ptr<Object> SP_Object;
	std::unique_ptr<GizmoManager> m_upkGizmoManager;
	std::list<SP_Object> m_listObject;
	SP_Object m_spkGrid;
	SP_Object m_spkSelectObject;
	POINT	m_poLastMousePos;
	DEFINE::CHANGE_TYPE m_eChangeType;
	//bool m_bSelectGizmo;
	SELECT_EXIS m_eSelectExis;

	std::function<void(float)> m_functionOfSelObj;
};