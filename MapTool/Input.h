#pragma once

#include <d3d9.h>
#include <dinput.h>

class Input
{
public:
	Input();
	~Input();

	bool	InitInput( HWND hWnd , HINSTANCE hInst );
	bool	Update();

	// Keyboard
	bool	KeyDown( BYTE key );
	bool	KeyUp( BYTE key );

	// Mouse
	bool	ButtonDown( int button );
	bool	ButtonUp( int button );
	void	GetMouseMovement( int &dx , int &dy );
	int		GetMouseWheelMovement();

	int		GetMousePosX() { return m_mouseState.lX; }
	int		GetMousePosY() { return m_mouseState.lY; }

private:
	LPDIRECTINPUT		m_pDI;
	LPDIRECTINPUTDEVICE m_pKeyboard;
	LPDIRECTINPUTDEVICE	m_pMouse;

	DIMOUSESTATE		m_mouseState;
	BYTE				m_KeyState[256];
};

