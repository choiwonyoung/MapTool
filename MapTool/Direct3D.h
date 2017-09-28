#pragma once




struct PREVIEWVERTEX
{
	D3DXVECTOR4 p;
	D3DXVECTOR3 t;

	/*
		D3DFVF_DIFFUSE : 정점 포맷이 diffuse 색 성분을 포함한다.

		D3DFVF_NORMAL : 정점 포맷이 정점 법선 벡터를 포함한다.

		D3DFVF_SPECULAR : 정점 포맷이 specular 색 성분을 포함한다.

		D3DFVF_XYZ : 정점 포맷이 변환되지 않는 정점의 위치 좌표를 포함한다.
		이 플래그는 보통 D3DFVF_NORMAL 플래그와 같이 사용되며 D3DFVF_XYZRHW 와 같이 사용할 수 없다. (3D 좌표)

		D3DFVF_XYZRHW : 정점 포맷이 변환된 정점의 위치 좌표를 포함한다. (2D 나 Pixel 좌표)
						RHW ( reciprocal homogeneous w : 동치 좌표 'w의 역수) , 1/w 
						-> 이것을 사용하는 이유 : perspective 변환 행렬을 거치면 나오는 행렬값을 우리가 원하는 동치좌표로 바꾸기 위해 w로 4번째 성분을 나눠야 하는데 , 이것의 역수를 미리 정의해놓은것이 rhw 라는 얘기가 됨
						파이프 라인을 이미 처리된 정보로 간주하므로  , 2D룰 바로 사용하기에 용이
						
		D3DFVF_XYZW : 정점 포맷이 변환 및 클리핑된 x,y,z,w데이터를 포함한다.
					  ProcessVetices는 클리퍼를 기동하지 않고 , 데이터를 클립좌표로 출력한다.
					  이 정수는 프로그래밍 할 수 있는 정점 파이프 라인에서의 사용을 목적으로 
					  한 정수로 프로그래밍 할 수 있는 정점 파이프 라인으로 밖에 사용하지 않는다.
	*/
	enum { FVF = D3DFVF_XYZRHW | D3DFVF_TEX1 };
};
class Direct3D
{
//////////////////////////////////////////////////////////////////////////
// 함수
//////////////////////////////////////////////////////////////////////////
public:
	Direct3D();
	~Direct3D();

	LPDIRECT3DDEVICE9	GetDevice() { return m_pDevice; }

	HRESULT				Init( HWND hWnd , int width , int height );
	void				SetUpFog( float , float , float density = 0.0f );

	// 텍스쳐 미리보기와 관련된 함수
	HRESULT				InitPreview( HWND hWnd , int width , int height );
	HRESULT				LoadPreviewTexture( const WCHAR* texName );
	void				ReleasePreview();
	void				RenderPreview();

private:
	void				_SetUpMatrix( int width , int height );
	void				_SetUpLight();

	void				_InitPreviewVertex( int width , int height );
	void				_DrawPreviewTexture();

//////////////////////////////////////////////////////////////////////////
// 변수
//////////////////////////////////////////////////////////////////////////
public:
	LPDIRECT3DDEVICE9	m_pDevice_Preview;

private:
	LPDIRECT3D9			m_pD3D;
	LPDIRECT3DDEVICE9	m_pDevice;

	LPDIRECT3DTEXTURE9	m_pTex;
	PREVIEWVERTEX		m_pPreview[4];			// 미리 보기 화면을 위해서 


};

