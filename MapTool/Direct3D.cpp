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
	m_pDevice->SetRenderState( D3DRS_LIGHTING , FALSE );

	_SetUpMatrix( width , height );
	_SetUpLight();
	SetUpFog( 900.0f , 950.0f );

	return S_OK;
}

void Direct3D::SetUpFog( float fStart , float fEnd, float density /*= 0.0f */ )
{

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


