#include "stdafx.h"
#include "Define.h"
#include "DeviceManager.h"
#include "GeometryGenerator.h"
#include "Camera.h"

#include "Object.h"

Object::Object()
{
	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
	m_pVertexLayout = nullptr;
	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
	m_pConstantBuffer = nullptr;
	m_pRasterizerStateWire = nullptr;
	m_pRasterizerStateSolid = nullptr;

	m_bSelect = false;

	m_f3Scale = XMFLOAT3(1.f, 1.f, 1.f);
	m_vRight = XMFLOAT3(1.f, 0.f, 0.f);
	m_vUp = XMFLOAT3(0.f, 1.f, 0.f);
	m_vLook = XMFLOAT3(0.f, 0.f, 1.f);
	m_vPosition = XMFLOAT3(0.f, 0.f, 0.f);
	m_f3Rotation = XMFLOAT3(0.f, 0.f, 0.f);

	m_AxisAlignedBox.Center = m_vPosition;
	m_AxisAlignedBox.Extents = XMFLOAT3(0.5f, 0.5f, 0.5f);
	m_AxisAlignedBox.Scale = XMFLOAT3(1.f, 1.f, 1.f);

	m_BaseOrientedBox.Center = m_vPosition;
	m_BaseOrientedBox.Extents = XMFLOAT3(0.5f, 0.5f, 0.5f);
	m_BaseOrientedBox.Orientation = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
	m_OrientedBox = m_BaseOrientedBox;

	m_f4Color = DEFINE::COLOR_WHITE;
}

Object::~Object()
{
	if (m_pRasterizerStateSolid) m_pRasterizerStateSolid->Release();
	if (m_pRasterizerStateWire) m_pRasterizerStateWire->Release();
	if (m_pConstantBuffer) m_pConstantBuffer->Release();
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pIndexBuffer) m_pIndexBuffer->Release();
	if (m_pVertexLayout) m_pVertexLayout->Release();
	if (m_pVertexShader) m_pVertexShader->Release();
	if (m_pPixelShader) m_pPixelShader->Release();
}

HRESULT Object::Init()
{
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = nullptr;
	hr = CreateVertexShader(pVSBlob);
	if (FAILED(hr))
	{
		std::cout << "Failed CreateVertexShader" << std::endl;
		return hr;
	}

	hr = CreateInputLayout(pVSBlob);
	if (FAILED(hr))
	{
		std::cout << "Failed CreateInputLayout" << std::endl;
		return hr;
	}

	hr = CreatePixelShader();
	if (FAILED(hr))
	{
		std::cout << "Failed CreatePixelShader" << std::endl;
		return hr;
	}

	BuildGeometryBuffers();

	hr = CreateConstantBuffer();
	if (FAILED(hr))
	{
		std::cout << "Failed CreateConstantBuffer" << std::endl;
		return hr;
	}

	hr = CreateRasterizerState();
	if (FAILED(hr))
	{
		std::cout << "Failed CreateRasterizerState" << std::endl;
		return hr;
	}

	XMMATRIX g_World = XMMatrixIdentity();
	XMStoreFloat4x4(&m_f4x4World, g_World);

	m_World = XMMatrixIdentity();

	return hr;
}

//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT Object::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

HRESULT Object::CreateRasterizerState()
{
	HRESULT hResult = S_OK;
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	hResult = DeviceManager::GetInstance().GetDevice()->CreateRasterizerState(&rsDesc, &m_pRasterizerStateWire);
	if (FAILED(hResult))
		return hResult;

	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	hResult = DeviceManager::GetInstance().GetDevice()->CreateRasterizerState(&rsDesc, &m_pRasterizerStateSolid);
	if (FAILED(hResult))
		return hResult;

	return hResult;
}

HRESULT Object::CreateConstantBuffer()
{
	HRESULT hResult = S_OK;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hResult = DeviceManager::GetInstance().GetDevice()->CreateBuffer(&bd, NULL, &m_pConstantBuffer);

	return hResult;
}

