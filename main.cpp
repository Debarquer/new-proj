//--------------------------------------------------------------------------------------
// BTH - Stefan Petersson 2014.
//--------------------------------------------------------------------------------------
//gitmagic


#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>

#include "SimpleMath.h"

#include "bth_image.h"

#include <WinUser.h>
#include <windowsx.h>

#include "Player.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

using namespace DirectX::SimpleMath;

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

Player player;

const int nrOfVertices = 12;

HRESULT hr = S_OK;

HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HRESULT CreateDirect3DContext(HWND wndHandle);

IDXGISwapChain* gSwapChain = nullptr;
ID3D11Device* gDevice = nullptr;
ID3D11DeviceContext* gDeviceContext = nullptr;
ID3D11RenderTargetView* gBackbufferRTV = nullptr;
ID3D11DepthStencilView* gDepthStencilView = nullptr;

ID3D11Buffer* gVertexBuffer = nullptr;
ID3D11Buffer* gConstantBuffer = nullptr;
ID3D11Texture2D* depthStencilBuffer = nullptr;

ID3D11InputLayout* gVertexLayout = nullptr;
ID3D11VertexShader* gVertexShader = nullptr;
ID3D11PixelShader* gPixelShader = nullptr;
ID3D11GeometryShader* gGeometryShader = nullptr;

float rotAngle = 0.0f;

ID3D11ShaderResourceView* gTextureView = nullptr;
ID3D11Texture2D *pTexture = nullptr;

struct CBuffers
{
	Matrix world;
	Matrix view;
	Matrix projection;
}cBuffers;

void createTexture()
{
	D3D11_TEXTURE2D_DESC bthTexDesc;
	ZeroMemory(&bthTexDesc, sizeof(bthTexDesc));
	bthTexDesc.Width = BTH_IMAGE_WIDTH;
	bthTexDesc.Height = BTH_IMAGE_HEIGHT;
	bthTexDesc.MipLevels = bthTexDesc.ArraySize = 1;
	bthTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bthTexDesc.SampleDesc.Count = 1;
	bthTexDesc.SampleDesc.Quality = 0;
	bthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	bthTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bthTexDesc.MiscFlags = 0;
	bthTexDesc.CPUAccessFlags = 0;

	ID3D11Texture2D *pTexture = NULL;
	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = (void*)BTH_IMAGE_DATA;
	data.SysMemPitch = BTH_IMAGE_WIDTH * 4 * sizeof(char);
	hr = gDevice->CreateTexture2D(&bthTexDesc, &data, &pTexture);
	if(hr != S_OK)assert(hr != S_OK);

	D3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc;
	//ZeroMemory(resViewDesc, sizeof(reif(sViewDesc));
	resViewDesc.Format = bthTexDesc.Format;
	resViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resViewDesc.Texture2D.MipLevels = bthTexDesc.MipLevels;
	resViewDesc.Texture2D.MostDetailedMip = 0;
	hr = gDevice->CreateShaderResourceView(pTexture, &resViewDesc, &gTextureView);
	if (hr != S_OK)assert(hr != S_OK);

	pTexture->Release();
}
void CreateShaders()
{
	//create vertex shader
	ID3DBlob* pVS = nullptr;
	D3DCompileFromFile(
		L"Vertex.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"VS_main",		// entry point
		"vs_4_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pVS,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
		// how to use the Error blob, see here
		// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
		);

	gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gVertexShader);
	
	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gVertexLayout);
	if (hr != S_OK)assert(hr != S_OK);
	// we do not need anymore this COM object, so we release it.
	pVS->Release();

	//create pixel shader
	ID3DBlob* pPS = nullptr;
	D3DCompileFromFile(
		L"Fragment.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"PS_main",		// entry point
		"ps_4_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pPS,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
		// how to use the Error blob, see here
		// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
		);

	hr = gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gPixelShader);
	if (hr != S_OK)assert(hr != S_OK);
	// we do not need anymore this COM object, so we release it.
	pPS->Release();

	ID3DBlob* pGS = nullptr;
	D3DCompileFromFile(
		L"Geometry.hlsl",
		nullptr,
		nullptr,
		"main",
		"gs_4_0",
		0,
		0,
		&pGS,
		nullptr);

	hr = gDevice->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &gGeometryShader);
	if (hr != S_OK)assert(hr != S_OK);

	pGS->Release();
}

void CreateTriangleData()
{
	struct TriangleVertex
	{
		float x, y, z;
		float r, g, b;
	};

	TriangleVertex triangleVertices[nrOfVertices] =
	{
		-10.0f, 10.0f, 0.0f,	//v00 pos
		1.0f, 0.0f, 0.0f,    //v00 uv

		-10.0f, -10.0f, 0.0f,	//v01
		1.0f, 0.0f, 0.0f,  //v00 uv

		10.0f, -10.0f, 0.0f,  //v02
		1.0f, 0.0f, 0.0f,   //v00 uv

		-10.0f, 10.0f, 0.0f,	//v10 pos
		1.0f, 0.0f, 0.0f,  //v00 uv

		10.0f, -10.0f, 0.0f,	//v11
		1.0f, 0.0f, 0.0f,   //v00 uv

		10.0f, 10.0f, 0.0f,   //v12
		1.0f, 0.0f, 0.0f,    //v00 uv
	};

	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(triangleVertices);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = triangleVertices;
	hr = gDevice->CreateBuffer(&bufferDesc, &data, &gVertexBuffer);
	if (hr != S_OK)assert(hr != S_OK);
}

