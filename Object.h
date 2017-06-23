#pragma once

#include "xnacollision.h"

struct SimpleVertex
{
	XMFLOAT3 m_f3Pos;
	XMFLOAT4 m_f4Color;
};

struct ConstantBuffer
{
	XMMATRIX m_mtWorld;
	XMMATRIX m_mtView;
	XMMATRIX m_mtProjection;
};

class Object
{
public:
	Object(ID3D11Device* &pd3dDevice, int num);
	~Object();

	HRESULT		Init(ID3D11Device* &pd3dDevice);
	
	HRESULT		CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT		CreateRasterizerState(ID3D11Device* &pd3dDevice);
	HRESULT		CreateConstantBuffer(ID3D11Device* &pd3dDevice);
	HRESULT		CreatePixelShader(ID3D11Device* &pd3dDevice);
	HRESULT		CreateInputLayout(ID3D11Device* &pd3dDevice, ID3DBlob * &pVSBlob);
	HRESULT		CreateVertexShader(ID3D11Device* &pd3dDevice, ID3DBlob * &pVSBlob);
	void		BuildGeometryBuffers(ID3D11Device* &pd3dDevice);
	void		CreateIndexBuffer(std::vector<UINT> &indices, ID3D11Device *& pd3dDevice);
	void		CreateVertexBuffer(std::vector<SimpleVertex> &vertices, ID3D11Device *& pd3dDevice);

	void		WorldMatrixSRT();
	void		Pitch(float fAngle);
	void		Yaw(float fAngle);
	void		Roll(float fAngle);
	void		Right(float d);
	void		Forward(float d);
	void		Up(float d);
	void		ScalingX(float size);
	void		ScalingY(float size);
	void		ScalingZ(float size);

	XMFLOAT3	GetRight() const { return m_vRight; }
	XMFLOAT3	GetUp() const { return m_vUp; }
	XMFLOAT3    GetLook() const { return m_vLook; }
	XNA::AxisAlignedBox GetAABB() const { return m_AxisAlignedBox; }
	void		SetSelection() { m_bSelect = !m_bSelect; }

	void		Render(ID3D11DeviceContext* pDeviceContext);

private:
	XMFLOAT3 m_vRight;
	XMFLOAT3 m_vUp;
	XMFLOAT3 m_vLook;
	XMFLOAT3 m_f3Scale;
	XMFLOAT3 m_f3Rotation;
	XMFLOAT3 m_vPosition;

	ID3D11VertexShader*     m_pVertexShader;
	ID3D11PixelShader*      m_pPixelShader;
	ID3D11InputLayout*      m_pVertexLayout;
	ID3D11Buffer*           m_pVertexBuffer;
	ID3D11Buffer*           m_pIndexBuffer;
	ID3D11Buffer*           m_pConstantBuffer;
	ID3D11RasterizerState*	m_pRasterizerStateWire;
	ID3D11RasterizerState*	m_pRasterizerStateSolid;

	XMFLOAT4X4			m_f4x4World;
	XMMATRIX                m_World;

	XNA::AxisAlignedBox m_AxisAlignedBox;
	bool m_bSelect;

	int m_iVertexOffset;
	UINT m_uiIndexOffset;
	UINT m_uiIndexCount;

	int m_iNum;
};

