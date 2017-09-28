#include "stdafx.h"
#include "Direct3D.h"


Direct3D::Direct3D()
	: m_pD3D(NULL)
	, m_pDevice(NULL)
	, m_pDevice_Preview(NULL)
	, m_pTex(NULL)
{
	ZeroMemory(m_pPreview , sizeof(m_pPreview));
}


Direct3D::~Direct3D()
{
	SAFE_RELEASE( m_pTex );
	SAFE_RELEASE( m_pDevice_Preview );
	SAFE_RELEASE( m_pDevice );
	SAFE_RELEASE( m_pD3D );
}

/*
	DirectX 를 초기화 하는 과정
	1. D3D 생성
	2. Present Parameter 설정
	3. D3Device 생성
	4. 렌더 상태 설정
*/
HRESULT Direct3D::Init( HWND hWnd , int width , int height )
{
	if( NULL == ( m_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return E_FAIL;

	// 장치에 대한 고유 정보를 가져온다.
	D3DCAPS9 caps;
	m_pD3D->GetDeviceCaps( 
		D3DADAPTER_DEFAULT ,		// 디스플레이 어댑터를 나타내는 서수 , D3DADAPTER_DEFAULT is always the primary display adapter.
		D3DDEVTYPE_HAL ,			// D3DDEVTYPE 열거형 멤버 , 장치의 종류를 나타낸다.
		&caps						// 장치의 기술 정보 저장
		);
	
	int vp = 0;
	if( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )		// HardWare로 변환 및 라이팅이 지원되는지 검사
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// 현재 DisplayMode 를 가져온다.
	D3DDISPLAYMODE d3ddm;
	if( FAILED( m_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT , &d3ddm ) ) )
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp , sizeof( d3dpp ) );

	d3dpp.BackBufferCount		= 1;
	d3dpp.Windowed				= TRUE;
	/*
		Present를 2가지 방식으로 구분
		1. 백버퍼의 내용이 유지됨을 보장해 주는 방식
		2. 백버퍼의 내용이 변경될 수 있는 방식
		D3DSWAPEFFECT_DISCARD 는 Present 할때 백버퍼의 내용이 변경되어도 상관 없다고 알려줌으로서 보다 효율 적인 페이지 플리핑을 가능하게 해줌

		이것을 쓰지 않으면 더블 버퍼링 하더라도 내부적으로는 현재 백버퍼의(0) 내용을 새로운 버퍼(1)에 복사하는 작업 수행

		D3DSWAPEFFCT_COPY/D3DSWAPEFFECT_FLIP
		Present를 2가지로 구분
		1. 백버퍼의 내용을 프론트 버퍼에 복사(COPY)하는 방식(백버퍼 고정 , 단일 백버퍼 전용)		- D3DSWAPEFFECT_COPY
		2. (다중)백버퍼+주소변경을 사용하여 백버퍼가 돌아가며 프론트 버퍼가 되는 방식			- D3DSWAPEFFECT_FLIP

		FLIP을 하더라도 DISCARD를 쓰지 않으면 백버퍼의 내용이 보존되어야 하기때문에 복사를 하게 됩니다.

	*/
	d3dpp.SwapEffect			= D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferWidth		= width;
	d3dpp.BackBufferHeight		= height;
	d3dpp.BackBufferFormat		= d3ddm.Format;
	d3dpp.hDeviceWindow			= hWnd;

	/*
		EnableAutoDepthStencil 이 TRUE 이면 깊이 버퍼를 관리하도록 설정
		AutoDepthStencilFormat은 유효한 깊이 버퍼 포맷으로 설정하는 것 D3DFMT_16은 16비트 깊이 버퍼를 사용할 수 있는 경유에 지정

	*/
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;					// 24 bit Depth / 8 Bit Stencil

	/*
		D3DPRESENT_INTERVAL_DEFAULT : DirectX 가 적절한 값을 선택하여 렌더링 간격을 설정
		D3DPRESENT_INTERVAL_IMMEDIATE : 항상 렌더링 한다.
		D3DPRESENT_INTERVAL_ONE : 모니터 갱신이 1번 이루어지는 시간에 렌더링 한다.
		....
	*/
	d3dpp.PresentationInterval	 = D3DPRESENT_INTERVAL_IMMEDIATE;	// Adapter가 Refresh 하는 동안 즉시 Update하도록 설정

	// Device 설정
	if( FAILED( m_pD3D->CreateDevice(
		D3DADAPTER_DEFAULT ,	// 디스플레이 어댑터 , D3DADAPTER_DEFAULT는 항상 1차 디스플레이 어댑터
		D3DDEVTYPE_HAL ,		// 목적의 장치 타입 (하드웨어 타입)
		hWnd ,					// 현재 만든 윈도우 핸들
		vp ,					// 장치 생성을 제어하는 1개 또는 복수 개의 옵션정보
		&d3dpp ,				// D3DPRESENT_PARAMETERS 정보
		&m_pDevice				//얻어오는 디바이스
		) ) )
	{
		return E_FAIL;
	}

	m_pDevice->SetRenderState( D3DRS_ZENABLE , TRUE );
	m_pDevice->SetRenderState( D3DRS_CULLMODE , D3DCULL_CCW );		// 폴리곤 뒷면 제거 설정 플래그 , D3DCULL_CCW(Counter Clock Wise): 시계 반대 방향으로 정점을 회전시킨 면을 뒷면으로 인식
	m_pDevice->SetRenderState( D3DRS_LIGHTING , FALSE );			// Light 설정 전까지는 Light 설정을 꺼둔다.

	_SetUpMatrix( width , height );
	_SetUpLight();
	SetUpFog( 900.0f , 950.0f );

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// 포그 설정도 쉐이더로 대체될수 있다.
//////////////////////////////////////////////////////////////////////////
void Direct3D::SetUpFog( float fStart , float fEnd, float density /*= 0.0f */ )
{
	/*
		density 값이 있으면 , D3DFOG_EXP 로 사용가능하고
		없다면 D3DFOG_LINEAR 로 설정해서 사용해야 함
	*/
	if( density )
	{
		m_pDevice->SetRenderState( D3DRS_FOGENABLE , TRUE );
		m_pDevice->SetRenderState( D3DRS_FOGCOLOR , D3DCOLOR_XRGB( 120 , 180 , 255 ) );
		m_pDevice->SetRenderState( D3DRS_FOGSTART , *(DWORD*)&fStart );
		m_pDevice->SetRenderState( D3DRS_FOGEND , *(LPDWORD)&fEnd );
		m_pDevice->SetRenderState( D3DRS_FOGDENSITY , *(LPDWORD)&density );
		m_pDevice->SetRenderState( D3DRS_FOGTABLEMODE , D3DFOG_EXP );
	}
	else
	{
		m_pDevice->SetRenderState( D3DRS_FOGENABLE , TRUE );
		m_pDevice->SetRenderState( D3DRS_FOGCOLOR , D3DCOLOR_XRGB( 120 , 180 , 255 ) );
		m_pDevice->SetRenderState( D3DRS_FOGSTART , *(LPDWORD)&fStart );
		m_pDevice->SetRenderState( D3DRS_FOGEND , *(LPDWORD)&fEnd );
		m_pDevice->SetRenderState( D3DRS_FOGVERTEXMODE , D3DFOG_LINEAR );
	}
}

void Direct3D::_SetUpMatrix( int width , int height )
{
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity( &matWorld );
	m_pDevice->SetTransform( D3DTS_WORLD , &matWorld );
		
	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH( 
		&matWorld ,					
		D3DX_PI / 4.0f ,				// y 방향의 시야(라디안 단위)
		(float)width / (float)height ,  // 가로 세로 비율 , 뷰 공간의 높이를 폭으로 나눈 값
		1.0f ,							// 가까운 뷰 평면의 Z 값
		2000.0f							// 먼 뷰 평면의 Z 값
		);

	m_pDevice->SetTransform( D3DTS_PROJECTION , &matProj );
}

void Direct3D::_SetUpLight()
{
	// Material 설정
	D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl , sizeof( mtrl ) );
	mtrl.Ambient	= D3DXCOLOR( 1.0f , 1.0f , 1.0f , 1.0f );
	mtrl.Diffuse	= D3DXCOLOR( 1.0f , 1.0f , 1.0f , 1.0f );
	mtrl.Specular	= D3DXCOLOR( 1.0f , 1.0f , 1.0f , 1.0f );
	m_pDevice->SetMaterial( &mtrl );

	// Light 설정
	D3DLIGHT9 light;
	ZeroMemory(&light , sizeof(light));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse = D3DXCOLOR( 1.0f , 1.0f , 1.0f , 1.0f );

	D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction , &D3DXVECTOR3( 1.0f , -1.0f , 1.0f ) );
	m_pDevice->SetLight( 0 , &light );
	m_pDevice->LightEnable( 0 , TRUE );
	m_pDevice->SetRenderState( D3DRS_LIGHTING , TRUE );
	m_pDevice->SetRenderState( D3DRS_AMBIENT , 0x00bfbfbf );			// 환경광원 값 설정
}


