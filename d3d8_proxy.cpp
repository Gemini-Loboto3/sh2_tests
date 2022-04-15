#include "framework.h"
#include "d3d8_proxy.h"

HWND mainWindow;

void Gfx_resize_window(HWND hWnd, int width, int height, RECT* lpResRect)
{
	bool menu;
	int borderless;
	DWORD dwStyle;
	UINT uFlags;
	struct tagPOINT p1;
	struct tagPOINT p0;
	RECT rc;

	int sx = 0;//gfx_main.DesktopW;
	int sy = 0;//gfx_main.DesktopH;

	menu = (GetMenu(hWnd) != 0);
	dwStyle = GetWindowLongA(hWnd, GWL_STYLE);

#define WS_BORDERLESS	(WS_POPUP | WS_VISIBLE)
#define WS_WINDOWED		((0x02CA0000 | WS_VISIBLE) /*& ~WS_MINIMIZEBOX*/)

	uFlags = SWP_NOZORDER | SWP_NOACTIVATE;
	int x = 0, y = 0;
	// reached full screen (height)
	if (sy == height)
	{
		borderless = 1;
		dwStyle = WS_BORDERLESS;
		SetRect(&rc, 0, 0, sx, sy);
	}
	// reached full screeen (width)
	else if (sx == width)
	{
		borderless = 2;
		dwStyle = WS_BORDERLESS;
		SetRect(&rc, 0, 0, sx, sy);
	}
	else
	{
		borderless = 0;
		dwStyle = WS_WINDOWED;
		SetRect(&rc, 0, 0, width, height);
		//uFlags |= SWP_NOMOVE;

		x = 0;// (gfx_main.DesktopW - rc.right) / 2;
		y = 0;//(gfx_main.hMon.GetWorkHeight() - rc.bottom) / 2;
	}
	SetWindowLongA(hWnd, GWL_STYLE, dwStyle);

	AdjustWindowRectEx(&rc, dwStyle, menu, 0);
	SetWindowPos(hWnd, HWND_TOP,       x, y, rc.right - rc.left, rc.bottom - rc.top, uFlags);
	SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	// generate rendering rectangle
	if (lpResRect)
	{
		int center;
		switch (borderless)
		{
		case 0:	// still windowed
			p0.y = 0;
			p0.x = 0;
			ClientToScreen(hWnd, &p0);
			p1.x = width;
			p1.y = height;
			ClientToScreen(hWnd, &p1);
			SetRect(lpResRect, p0.x, p0.y, p1.x, p1.y);
			break;
		case 1:	// borderless vertical
			center = (sx - width) / 2;
			SetRect(lpResRect, center, 0, center + width, height);
			break;
		case 2:	// borderless horizontal
			center = (sy - height) / 2;
			SetRect(lpResRect, 0, center, width, center + height);
			break;
		}
	}
	InvalidateRect(hWnd, nullptr, true);
}

HRESULT IDirect3D8Proxy::QueryInterface(REFIID riid, void** ppvObj) { return d3d8->QueryInterface(riid, ppvObj); }
ULONG IDirect3D8Proxy::AddRef() { return d3d8->AddRef(); }
ULONG IDirect3D8Proxy::Release()
{
	auto hr = d3d8->Release();
	delete this;
	return hr;
}
HRESULT IDirect3D8Proxy::RegisterSoftwareDevice(void* pInitializeFunction) { return d3d8->RegisterSoftwareDevice(pInitializeFunction); }
UINT IDirect3D8Proxy::GetAdapterCount() { return d3d8->GetAdapterCount(); }
HRESULT IDirect3D8Proxy::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8* pIdentifier) { return d3d8->GetAdapterIdentifier(Adapter, Flags, pIdentifier); }
UINT IDirect3D8Proxy::GetAdapterModeCount(UINT Adapter) { return d3d8->GetAdapterModeCount(Adapter); }
HRESULT IDirect3D8Proxy::EnumAdapterModes(UINT Adapter, UINT Mode, D3DDISPLAYMODE* pMode) { return d3d8->EnumAdapterModes(Adapter, Mode, pMode); }
HRESULT IDirect3D8Proxy::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode) { return d3d8->GetAdapterDisplayMode(Adapter, pMode); }
HRESULT IDirect3D8Proxy::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed)
{ return d3d8->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed); }
HRESULT IDirect3D8Proxy::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{ return d3d8->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat); }
HRESULT IDirect3D8Proxy::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType)
{ return d3d8->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType); }
HRESULT IDirect3D8Proxy::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{ return d3d8->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat); }
HRESULT IDirect3D8Proxy::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8* pCaps) { return d3d8->GetDeviceCaps(Adapter, DeviceType, pCaps); }
HMONITOR IDirect3D8Proxy::GetAdapterMonitor(UINT Adapter) { return d3d8->GetAdapterMonitor(Adapter); }
HRESULT IDirect3D8Proxy::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface)
{
	pPresentationParameters->Windowed = true;
	auto hr = d3d8->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	mainWindow = hFocusWindow;
	Gfx_resize_window(mainWindow, pPresentationParameters->BackBufferWidth,
		pPresentationParameters->BackBufferHeight, nullptr);
	
	d3d8dev = new IDirect3DDevice8Proxy();
	d3d8dev->d3d8 = *ppReturnedDeviceInterface;
	*ppReturnedDeviceInterface = (IDirect3DDevice8*)d3d8dev;

	return hr;
}

