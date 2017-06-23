#pragma once

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

	void CreateObjects(ID3D11Device *&pd3dDevice);
	void ReleaseObjects();

	bool ProcessInput(float timeElapsed, HWND hwnd);
	void AnimateObjects(float fTimeElapsed) {}
	void Render(ID3D11DeviceContext*&pd3dDeviceContext, float fTimeElapsed);

private:
	typedef std::list<std::shared_ptr<Object>> List_spObject;
	List_spObject m_listObject;
	List_spObject::iterator m_itSelectObject;
	POINT	m_poLastMousePos;
};