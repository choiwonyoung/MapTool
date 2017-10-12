#include "stdafx.h"
#include "Input.h"


Input::Input()
	: m_pDI(NULL)
	, m_pKeyboard(NULL)
	, m_pMouse(NULL)
{
	ZeroMemory( &m_mouseState , sizeof( m_mouseState ) );
	ZeroMemory( &m_KeyState , sizeof( m_KeyState ) );
}


Input::~Input()
{
	if( m_pDI )
	{
		if( m_pKeyboard )
		{
			m_pKeyboard->Unacquire();
			m_pKeyboard->Release();
			m_pKeyboard = NULL;
		}

		if( m_pMouse )
		{
			m_pMouse->Unacquire();
			m_pMouse->Release();
			m_pMouse = NULL;
		}

		m_pDI->Release();
		m_pDI = NULL;
	}
}

//-----------------------------------------------------------------------------
// Direct Input �ʱ�ȭ
//-----------------------------------------------------------------------------
bool Input::InitInput( HWND hWnd , HINSTANCE hInst )
{
	//-----------------------------------------------------------------------------
	// DirectInput ����
	//-----------------------------------------------------------------------------
	if( FAILED( DirectInput8Create( 
		hInst , 
		DIRECTINPUT_VERSION , 
		IID_IDirectInput8 , 
		(void**)&m_pDI , 
		NULL 
		) ) )
		return false;

	//-----------------------------------------------------------------------------
	// keyboard �ʱ�ȭ
	//-----------------------------------------------------------------------------

	// keyboard ����̽� ����
	if( FAILED( m_pDI->CreateDevice( GUID_SysKeyboard , &m_pKeyboard , NULL ) ) )
	{
		::MessageBox( NULL , L"Keyboard Device Error." , L"Error" , MB_OK );
		return false;
	}

	// keyboard ���� ���� ����
	/*
		dwFlag : ��ġ�� �������� ���� ������ ��Ÿ��.
		DISCL_BACKGROUND : ��׶��� �׼������� �䱸 , ��׶��� ���������� �㰡�Ǹ� �Ҵ�� �����찡 ��Ƽ�� �����찡 �ƴ� ��쿡�� ������ ��ġ ��氡������
		DISCL_EXCLUSIVE : ��Ÿ ���������� �䱸 , ��Ÿ �׼������� �־����� ��ġ�� ���ǰ� �ִ� ����, ��ġ�� �ٸ� �ν��Ͻ��� ����ġ�� ��Ÿ �׼������� ����� �� ����
						  �ٸ� , �ٸ� ���ø����̼��� ��Ÿ �׼������� ����ϰ� �־ , ��ġ�� ��Ÿ ���������� �׻� ������
						  ���ø����̼��� ���콺 �Ǵ� Ű���� ��ġ�� ��Ÿ ���� ����ϰ� �ִ� ���� , WM_ENTERSIZEMOVE �޼��� �� WM_ENTERMENULOOP �޼����� �޾Ƶ鿴�� ����, 
						  �׻� ��ġ�� ������ �� �ʿ䰡 ����
		DISCL_FOREGROUND : ���ø����̼��� foreground �׼������� �䱸�Ѵ�. �� ��Ǹ� ��׶���� �̵� �� �� , ��ġ�� �ڵ������� ������
		DISCL_NONEXCLUSIVE : �� ��Ÿ �׼����� �䱸 , ��ġ�� ���� �׼������� ���� ��ġ�� �׼��� �ϰ� �ִ� �ٸ� ���ø����̼��� �������� ����
		DISCL_NOWINKEY : ������ Ű�� ��ȿ�� ��, �� �÷��׸� �����ϸ� , ����ڴ� �߸��� ���ø����̼��� �����ϴ� ���� ������

	*/
	if( FAILED( m_pKeyboard->SetCooperativeLevel( hWnd , DISCL_BACKGROUND | DISCL_NONEXCLUSIVE ) ) )
	{
		MessageBox( NULL , L"Keyboard SetCooperativeLevel Error" , L"Error" , MB_OK );
		return false;
	}

	// Keyboard ����Ÿ ���� ����
	/*
		lpdf :	c_dfDIKeyboard
				c_dfDIMouse
				c_dfDIMouse2
				c_dfDIJoystick
				c_dfDIJoystick2
	*/
	if( FAILED( m_pKeyboard->SetDataFormat( &c_dfDIKeyboard ) ) )
	{
		MessageBox( NULL , L"Keyboard SetDataFormat Error" , L"Error" , MB_OK );
		return false;
	}

	// Keyboard ��ġ ������
	if( FAILED( m_pKeyboard->Acquire() ) )
	{
		MessageBox( NULL , L"Keyboard Acquire Error" , L"Error" , MB_OK );
		return false;
	}

	//-----------------------------------------------------------------------------
	// Mouse �ʱ�ȭ
	//-----------------------------------------------------------------------------

	//���콺 ����̽� ����
	if( FAILED( m_pDI->CreateDevice( GUID_SysMouse , &m_pMouse , NULL ) ) )
	{
		MessageBox( NULL , L"Mouse Device Error" , L"Error" , MB_OK );
		return false;
	}

	// Mouse ���� ���� ����
	if( FAILED( m_pMouse->SetCooperativeLevel( hWnd , DISCL_BACKGROUND | DISCL_NONEXCLUSIVE ) ) )
	{
		MessageBox( NULL , L"Mouse SetCooperativeLevel Error" , L"Error" , MB_OK );
		return false;
	}

	// Mouse ������ ����
	if( FAILED( m_pMouse->SetDataFormat( &c_dfDIMouse ) ) )
	{
		MessageBox( NULL , L"Mouse SetDataFormat Error" , L"Error" , MB_OK );
		return false;
	}

	if( FAILED( m_pMouse->Acquire() ) )
	{
		MessageBox( NULL , L"Mouse Acquire Error" , L"Error" , MB_OK );
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// keyboard , mouse �Է°� ������
//-----------------------------------------------------------------------------
bool Input::Update()
{
	/*
		GetDeviceState �� �ش� �Է���ġ�� ����Ÿ�� �����´�.
	*/
	if( FAILED( m_pKeyboard->GetDeviceState( sizeof( m_KeyState ) , (LPVOID)m_KeyState ) ) )
	{
		if( m_pKeyboard->Acquire() )
			return false;

		if( FAILED( m_pKeyboard->GetDeviceState( sizeof( m_KeyState ) , (LPVOID)m_KeyState ) ) )
			return false;
	}

	if( FAILED( m_pMouse->GetDeviceState( sizeof( DIMOUSESTATE ) , &m_mouseState ) ) )
	{
		if( FAILED(m_pMouse->Acquire() ) )
			return false;

		if( FAILED( m_pMouse->GetDeviceState( sizeof( DIMOUSESTATE ) , &m_mouseState ) ) )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Ű���� Ű �� , �ٿ�
//-----------------------------------------------------------------------------
bool Input::KeyDown( BYTE key )
{
	return ( m_KeyState[key] & 0x80 ) ? true : false;
}

bool Input::KeyUp( BYTE key )
{
	return ( m_KeyState[key] & 0x80 ) ? true : false;
}

//-----------------------------------------------------------------------------
// ���콺 ��ư �� , �ٿ� ,
// button : 0 : ���� , 1 : ����
//-----------------------------------------------------------------------------
bool Input::ButtonDown( int button )
{
	return ( m_mouseState.rgbButtons[button] & 0x80 ) ? true : false;
}

bool Input::ButtonUp( int button )
{
	return ( m_mouseState.rgbButtons[button] & 0x80 ) ? true : false;
}

//-----------------------------------------------------------------------------
// ���콺�� �����϶� ��ǥ
//-----------------------------------------------------------------------------
void Input::GetMouseMovement( int &dx , int &dy )
{
	dx = m_mouseState.lX;
	dy = m_mouseState.lY;
}

//-----------------------------------------------------------------------------
// ���콺 ���� �����϶�
//-----------------------------------------------------------------------------
int Input::GetMouseWheelMovement()
{
	return m_mouseState.lZ;
}


