#pragma once

#include "Direct3D.h"
#include "Camera.h"
#include "QuadTree.h"
#include "D3DFont.h"

enum MODE
{
	EDIT_HEIGHT = 0,
	EDIT_TEXTURE,
	EDIT_OBJECT,
};

// Input 구조체
struct UserInput
{
	// 마우스 관련
	BOOL			m_bLButtonDown;		// 마우스 왼쪽 버튼
	BOOL			m_bRButtonDown;		// 마우스 오른쪽 버튼
	BOOL			m_bLCtrlKeyDown;	// 키보드 왼쪽 컨트롤 키 다운
	BOOL			m_bLAltKeyDown;		// 키보드 왼쪽 알트 키 다운
	BOOL			m_bLShiftKeyDown;	// 키보드 왼쪽 쉬프트 키 다운

	// Delta 관련
	FLOAT			m_fMouseMoveDeltaX;	// x축 움직임
	FLOAT			m_fMouseMoveDeltaY;	// y축 움직임
	FLOAT			m_fMouseWheelDelta;	// 휠 움직임

	FLOAT			m_fMouseSensitive;	// 마우스 감도
	D3DXVECTOR3		m_vMousePos;		// 마우스 위치

	UserInput() { m_fMouseSensitive = 0.2f; }
};

// 툴에 보이는 축 라인
struct AxisLine
{
	D3DXVECTOR3 p;
	DWORD color;

	enum { FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE };
};
//////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////
class ToolManager
{
public:
	ToolManager();
	~ToolManager();

	const Direct3D* GetDirect3D()	{ return m_pDirect3D; }
	const Camera*	GetCamera()		{ return m_pCamera; }

	HRESULT Init( HWND hWnd , HINSTANCE hInst , int width , int height );
	void CameraSetting();
	void CreateMap();
	void DeleteMap();
	void InitTexturePreview();
	void CountFPS();
	void DrawAxisLine();
	void FrameMove( float deltaTime );
	void Render();
	
private:
	void UpdateInput( float timeDelta );
	void UpdateCamera();

private:
	LPDIRECT3DDEVICE9	m_pDevice;
	QuadTree*			m_pQuadTree;
	D3DFont*			m_pFont;

	// User Input
	UserInput			m_UserInput;

	// FPS
	float				m_fps;
	int					m_nFrameCount;
	LARGE_INTEGER		m_frequency;				// 타이머 빈도수를 저장
	LARGE_INTEGER		m_startTime;				// 프레임 카운트가 0일때 시간이 저장되는 변수

	Direct3D*			m_pDirect3D;
	Camera*				m_pCamera;

	int					m_nWidth;
	int					m_nHeight;

	// 탭 선택
	int					m_nEditMode;				// 지형 , 텍스쳐 , 오브젝트

	// 브러쉬
	BOOL				m_bPicking;					// 피킹을 할 건지 체크
	BOOL				m_bBrushInView;				// 마우스가 VIEW 안에 있는지 체크

	// 카메라 , 마우스
	int					m_nCameraMove;

	// 랜더링 옵션
	BOOL				m_bCoordinateAxis;			// 좌표축

	// 미리보기
	// 미리보기 정보는 CONTROLVIEW 안의 텍스쳐 다이알 로그에서 가져온다.
	HWND				m_hWnd_Preview;				// 텍스처의 미리보기 핸들
	int					m_nWidth_Preview;			// 텍스처의 미리보기 가로크기
	int					m_nHeight_Preview;			// 텍스처의 미리보기 세로크기
};