HRESULT Object::CreatePixelShader()
{
	HRESULT hResult = S_OK;
	ID3DBlob* pPSBlob = NULL;
	hResult = CompileShaderFromFile(L"AreaTool.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hResult))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hResult;
	}

	hResult = DeviceManager::GetInstance().GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader);
	pPSBlob->Release();

	return hResult;
}

HRESULT Object::CreateInputLayout(ID3DBlob * &pVSBlob)
{
	HRESULT hResult = S_OK;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hResult = DeviceManager::GetInstance().GetDevice()->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &m_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hResult))
		return hResult;

	return hResult;
}

HRESULT Object::CreateVertexShader(ID3DBlob * &pVSBlob)
{
	HRESULT hResult = S_OK;
	hResult = CompileShaderFromFile(L"AreaTool.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hResult))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hResult;
	}

	hResult = DeviceManager::GetInstance().GetDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader);
	if (FAILED(hResult))
	{
		pVSBlob->Release();
		return hResult;
	}

	return hResult;
}

void Object::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData object;

	CreateFigure(object);

	m_iVertexOffset = 0;
	m_uiIndexCount = static_cast<UINT>(object.Indices.size());
	m_uiIndexOffset = 0;

	std::vector<SimpleVertex> vertices(object.Vertices.size());

	for (size_t i = 0; i < object.Vertices.size(); ++i)
	{
		vertices[i].m_f3Pos = object.Vertices[i].Position;
		vertices[i].m_f4Color = m_f4Color;
	}

	CreateVertexBuffer(vertices);
	CreateIndexBuffer(object.Indices);
}

void Object::CreateIndexBuffer(const std::vector<UINT> &indices)
{
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	DeviceManager::GetInstance().GetDevice()->CreateBuffer(&ibd, &iinitData, &m_pIndexBuffer);
}

void Object::CreateVertexBuffer(const std::vector<SimpleVertex> &vertices)
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * static_cast<UINT>(vertices.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	DeviceManager::GetInstance().GetDevice()->CreateBuffer(&vbd, &vinitData, &m_pVertexBuffer);
}

void Object::WorldMatrixSRT()
{
	XMMATRIX scale = XMMatrixScaling(m_f3Scale.x, m_f3Scale.y, m_f3Scale.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z));
	XMMATRIX trans = XMMatrixTranslation(m_vPosition.x, m_vPosition.y, m_vPosition.z);
	m_World = XMMatrixIdentity();
	m_World = XMMatrixMultiply(XMMatrixMultiply(XMMatrixMultiply(m_World, scale), rot), trans);
	
	TransformAxisAlignedBox(&m_AxisAlignedBox, &m_AxisAlignedBox, XMLoadFloat3(&m_f3Scale), XMQuaternionRotationMatrix(rot), XMLoadFloat3(&m_vPosition));
	
	m_OrientedBox.Extents.x = m_BaseOrientedBox.Extents.x * m_f3Scale.x;
	m_OrientedBox.Extents.y = m_BaseOrientedBox.Extents.y * m_f3Scale.y;
	m_OrientedBox.Extents.z = m_BaseOrientedBox.Extents.z * m_f3Scale.z;
	
	XMVECTOR vQuaternion = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z));
	XMStoreFloat4(&m_BaseOrientedBox.Orientation, vQuaternion);
	m_BaseOrientedBox.Center = m_vPosition;
}

void Object::Pitch(float fAngle)
{
	m_f3Rotation.x += fAngle;
}

void Object::Yaw(float fAngle)
{
	m_f3Rotation.y += fAngle;
}

void Object::Roll(float fAngle)
{
	m_f3Rotation.z += fAngle;
}