HRESULT IDirect3DDevice8Proxy::QueryInterface(REFIID riid, void** ppvObj) {	return d3d8->QueryInterface(riid, ppvObj); }
ULONG IDirect3DDevice8Proxy::AddRef() {	return d3d8->AddRef(); }
ULONG IDirect3DDevice8Proxy::Release()
{
	auto hr = d3d8->Release();
	delete this;
	return hr;
}
HRESULT IDirect3DDevice8Proxy::TestCooperativeLevel()
{ return d3d8->TestCooperativeLevel(); }
UINT IDirect3DDevice8Proxy::GetAvailableTextureMem()
{ return d3d8->GetAvailableTextureMem(); }
HRESULT IDirect3DDevice8Proxy::ResourceManagerDiscardBytes(DWORD Bytes)
{ return d3d8->ResourceManagerDiscardBytes(Bytes); }
HRESULT IDirect3DDevice8Proxy::GetDirect3D(IDirect3D8** ppD3D8)
{ return d3d8->GetDirect3D(ppD3D8); }
HRESULT IDirect3DDevice8Proxy::GetDeviceCaps(D3DCAPS8* pCaps)
{ return d3d8->GetDeviceCaps(pCaps); }
HRESULT IDirect3DDevice8Proxy::GetDisplayMode(D3DDISPLAYMODE* pMode)
{ return d3d8->GetDisplayMode(pMode); }
HRESULT IDirect3DDevice8Proxy::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters)
{ return d3d8->GetCreationParameters(pParameters); }
HRESULT IDirect3DDevice8Proxy::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface8* pCursorBitmap)
{
	return d3d8->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}
void IDirect3DDevice8Proxy::SetCursorPosition(int X, int Y, DWORD Flags)
{
	d3d8->SetCursorPosition(X, Y, Flags);
}
BOOL IDirect3DDevice8Proxy::ShowCursor(BOOL bShow)
{
	return d3d8->ShowCursor(bShow);
}
HRESULT IDirect3DDevice8Proxy::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain8** pSwapChain)
{ return d3d8->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain); }
HRESULT IDirect3DDevice8Proxy::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	Gfx_resize_window(mainWindow, pPresentationParameters->BackBufferWidth,
		pPresentationParameters->BackBufferHeight, nullptr);
	return d3d8->Reset(pPresentationParameters);
}
HRESULT IDirect3DDevice8Proxy::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{ return d3d8->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion); }
HRESULT IDirect3DDevice8Proxy::GetBackBuffer(UINT BackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8** ppBackBuffer)
{ return d3d8->GetBackBuffer(BackBuffer, Type, ppBackBuffer); }
HRESULT IDirect3DDevice8Proxy::GetRasterStatus(D3DRASTER_STATUS* pRasterStatus)
{ return d3d8->GetRasterStatus(pRasterStatus); }
void IDirect3DDevice8Proxy::SetGammaRamp(DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{ d3d8->SetGammaRamp(Flags, pRamp); }
void IDirect3DDevice8Proxy::GetGammaRamp(D3DGAMMARAMP* pRamp)
{ d3d8->GetGammaRamp(pRamp); }
HRESULT IDirect3DDevice8Proxy::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8** ppTexture)
{ return d3d8->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture); }
HRESULT IDirect3DDevice8Proxy::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture8** ppVolumeTexture)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8** ppCubeTexture)
{ return d3d8->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture); }
HRESULT IDirect3DDevice8Proxy::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8** ppVertexBuffer)
{ return d3d8->CreateVertexBuffer(Length,Usage, FVF, Pool, ppVertexBuffer); }
HRESULT IDirect3DDevice8Proxy::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8** ppIndexBuffer)
{ return d3d8->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer); }
HRESULT IDirect3DDevice8Proxy::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, IDirect3DSurface8** ppSurface)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::CreateImageSurface(UINT Width, UINT Height, D3DFORMAT Format, IDirect3DSurface8** ppSurface)
{ return d3d8->CreateImageSurface(Width, Height, Format, ppSurface); }
HRESULT IDirect3DDevice8Proxy::CopyRects(IDirect3DSurface8* pSourceSurface, CONST RECT* pSourceRectsArray, UINT cRects, IDirect3DSurface8* pDestinationSurface, CONST POINT* pDestPointsArray)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::UpdateTexture(IDirect3DBaseTexture8* pSourceTexture, IDirect3DBaseTexture8* pDestinationTexture)
{ return d3d8->UpdateTexture(pSourceTexture, pDestinationTexture); }
HRESULT IDirect3DDevice8Proxy::GetFrontBuffer(IDirect3DSurface8* pDestSurface)
{
#if 0
	D3DLOCKED_RECT r0, r1;
	pDestSurface->LockRect(&r0, 0, 0);
	D3DSURFACE_DESC desc;
	pDestSurface->GetDesc(&desc);

	IDirect3DSurface8* pSrcSurface;
	d3d8->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pSrcSurface);
	pSrcSurface->LockRect(&r1, 0, 0);

	INT pitch = min(r0.Pitch, r1.Pitch);
	BYTE *row0 = (BYTE*)r0.pBits, *row1 = (BYTE*)r1.pBits;
	for (int y = 0; y < desc.Height; y++, row0 += r0.Pitch, row1 += r1.Pitch)
		memcpy(row0, row1, pitch);

	pDestSurface->UnlockRect();
	pSrcSurface->UnlockRect();
	pSrcSurface->Release();

	return S_OK;