HRESULT Direct3D::InitPreview( HWND hWnd , int width , int height )
{
	D3DDISPLAYMODE d3ddm;
	if( FAILED( m_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT , &d3ddm ) ) )
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpm;
	ZeroMemory( &d3dpm , sizeof( d3dpm ) );

	d3dpm.BackBufferCount			= 1;
	d3dpm.Windowed					= TRUE;
	d3dpm.SwapEffect				= D3DSWAPEFFECT_DISCARD;
	d3dpm.BackBufferWidth			= width;
	d3dpm.BackBufferHeight			= height;
	d3dpm.BackBufferFormat			= d3ddm.Format;
	d3dpm.hDeviceWindow				= hWnd;
	d3dpm.EnableAutoDepthStencil	= TRUE;
	d3dpm.AutoDepthStencilFormat	= D3DFMT_D24S8;

	//
	if( FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT , D3DDEVTYPE_HAL , hWnd , D3DCREATE_SOFTWARE_VERTEXPROCESSING , &d3dpm , &m_pDevice_Preview)))
		return E_FAIL;

	_InitPreviewVertex( width , height );

	/*
		텍스쳐는 스크린 좌표에서 삼각형에 입혀진다.
		텍스쳐 : 픽셀의 매트릭스이며 표면과 비슷하지만 삼각형에 입힐 수 있음
		텍스쳐 매핑 : 이미지 데이터를 삼각형에 입힐 수 있게 하는 기술 , 스크린 스페이스에서 입혀짐

		일반적으로 텍스쳐의 삼각형과 스크린의 삼각형의 크기는 서로다름
		텍스쳐의 삼각형이 스크린의 삼각형보다 작은 경우 텍스쳐의 삼각형을 확대하여 크기를 맞추고,
		텍스쳐의 삼각형이 스크린의 삼각형보다 큰 경우 텍스쳐의 삼각형을 축소하여 크기를 맞춤
		이 두가지 경우 모두 얼마간의 왜곡 현상이 발생하는데, 필터링을 통해 왜곡현상을 줄이고 , 부드럽게 이미지를 만들어 내기 위해 SetSampleState를 이용 할 수 있음
	*/
	m_pDevice_Preview->SetSamplerState( 
		0 ,						// 텍스쳐 스테이지
		D3DSAMP_MINFILTER ,		// 필터 타입 (축소 필터 방식)
		D3DTEXF_LINEAR );		// 필터링 방식
	m_pDevice_Preview->SetSamplerState(
		0 , 
		D3DSAMP_MAGFILTER ,		// 확대 필터 방식
		D3DTEXF_LINEAR 
		);
	m_pDevice_Preview->SetSamplerState( 
		0 , 
		D3DSAMP_MIPFILTER ,		// 밉맵 레벨간 사용 필터
		D3DTEXF_LINEAR 
		);

	m_pDevice_Preview->SetRenderState( D3DRS_CULLMODE , D3DCULL_CCW );
	m_pDevice_Preview->SetRenderState( D3DRS_LIGHTING , FALSE );

	return S_OK;
}

