#pragma once

#include "Direct3D.h"

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
	BOOL			m_bLCtrlKeyDown;		// Ű���� ���� ��Ʈ�� Ű �ٿ�
	BOOL			m_bLAltKeyDown;		// Ű���� ���� ��Ʈ Ű �ٿ�
	BOOL			m_bLShiftKeyDown;		// Ű���� ���� ����Ʈ Ű �ٿ�

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
};