void SetViewport()
{
	D3D11_VIEWPORT vp;
	vp.Width = (float)SCREEN_WIDTH;
	vp.Height = (float)SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gDeviceContext->RSSetViewports(1, &vp);
}

void Render()
{
	// clear the back buffer to a deep blue
	float clearColor[] = { 0, 0, 0, 1 };
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV, clearColor);
	gDeviceContext->ClearDepthStencilView(gDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0); //Might be needed soon

	gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);
	gDeviceContext->GSSetShader(gGeometryShader, nullptr, 0);

	UINT32 vertexSize = sizeof(float) * 6;
	UINT32 offset = 0;
	gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);
	gDeviceContext->GSSetConstantBuffers(0, 1, &gConstantBuffer);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gDeviceContext->IASetInputLayout(gVertexLayout);

	gDeviceContext->PSSetShaderResources(0, 1, &gTextureView);

	cBuffers.world = Matrix::CreateRotationY(rotAngle);
	cBuffers.view = Matrix::CreateLookAt(player.cameraPosition, player.lookAt, Vector3(0.0f, 1.0f, 0.0f));
	cBuffers.projection = Matrix::CreatePerspectiveFieldOfView(3.14f * 0.45f, 640.0f / 480.0f, 0.5f, 200.0f);

	rotAngle -= 0.000f;

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(CBuffers);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &cBuffers;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	hr = gDevice->CreateBuffer(&desc, &data, &gConstantBuffer);
	if (hr != S_OK)assert(hr != S_OK);

	gDeviceContext->UpdateSubresource(gConstantBuffer, 0, 0, &cBuffers, 0, 0);

	gDeviceContext->Draw(nrOfVertices, 0);
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa fönster
	
	if (wndHandle)
	{
		CreateDirect3DContext(wndHandle); //2. Skapa och koppla SwapChain, Device och Device Context

		SetViewport(); //3. Sätt viewport

		createTexture();

		CreateShaders(); //4. Skapa vertex- och pixel-shaders

		CreateTriangleData(); //5. Definiera triangelvertiser, 6. Skapa vertex buffer, 7. Skapa input layout

		ShowWindow(wndHandle, nCmdShow);

		while (WM_QUIT != msg.message)
		{
			float rotateSpeed = 0.0002;
			float rotateDistance = 400;

			POINT p;
			bool success = GetCursorPos(&p);

			if (!success)assert(!success);

			if (p.x < SCREEN_WIDTH / 2)
			{
				if (p.x < rotateDistance)
				{
					//lookAtX += rotateSpeed;
				}
			}
			if (p.x > SCREEN_WIDTH / 2)
			{
				if (p.x > SCREEN_WIDTH - rotateDistance)
				{
					//lookAtX -= rotateSpeed;
				}
			}

			if (p.y < SCREEN_HEIGHT / 2)
			{
				if (p.y < rotateDistance)
				{
					//lookAtY += rotateSpeed;
				}
			}
			if (p.y > SCREEN_HEIGHT / 2)
			{
				if (p.y > SCREEN_HEIGHT - rotateDistance)
				{
					//lookAtY -= rotateSpeed;
				}
			}

			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				Render(); //8. Rendera

				gSwapChain->Present(0, 0); //9. Växla front- och back-buffer
			}
		}

		gVertexBuffer->Release();

		gVertexLayout->Release();
		gVertexShader->Release();
		gPixelShader->Release();

		gBackbufferRTV->Release();
		gSwapChain->Release();
		gDevice->Release();
		gDeviceContext->Release();
		DestroyWindow(wndHandle);
	}

	return (int) msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.hInstance      = hInstance;
	wcex.lpszClassName = L"BTH_D3D_DEMO";
	if (!RegisterClassEx(&wcex))
		return false;

	RECT rc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindow(
		L"BTH_D3D_DEMO",
		L"BTH Direct3D Demo",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	return handle;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;		
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 0x41:
		case VK_LEFT:
			player.moveCharacter(1, 0, 0);
			break;
		case 0x44:
		case VK_RIGHT:
			player.moveCharacter(-1, 0, 0);
			break;
		case 0x57:
		case VK_UP:
			player.moveCharacter(0, 0, 1);
			break;
		case 0x53:
		case VK_DOWN:
			player.moveCharacter(0, 0, -1);
			break;
		case VK_SPACE:
			player.moveCharacter(0, 1, 0);
			break;
		case VK_SHIFT:
			player.moveCharacter(0, -1, 0);
			break;
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HRESULT CreateDirect3DContext(HWND wndHandle)
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = wndHandle;                           // the window to be used
	scd.SampleDesc.Count = 4;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

	// create a device, device context and swap chain using the information in the scd struct
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&gSwapChain,
		&gDevice,
		NULL,
		&gDeviceContext);

	if (SUCCEEDED(hr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackbufferRTV);
		pBackBuffer->Release();

		// set the render target as the back buffer
		gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, NULL);
	}

	// Create depth stencil.
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = SCREEN_WIDTH;
	depthStencilDesc.Height = SCREEN_HEIGHT;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 4;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	if (SUCCEEDED(hr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackbufferRTV);
		pBackBuffer->Release();

		// set the render target as the back buffer
		gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, NULL);

		gDevice->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
		gDevice->CreateDepthStencilView(depthStencilBuffer, NULL, &gDepthStencilView);
		gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, gDepthStencilView);
	}
	return hr;
}