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

// Input ����ü
struct UserInput
{
	// ���콺 ����
	BOOL			m_bLButtonDown;		// ���콺 ���� ��ư
	BOOL			m_bRButtonDown;		// ���콺 ������ ��ư
	BOOL			m_bLCtrlKeyDown;	// Ű���� ���� ��Ʈ�� Ű �ٿ�
	BOOL			m_bLAltKeyDown;		// Ű���� ���� ��Ʈ Ű �ٿ�
	BOOL			m_bLShiftKeyDown;	// Ű���� ���� ����Ʈ Ű �ٿ�

	// Delta ����
	FLOAT			m_fMouseMoveDeltaX;	// x�� ������
	FLOAT			m_fMouseMoveDeltaY;	// y�� ������
	FLOAT			m_fMouseWheelDelta;	// �� ������

	FLOAT			m_fMouseSensitive;	// ���콺 ����
	D3DXVECTOR3		m_vMousePos;		// ���콺 ��ġ

	UserInput() { m_fMouseSensitive = 0.2f; }
};

// ���� ���̴� �� ����
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
	LARGE_INTEGER		m_frequency;				// Ÿ�̸� �󵵼��� ����
	LARGE_INTEGER		m_startTime;				// ������ ī��Ʈ�� 0�϶� �ð��� ����Ǵ� ����

	Direct3D*			m_pDirect3D;
	Camera*				m_pCamera;

	int					m_nWidth;
	int					m_nHeight;

	// �� ����
	int					m_nEditMode;				// ���� , �ؽ��� , ������Ʈ

	// �귯��
	BOOL				m_bPicking;					// ��ŷ�� �� ���� üũ
	BOOL				m_bBrushInView;				// ���콺�� VIEW �ȿ� �ִ��� üũ

	// ī�޶� , ���콺
	int					m_nCameraMove;

	// ������ �ɼ�
	BOOL				m_bCoordinateAxis;			// ��ǥ��

	// �̸�����
	// �̸����� ������ CONTROLVIEW ���� �ؽ��� ���̾� �α׿��� �����´�.
	HWND				m_hWnd_Preview;				// �ؽ�ó�� �̸����� �ڵ�
	int					m_nWidth_Preview;			// �ؽ�ó�� �̸����� ����ũ��
	int					m_nHeight_Preview;			// �ؽ�ó�� �̸����� ����ũ��
};