HRESULT Direct3D::LoadPreviewTexture( const WCHAR* texName )
{
	SAFE_RELEASE( m_pTex );

	if( FAILED( D3DXCreateTextureFromFileEx(
		m_pDevice ,					// IDirective3DDevice9에 대한 포인터
		texName ,					// 텍스쳐 파일명
		D3DX_DEFAULT ,				// 폭과 0인 경우 파일로부터 취득 (2의 승수이면 DEFAULT를 썼을때 제대로 된 값으로 들어가고 그렇지 않으면 실제 이미지의 가로 크기를 넣어주면 원하는 크기로 나옴)
		D3DX_DEFAULT ,				// 높이 0인 경우 파일로부터 취득 (2의 승수이면 DEFAULT를 썼을때 제대로 된 값으로 들어가고 그렇지 않으면 실제 이미지의 가로 크기를 넣어주면 원하는 크기로 나옴)
		D3DX_DEFAULT ,				// 밉맵레벨의 수가 0인 경우 완전한 밉맵 체인이 생성
		0 ,							// 사용 상태 값을 플래그로 설정( D3DUSAGE_RENDERTARGET(표면이 렌더링 대상) , D3DUSAGE_DYNAMIC(표면을 동적처리) )
		D3DFMT_UNKNOWN ,			// D3DFORMAT 열거형 (D3DFMT_UNKNOWN은 포멧을 파일에서 가져옴 , D3DFMT_FROM_FILE 경우 파일에 있는 그대로의 형식을 가지고 있음 )
		D3DPOOL_DEFAULT ,			// D3DPOOL 열거형
		D3DX_DEFAULT ,				// D3DX_FILTER로 하나 이상의 이미지를 필터링 하는 방법을 제어하는 상수 , D3DX_DEFAULT를 지정하는 것은 D3DX_FILTER_TRIANGLE | D3DX_FILTER_DITHER 를 지정하는 것으로 동일
		D3DX_DEFAULT ,				// D3DX_FILTER의 하나 이상의 내용을 조합 , D3DX_DEFAULT를 지정하는 것은 D3DX_FILTER_BOX를 지정하는 것과 동일
		0 ,							// D3DCOLOR 색상 키 값
		NULL ,						// D3DXIMAGE_INFO의 포인터 
		NULL ,						// PALETTEENTRY를 기입하는 256 색 팔레트를 나타내는 구조
		&m_pTex						// 반환되는 Texture 포인터 값
		) ) )				
	{
		return E_FAIL;
	}
		

	return S_OK;
}