#else
	return d3d8->GetFrontBuffer(pDestSurface);
#endif
}
HRESULT IDirect3DDevice8Proxy::SetRenderTarget(IDirect3DSurface8* pRenderTarget, IDirect3DSurface8* pNewZStencil)
{ return d3d8->SetRenderTarget(pRenderTarget, pNewZStencil); }
HRESULT IDirect3DDevice8Proxy::GetRenderTarget(IDirect3DSurface8** ppRenderTarget)
{ return d3d8->GetRenderTarget(ppRenderTarget); }
HRESULT IDirect3DDevice8Proxy::GetDepthStencilSurface(IDirect3DSurface8** ppZStencilSurface)
{ return d3d8->GetDepthStencilSurface(ppZStencilSurface); }
HRESULT IDirect3DDevice8Proxy::BeginScene()
{ return d3d8->BeginScene(); }
HRESULT IDirect3DDevice8Proxy::EndScene()
{ return d3d8->EndScene(); }
HRESULT IDirect3DDevice8Proxy::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{ return d3d8->Clear(Count, pRects, Flags, Color, Z, Stencil); }
HRESULT IDirect3DDevice8Proxy::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{ return d3d8->SetTransform(State, pMatrix); }
HRESULT IDirect3DDevice8Proxy::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
{ return d3d8->GetTransform(State, pMatrix); }
HRESULT IDirect3DDevice8Proxy::MultiplyTransform(D3DTRANSFORMSTATETYPE a, CONST D3DMATRIX* b)
{ return d3d8->MultiplyTransform(a, b); }
HRESULT IDirect3DDevice8Proxy::SetViewport(CONST D3DVIEWPORT8* pViewport)
{ return d3d8->SetViewport(pViewport); }
HRESULT IDirect3DDevice8Proxy::GetViewport(D3DVIEWPORT8* pViewport)
{ return d3d8->GetViewport(pViewport); }
HRESULT IDirect3DDevice8Proxy::SetMaterial(CONST D3DMATERIAL8* pMaterial)
{ return d3d8->SetMaterial(pMaterial); }
HRESULT IDirect3DDevice8Proxy::GetMaterial(D3DMATERIAL8* pMaterial)
{ return d3d8->GetMaterial(pMaterial); }
HRESULT IDirect3DDevice8Proxy::SetLight(DWORD Index, CONST D3DLIGHT8*)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::GetLight(DWORD Index, D3DLIGHT8*)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::LightEnable(DWORD Index, BOOL Enable)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::GetLightEnable(DWORD Index, BOOL* pEnable)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::SetClipPlane(DWORD Index, CONST float* pPlane)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::GetClipPlane(DWORD Index, float* pPlane)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{ return d3d8->SetRenderState(State, Value); }
HRESULT IDirect3DDevice8Proxy::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{ return d3d8->GetRenderState(State, pValue); }
HRESULT IDirect3DDevice8Proxy::BeginStateBlock()
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::EndStateBlock(DWORD* pToken)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::ApplyStateBlock(DWORD Token)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::CaptureStateBlock(DWORD Token)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::DeleteStateBlock(DWORD Token)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::CreateStateBlock(D3DSTATEBLOCKTYPE Type, DWORD* pToken)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::SetClipStatus(CONST D3DCLIPSTATUS8* pClipStatus)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::GetClipStatus(D3DCLIPSTATUS8* pClipStatus)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::GetTexture(DWORD Stage, IDirect3DBaseTexture8** ppTexture)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::SetTexture(DWORD Stage, IDirect3DBaseTexture8* pTexture)
{ return d3d8->SetTexture(Stage, pTexture); }
HRESULT IDirect3DDevice8Proxy::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{ return d3d8->GetTextureStageState(Stage, Type, pValue); }
HRESULT IDirect3DDevice8Proxy::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{ return d3d8->SetTextureStageState(Stage, Type, Value); }
HRESULT IDirect3DDevice8Proxy::ValidateDevice(DWORD* pNumPasses)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::GetInfo(DWORD DevInfoID, void* pDevInfoStruct, DWORD DevInfoStructSize)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::SetCurrentTexturePalette(UINT PaletteNumber)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::GetCurrentTexturePalette(UINT* PaletteNumber)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{ return d3d8->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount); }
HRESULT IDirect3DDevice8Proxy::DrawIndexedPrimitive(D3DPRIMITIVETYPE p, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{ return d3d8->DrawIndexedPrimitive(p, minIndex, NumVertices, startIndex, primCount); }
HRESULT IDirect3DDevice8Proxy::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{ return d3d8->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride); }
HRESULT IDirect3DDevice8Proxy::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer8* pDestBuffer, DWORD Flags)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::CreateVertexShader(CONST DWORD* pDeclaration, CONST DWORD* pFunction, DWORD* pHandle, DWORD Usage)
{ return d3d8->CreateVertexShader(pDeclaration, pFunction, pHandle, Usage); }
HRESULT IDirect3DDevice8Proxy::SetVertexShader(DWORD Handle)
{ return d3d8->SetVertexShader(Handle); }
HRESULT IDirect3DDevice8Proxy::GetVertexShader(DWORD* pHandle)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::DeleteVertexShader(DWORD Handle)
{ return d3d8->DeleteVertexShader(Handle); }
HRESULT IDirect3DDevice8Proxy::SetVertexShaderConstant(DWORD Register, CONST void* pConstantData, DWORD ConstantCount)
{ return d3d8->SetVertexShaderConstant(Register, pConstantData, ConstantCount); }
HRESULT IDirect3DDevice8Proxy::GetVertexShaderConstant(DWORD Register, void* pConstantData, DWORD ConstantCount)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::GetVertexShaderDeclaration(DWORD Handle, void* pData, DWORD* pSizeOfData)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::GetVertexShaderFunction(DWORD Handle, void* pData, DWORD* pSizeOfData)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8* pStreamData, UINT Stride)
{ return d3d8->SetStreamSource(StreamNumber, pStreamData, Stride); }
HRESULT IDirect3DDevice8Proxy::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8** ppStreamData, UINT* pStride)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::SetIndices(IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex)
{ return d3d8->SetIndices(pIndexData, BaseVertexIndex); }
HRESULT IDirect3DDevice8Proxy::GetIndices(IDirect3DIndexBuffer8** ppIndexData, UINT* pBaseVertexIndex)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::CreatePixelShader(CONST DWORD* pFunction, DWORD* pHandle)
{ return d3d8->CreatePixelShader(pFunction, pHandle); }
HRESULT IDirect3DDevice8Proxy::SetPixelShader(DWORD Handle)
{ return d3d8->SetPixelShader(Handle); }
HRESULT IDirect3DDevice8Proxy::GetPixelShader(DWORD* pHandle)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::DeletePixelShader(DWORD Handle)
{ return d3d8->DeletePixelShader(Handle); }
HRESULT IDirect3DDevice8Proxy::SetPixelShaderConstant(DWORD Register, CONST void* pConstantData, DWORD ConstantCount)
{ return d3d8->SetPixelShaderConstant(Register, pConstantData, ConstantCount); }
HRESULT IDirect3DDevice8Proxy::GetPixelShaderConstant(DWORD Register, void* pConstantData, DWORD ConstantCount)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::GetPixelShaderFunction(DWORD Handle, void* pData, DWORD* pSizeOfData)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{ return S_OK; }
HRESULT IDirect3DDevice8Proxy::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{ return d3d8->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo); }
HRESULT IDirect3DDevice8Proxy::DeletePatch(UINT Handle)
{ return d3d8->DeletePatch(Handle); }
