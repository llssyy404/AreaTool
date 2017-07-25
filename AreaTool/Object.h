#pragma once

#include "Define.h"
#include "xnacollision.h"
#include "GeometryGenerator.h"

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
	Object();
	virtual ~Object();

	HRESULT		Init();
	HRESULT		CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT		CreateRasterizerState();
	HRESULT		CreateConstantBuffer();
	HRESULT		CreatePixelShader();
	HRESULT		CreateInputLayout(ID3DBlob * &pVSBlob);
	HRESULT		CreateVertexShader(ID3DBlob * &pVSBlob);
	virtual	void	CreateFigure(GeometryGenerator::MeshData& kMeshData) = 0;
	virtual void	BuildGeometryBuffers();
	void		CreateIndexBuffer(const std::vector<UINT> &indices);
	void		CreateVertexBuffer(const std::vector<SimpleVertex> &vertices);

	void		WorldMatrixSRT();
	void		Pitch(float fAngle);
	void		Yaw(float fAngle);
	void		Roll(float fAngle);
	void		Right(float d);
	void		Forward(float d);
	void		Up(float d);
	virtual void		ScalingX(float size);
	virtual void		ScalingY(float size);
	virtual void		ScalingZ(float size);

	XMFLOAT3	GetPosition() const { return m_vPosition; }
	XMFLOAT3	GetRight() const { return m_vRight; }
	XMFLOAT3	GetUp() const { return m_vUp; }
	XMFLOAT3    GetLook() const { return m_vLook; }
	XNA::AxisAlignedBox GetAABB() const { return m_AxisAlignedBox; }
	void		SetPosition(XMFLOAT3 vPosition) { m_vPosition = vPosition; }
	void		SetSelection(bool sel) { m_bSelect = sel; }

	virtual void	AnimateObjects(float fTimeElapsed);
	virtual void	Render();

protected:
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

	XMFLOAT4X4				m_f4x4World;
	XMMATRIX                m_World;

	XNA::AxisAlignedBox m_AxisAlignedBox;
	bool m_bSelect;

	int m_iVertexOffset;
	UINT m_uiIndexOffset;
	UINT m_uiIndexCount;
};

class Box : public Object
{
public:
	Box();
	virtual ~Box();
	virtual void CreateFigure(GeometryGenerator::MeshData& kMeshData) override;
};

class Cylinder : public Object
{
public:
	Cylinder();
	virtual ~Cylinder();
	virtual void CreateFigure(GeometryGenerator::MeshData& kMeshData) override;
};

class Sphere : public Object
{
public:
	Sphere();
	virtual ~Sphere();
	virtual void CreateFigure(GeometryGenerator::MeshData& kMeshData) override;
};

class Grid : public Object
{
public:
	Grid();
	virtual ~Grid();
	virtual void CreateFigure(GeometryGenerator::MeshData& kMeshData) override;
};

class Gizmo : public Object
{
public:
	Gizmo() : Object() {}
	virtual ~Gizmo() {}
	virtual void	CreateFigure(GeometryGenerator::MeshData& kMeshData) override = 0;
	virtual void	BuildGeometryBuffers() override = 0;
	virtual void	Render() override = 0;
};

class TransGizmo : public Gizmo
{
public:
	TransGizmo();
	virtual ~TransGizmo();
	virtual void	CreateFigure(GeometryGenerator::MeshData& kMeshData) override;
	virtual void	BuildGeometryBuffers() override;
	virtual void	Render() override;

private:
	UINT m_uiIndexCountCone;
	DEFINE::CHANGE_TYPE m_eChangeType;
};

class RotationGizmo : public Gizmo
{
public:
	RotationGizmo();
	virtual ~RotationGizmo();
	virtual void	CreateFigure(GeometryGenerator::MeshData& kMeshData) override;
	virtual void	BuildGeometryBuffers() override;
	virtual void	Render() override;
};

class ScalingGizmo : public Gizmo
{
public:
	ScalingGizmo();
	virtual ~ScalingGizmo();
	virtual void	CreateFigure(GeometryGenerator::MeshData& kMeshData) override;
	virtual void	BuildGeometryBuffers() override;
	virtual void	Render() override;
};

class GizmoManager
{
public:
	GizmoManager();
	~GizmoManager();
	void SetSelection(DEFINE::CHANGE_TYPE eChangeType, bool bSelection);
	void AnimateObjects(DEFINE::CHANGE_TYPE eChangeType, float fTimeElapsed);
	void Render(DEFINE::CHANGE_TYPE eChangeType);

private:
	std::vector<std::shared_ptr<Gizmo>> m_vecGizmo;
	bool m_bSelectGiszmo;
};