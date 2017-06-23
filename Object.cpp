#include "stdafx.h"
#include "Define.h"
#include "GeometryGenerator.h"
#include "Camera.h"

#include "Object.h"

Object::Object(ID3D11Device* &pd3dDevice, int num)
{
	m_iNum = num;
	m_bSelect = false;

	m_f3Scale = XMFLOAT3(1.f, 1.f, 1.f);
	m_vRight = XMFLOAT3(1.f, 0.f, 0.f);
	m_vUp = XMFLOAT3(0.f, 1.f, 0.f);
	m_vLook = XMFLOAT3(0.f, 0.f, 1.f);
	m_vPosition = XMFLOAT3(0.f, 0.f, 0.f);
	m_f3Rotation = XMFLOAT3(0.f, 0.f, 0.f);
	Init(pd3dDevice);

	if (m_iNum != DEFINE::F_GRID)
	{
		m_AxisAlignedBox.Center = m_vPosition;
		m_AxisAlignedBox.Extents = XMFLOAT3(0.5f, 0.5f, 0.5f);
		m_AxisAlignedBox.Scale = XMFLOAT3(1.f, 1.f, 1.f);
	}
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

HRESULT Object::Init(ID3D11Device* &pd3dDevice)
{
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = NULL;
	hr = CreateVertexShader(pd3dDevice, pVSBlob);
	if (FAILED(hr))
	{
		std::cout << "Failed CreateVertexShader" << std::endl;
		return hr;
	}

	hr = CreateInputLayout(pd3dDevice, pVSBlob);
	if (FAILED(hr))
	{
		std::cout << "Failed CreateInputLayout" << std::endl;
		return hr;
	}

	hr = CreatePixelShader(pd3dDevice);
	if (FAILED(hr))
	{
		std::cout << "Failed CreatePixelShader" << std::endl;
		return hr;
	}

	BuildGeometryBuffers(pd3dDevice);

	hr = CreateConstantBuffer(pd3dDevice);
	if (FAILED(hr))
	{
		std::cout << "Failed CreateConstantBuffer" << std::endl;
		return hr;
	}

	hr = CreateRasterizerState(pd3dDevice);
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

HRESULT Object::CreateRasterizerState(ID3D11Device* &pd3dDevice)
{
	HRESULT hResult = S_OK;
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	hResult = pd3dDevice->CreateRasterizerState(&rsDesc, &m_pRasterizerStateWire);
	if (FAILED(hResult))
		return hResult;

	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	hResult = pd3dDevice->CreateRasterizerState(&rsDesc, &m_pRasterizerStateSolid);
	if (FAILED(hResult))
		return hResult;

	return hResult;
}

HRESULT Object::CreateConstantBuffer(ID3D11Device* &pd3dDevice)
{
	HRESULT hResult = S_OK;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hResult = pd3dDevice->CreateBuffer(&bd, NULL, &m_pConstantBuffer);

	return hResult;
}

HRESULT Object::CreatePixelShader(ID3D11Device* &pd3dDevice)
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

	hResult = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader);
	pPSBlob->Release();

	return hResult;
}

HRESULT Object::CreateInputLayout(ID3D11Device* &pd3dDevice, ID3DBlob * &pVSBlob)
{
	HRESULT hResult = S_OK;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hResult = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &m_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hResult))
		return hResult;

	return hResult;
}

HRESULT Object::CreateVertexShader(ID3D11Device* &pd3dDevice, ID3DBlob * &pVSBlob)
{
	HRESULT hResult = S_OK;
	hResult = CompileShaderFromFile(L"AreaTool.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hResult))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hResult;
	}

	hResult = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader);
	if (FAILED(hResult))
	{
		pVSBlob->Release();
		return hResult;
	}

	return hResult;
}

void Object::BuildGeometryBuffers(ID3D11Device* &pd3dDevice)
{
	GeometryGenerator::MeshData object;
	GeometryGenerator geoGen;
	XMFLOAT4 color = DEFINE::COLOR_WHITE;

	switch (m_iNum)
	{
	case DEFINE::F_BOX:
		geoGen.CreateBox(1.0f, 1.0f, 1.0f, object);
		break;
	case DEFINE::F_GRID:
	{
		color = DEFINE::COLOR_GRAY;
		geoGen.CreateGrid(30.0f, 30.0f, 30, 30, object);
	}break;
	case DEFINE::F_SPHERE:
		geoGen.CreateSphere(0.5f, 10, 10, object);
		break;
	case DEFINE::F_CYLINDER:
		geoGen.CreateCylinder(0.5f, 0.5f, 1.0f, 8, 1, object);
		break;
	default:
	{
		std::cout << "Invalid Figure Type" << std::endl;
	}break;
	}

	m_iVertexOffset = 0;
	m_uiIndexCount = object.Indices.size();
	m_uiIndexOffset = 0;

	std::vector<SimpleVertex> vertices(object.Vertices.size());

	UINT k = 0;
	for (size_t i = 0; i < object.Vertices.size(); ++i, ++k)
	{
		vertices[k].m_f3Pos = object.Vertices[i].Position;
		vertices[k].m_f4Color = color;
	}

	CreateVertexBuffer(vertices, pd3dDevice);
	CreateIndexBuffer(object.Indices, pd3dDevice);
}