void Object::Translate(float d, XMFLOAT3 f3Axis)
{
	XMVECTOR vDis = XMVectorReplicate(d);
	XMVECTOR vAxis = XMLoadFloat3(&f3Axis);
	XMVECTOR position = XMLoadFloat3(&m_vPosition);
	XMStoreFloat3(&m_vPosition, XMVectorMultiplyAdd(vDis, vAxis, position));
}

void Object::Right(float d)
{
	Translate(d, m_vRight);
}

void Object::Forward(float d)
{
	Translate(d, m_vLook);
}

void Object::Up(float d)
{
	Translate(d, m_vUp);
}

void Object::ScalingX(float size)
{
	if (0 >= m_f3Scale.x + size)
		return;

	m_f3Scale.x += size;
}

void Object::ScalingY(float size)
{
	if (0 >= m_f3Scale.y + size)
		return;

	m_f3Scale.y += size;
}

void Object::ScalingZ(float size)
{
	if (0 >= m_f3Scale.z + size)
		return;

	m_f3Scale.z += size;
}

void Object::AnimateObjects(float fTimeElapsed)
{
	WorldMatrixSRT();
}

void Object::Render()
{
	ConstantBuffer cb;
	cb.m_mtWorld = XMMatrixTranspose(m_World);
	cb.m_mtView = XMMatrixTranspose(Camera::GetInstance().View());
	cb.m_mtProjection = XMMatrixTranspose(Camera::GetInstance().Proj());
	DeviceManager::GetInstance().GetDeviceContext()->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	DeviceManager::GetInstance().GetDeviceContext()->IASetInputLayout(m_pVertexLayout);
	DeviceManager::GetInstance().GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	DeviceManager::GetInstance().GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceManager::GetInstance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	if(m_bSelect)
		DeviceManager::GetInstance().GetDeviceContext()->RSSetState(m_pRasterizerStateSolid);
	else
		DeviceManager::GetInstance().GetDeviceContext()->RSSetState(m_pRasterizerStateWire);
	DeviceManager::GetInstance().GetDeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	DeviceManager::GetInstance().GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceManager::GetInstance().GetDeviceContext()->PSSetShader(m_pPixelShader, NULL, 0);
	
	DeviceManager::GetInstance().GetDeviceContext()->DrawIndexed(m_uiIndexCount, m_uiIndexOffset, m_iVertexOffset);
}

///////////////////////////////////////////////////////////////////////////
Box::Box() : Object()
{
	Init();
}

Box::~Box()
{

}

void Box::CreateFigure(GeometryGenerator::MeshData& kMeshData)
{
	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, kMeshData);
}

///////////////////////////////////////////////////////////////////////////
Cylinder::Cylinder() : Object()
{
	Init();

}

Cylinder::~Cylinder()
{

}

void Cylinder::CreateFigure(GeometryGenerator::MeshData& kMeshData)
{
	GeometryGenerator geoGen;
	geoGen.CreateCylinder(0.5f, 0.5f, 1.0f, 8, 1, kMeshData);
}

///////////////////////////////////////////////////////////////////////////
Sphere::Sphere() : Object()
{
	Init();
}

Sphere::~Sphere()
{

}

void Sphere::CreateFigure(GeometryGenerator::MeshData& kMeshData)
{
	GeometryGenerator geoGen;
	geoGen.CreateSphere(0.5f, 10, 10, kMeshData);
}

///////////////////////////////////////////////////////////////////////////
Grid::Grid() : Object()
{
	m_f4Color = DEFINE::COLOR_GRAY;

	Init();

	m_AxisAlignedBox.Center = m_vPosition;
	m_AxisAlignedBox.Extents = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

Grid::~Grid()
{

}

void Grid::CreateFigure(GeometryGenerator::MeshData& kMeshData)
{
	GeometryGenerator geoGen;
	geoGen.CreateGrid(30.0f, 30.0f, 30, 30, kMeshData);
}

///////////////////////////////////////////////////////////////////////////