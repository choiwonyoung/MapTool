#pragma once

#include "Direct3D.h"

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
	BOOL			m_bLCtrlKeyDown;		// 키보드 왼쪽 컨트롤 키 다운
	BOOL			m_bLAltKeyDown;		// 키보드 왼쪽 알트 키 다운
	BOOL			m_bLShiftKeyDown;		// 키보드 왼쪽 쉬프트 키 다운

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
};