/*
	랜더링 순서
	1. Device Clear
	2. BeginScene , EndScene 사이에 Rendering 해야 할것을 넣음
	3. Present -> Front buffer에 Back Buffer 로 바꿔줌
*/
void Direct3D::RenderPreview()
{
	if( m_pDevice_Preview )
	{
		m_pDevice_Preview->Clear( 
			0L ,										// 2번째 인자인 pRect의 배열에 있는 직사각형 수 , pRect 를 NULL로 설정하는 경우는 , 이 파라미터를 0 으로 설정
			NULL ,										// D3DRECT 구조체의 배열의 포인터, 클리어 하는 직사각형을 기술 , NULL 로 설정하면 , 뷰포트 직사각형 전체를 클리어 하게 됨
			D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER ,		// 클리어하는 표면을 나타내는 플래그 , D3DCLEAR_STENCIL (스텐실 버퍼 클리어 , stencil 파라미터 값으로 함) , D3DCLEAR_TARGET (렌더링 타겟을 클리어 , Color 파라미터 색으로 함) , D3DCLEAR_ZBUFFER (깊이 버퍼 클리어 , Z 파라미터 값으로 함)
			0xffffffff ,								// 렌더 타겟 표면을 클리어 하는 32비트 ARGB 색값
			1.0f ,										// 이 매서드가 깊이 버퍼에 보존하는 새로운 z값(0.0~1.0) , 0은 뷰어에 가장 가까운 거리 , 1.0은 가장 먼 거리
			0L											// 각 스텐실 버퍼의 엔트리에 보존하는 정수값
			);

		if( SUCCEEDED( m_pDevice_Preview->BeginScene() ) )
		{
			_DrawPreviewTexture();
			m_pDevice_Preview->EndScene();
		}

		/*
			장치가 소유하는 백 버퍼의 순서 중에서 , 다음 버퍼의 컨텐츠를 프레젠테이션 한다.
		*/
		m_pDevice_Preview->Present(
			NULL ,		// 스왑 체인이 D3DSWAPEFFECT_COPY로 생성 되고 있는 경우를 제외해 , NULL를 지정해야 하는 값의 포인터 , NULL은 전송원 표면 저체가 표시
			NULL ,		// 스왑 체인이 D3DSWAPEFFECT_COPY로 생성 되고 있는 경우를 제외해 , NULL를 지정해야 하는 값의 포인터 , NULL은 클라이언트 영역 전체가 채워짐
			NULL ,		// 클라이언트 영역이 이 프레젠테이션의 대상으로 해 다루어지는 목적지 윈도우 포인터 , NULL 인 경우 , D3DPRESENT_PARAMETERS 의 hWndDeviceWindow 멤버가 사용됨
			NULL		// 스왑체인이 D3DSWAPEFFECT_COPY로 생성되지 않는 한, NULL이여야 함 , 
			);
	}

}
void Direct3D::ReleasePreview()
{
	SAFE_RELEASE( m_pTex );
	SAFE_RELEASE( m_pDevice_Preview );
}