void Object::CreateIndexBuffer(std::vector<UINT> &indices, ID3D11Device *& pd3dDevice)
{
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	pd3dDevice->CreateBuffer(&ibd, &iinitData, &m_pIndexBuffer);
}

void Object::CreateVertexBuffer(std::vector<SimpleVertex> &vertices, ID3D11Device *& pd3dDevice)
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	pd3dDevice->CreateBuffer(&vbd, &vinitData, &m_pVertexBuffer);
}

void Object::WorldMatrixSRT()
{
	XMMATRIX scale = XMMatrixScaling(m_f3Scale.x, m_f3Scale.y, m_f3Scale.z);
	XMMATRIX rot = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_f3Rotation.x), XMConvertToRadians(m_f3Rotation.y), XMConvertToRadians(m_f3Rotation.z));
	XMMATRIX trans = XMMatrixTranslation(m_vPosition.x, m_vPosition.y, m_vPosition.z);
	m_World = XMMatrixIdentity();
	m_World = XMMatrixMultiply(XMMatrixMultiply(XMMatrixMultiply(m_World, scale), rot), trans);
	
	TransformAxisAlignedBox(&m_AxisAlignedBox, &m_AxisAlignedBox, XMLoadFloat3(&m_f3Scale), XMQuaternionRotationMatrix(rot), XMLoadFloat3(&m_vPosition));
}

void Object::Pitch(float fAngle)
{
	m_f3Rotation.x += fAngle;
	WorldMatrixSRT();
}

void Object::Yaw(float fAngle)
{
	m_f3Rotation.y += fAngle;
	WorldMatrixSRT();
}


void Object::Roll(float fAngle)
{
	m_f3Rotation.z += fAngle;
	WorldMatrixSRT();
}

void Object::Right(float d)
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&m_vRight);
	XMVECTOR position = XMLoadFloat3(&m_vPosition);
	XMStoreFloat3(&m_vPosition, XMVectorMultiplyAdd(s, r, position));

	WorldMatrixSRT();
}

void Object::Forward(float d)
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&m_vLook);
	XMVECTOR position = XMLoadFloat3(&m_vPosition);
	XMStoreFloat3(&m_vPosition, XMVectorMultiplyAdd(s, l, position));

	WorldMatrixSRT();
}

void Object::Up(float d)
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR u = XMLoadFloat3(&m_vUp);
	XMVECTOR position = XMLoadFloat3(&m_vPosition);
	XMStoreFloat3(&m_vPosition, XMVectorMultiplyAdd(s, u, position));

	WorldMatrixSRT();
}

void Object::ScalingX(float size)
{
	if (0 >= m_f3Scale.x + size)
		return;

	m_f3Scale.x += size;
	WorldMatrixSRT();
}

void Object::ScalingY(float size)
{
	if (0 >= m_f3Scale.y + size)
		return;

	m_f3Scale.y += size;
	WorldMatrixSRT();
}

void Object::ScalingZ(float size)
{
	if (0 >= m_f3Scale.z + size)
		return;

	m_f3Scale.z += size;
	WorldMatrixSRT();
}

void Object::Render(ID3D11DeviceContext* pDeviceContext)
{
	ConstantBuffer cb;
	cb.m_mtWorld = XMMatrixTranspose(m_World);
	cb.m_mtView = XMMatrixTranspose(Camera::GetInstance().View());
	cb.m_mtProjection = XMMatrixTranspose(Camera::GetInstance().Proj());
	pDeviceContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	pDeviceContext->IASetInputLayout(m_pVertexLayout);
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	if(m_bSelect)
		pDeviceContext->RSSetState(m_pRasterizerStateSolid);
	else
		pDeviceContext->RSSetState(m_pRasterizerStateWire);
	pDeviceContext->VSSetShader(m_pVertexShader, NULL, 0);
	pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	pDeviceContext->PSSetShader(m_pPixelShader, NULL, 0);
	
	pDeviceContext->DrawIndexed(m_uiIndexCount, m_uiIndexOffset, m_iVertexOffset);
}