void Direct3D::_InitPreviewVertex( int width , int height )
{
	::ZeroMemory( &m_pPreview , sizeof( m_pPreview ) );

	m_pPreview[0].p = D3DXVECTOR4( 0.0f			, 0.0f			, 0.5f , 1.0f );
	m_pPreview[1].p = D3DXVECTOR4( (float)width , 0.0f			, 0.5f , 1.0f );
	m_pPreview[2].p = D3DXVECTOR4( 0.0f			, (float)height , 0.5f , 1.0f );
	m_pPreview[3].p = D3DXVECTOR4( (float)width , (float)height , 0.5f , 1.0f );

	// uv 좌표
	m_pPreview[0].t.x = 0.0f;
	m_pPreview[0].t.y = 0.0f;
	m_pPreview[1].t.x = 1.0f;
	m_pPreview[1].t.y = 0.0f;
	m_pPreview[2].t.x = 0.0f;
	m_pPreview[2].t.y = 1.0f;
	m_pPreview[3].t.x = 1.0f;
	m_pPreview[3].t.y = 1.0f;
}

void Direct3D::_DrawPreviewTexture()
{
	// 해당 Device에서 뿌려지는 텍스쳐들에 대한 설정값
	m_pDevice_Preview->SetTextureStageState( 0 , D3DTSS_COLOROP , D3DTOP_MODULATE );
	m_pDevice_Preview->SetTextureStageState( 0 , D3DTSS_COLORARG1 , D3DTA_TEXTURE );
	m_pDevice_Preview->SetTextureStageState( 0 , D3DTSS_COLORARG2 , D3DTA_DIFFUSE );
	m_pDevice_Preview->SetTextureStageState( 0 , D3DTSS_ALPHAOP , D3DTOP_MODULATE );
	m_pDevice_Preview->SetTextureStageState( 0 , D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
	m_pDevice_Preview->SetTextureStageState( 0 , D3DTSS_ALPHAARG2 , D3DTA_DIFFUSE );

	m_pDevice_Preview->SetTexture( 0 , m_pTex );
	m_pDevice_Preview->SetFVF( PREVIEWVERTEX::FVF );
	/*
	DrawPrimitive의 업그레이드 버젼 , DrawPrimitive는 CreateVertexBuffer로 버텍스를 생성해야 하는데
	DrawPrimitiveUP은 버텍스 버퍼를 생성하지 않고 시스템 메모리의 정보만으로 폴리곤을 그릴 수 있다.
	하지만 , 시스템 메모리의 내용을 GPU에 써야하기 때문에 속도가 느림
	DrawPrimitiveUP 에서 UP 는 User Point를 의미함
	*/
	m_pDevice_Preview->DrawPrimitiveUP(
		D3DPT_TRIANGLESTRIP ,			// D3DPRIMITIVETYPE 열거형 멤버
		2 ,								// 렌더링 하는 기본도형의 수 , D3DCAPS9 구조체의 MaxPrimitiveCount 멤버를 체크해서 결정된다.
		m_pPreview ,					// 정점 데이터의 사용자 메모리 포인터
		sizeof( PREVIEWVERTEX )
		);
}






