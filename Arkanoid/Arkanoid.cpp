#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include <malloc.h>
#include <stdio.h>
#include <time.h>
#include <list>
#include <iterator>

#include "Brick.h"
#include "Paddle.h"



void Reset();
void OnCreate(HWND hWnd);
void OnUpdate(HWND hWnd);
void OnTimer(HWND hWnd);
void OnMouseMove(HWND hWnd, int x, int y);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);




int windowWidth = 720;
int windowHeight = 720;

int ballX = 0;
int ballY = 0;
int dBallX = 0;
int dBallY = 0;
int ballSpeed = 4;

std::list <Brick> bricks;
Paddle paddle = Paddle(0, windowHeight - 100);



void Reset()
{
	// Set default Ball position and direction
	ballX = windowWidth / 2;
	ballY = windowHeight / 2;
	dBallX = rand() % 2;
	dBallY = rand() % 2;
	dBallX = (dBallX == 0) ? -1 : 1;
	dBallY = (dBallY == 0) ? -1 : 1;

	// Create bricks
	bricks.clear();
	int numberOfBricksPerLevel = windowWidth / Brick::width;
	for (size_t level = 0;  level < 3;  level++)
	{
		for (size_t i = 0; i < numberOfBricksPerLevel; i++)
		{
			Brick brick = Brick(i * (100 + 20), 30 + 50 * level, level);
			bricks.push_back(brick);
		}
	}
}

void OnCreate(HWND hWnd)
{
	// Set timer to clear window every 5ms
	SetTimer(hWnd, 1, 1, NULL);

	srand(time(0));
	Reset();
}

void OnTimer(HWND hWnd)
{
	// Update Ball position
	ballX += dBallX * ballSpeed;
	ballY += dBallY * ballSpeed;

	// Check for ball collisions
	if (ballY <= 0)
	{
		dBallY = 1;
	}
	if (ballY >= windowHeight - 35)
	{
		Reset(); // Reset game if ball hits the bottom wall
	}
	if (ballX <= 0)
	{
		dBallX = 1;
	}
	if (ballX >= windowWidth - 35)
	{
		dBallX = -1;
	}

	// Check for Ball on Paddle
	if (ballX >= paddle.x - 10 && ballX < paddle.x + Paddle::width && ballY == paddle.y - Paddle::height)
	{
		dBallY = -1;
	}

	// Check for Ball on Brick
	std::list<Brick>::iterator brick;
	for (brick = bricks.begin(); brick != bricks.end(); brick++)
	{
		if (brick->isHit(ballX, ballY))
		{
			dBallY = 1;
			bricks.erase(brick);
			break;
		}
	}

	// Call OnUpdate()
	RECT rt;
	GetClientRect(hWnd, &rt);
	InvalidateRect(hWnd, &rt, FALSE);	
}

void __DrawLine(HDC DC, int x1, int y1, int x2, int y2, COLORREF color, int width)
{
	HPEN Stift = CreatePen(PS_SOLID, width, color);
	SelectObject(DC, Stift);
	MoveToEx(DC, x1, y1, NULL);
	LineTo(DC, x2, y2);
	DeleteObject(Stift);
}

void __DrawRectangle(HDC DC, int x1, int y1, int x2, int y2, COLORREF color, int width)
{
	__DrawLine(DC, x1, y1, x2, y1, color, width); // top line
	__DrawLine(DC, x1, y1, x1, y2, color, width); // left line
	__DrawLine(DC, x1, y2, x2, y2, color, width); // bottom line
	__DrawLine(DC, x2, y1, x2, y2, color, width); // right line
}

void OnUpdate(HWND hWnd)
{
	PAINTSTRUCT PaintStruct;
	HDC DC_ = BeginPaint(hWnd, &PaintStruct);
	HBITMAP hbmMem, hbmOld;
	HDC DC = CreateCompatibleDC(DC_);
	RECT rc;
	GetClientRect(hWnd, &rc);
	hbmMem = CreateCompatibleBitmap(DC_, rc.right - rc.left, 2000);
	hbmOld = (HBITMAP)SelectObject(DC, hbmMem);

	// Draw background
	COLORREF bg = RGB(100, 100, 100);
	HBRUSH hbrBkGnd = CreateSolidBrush(bg);
	FillRect(DC, &rc, hbrBkGnd);
	DeleteObject(hbrBkGnd);

	// Draw ball
	__DrawLine(DC, ballX, ballY, ballX + 1, ballY + 1, RGB(255, 0, 0), 25);

	// Draw paddle
	__DrawRectangle(DC, paddle.x, paddle.y, paddle.x + Paddle::width, paddle.y + Paddle::height, RGB(0, 0, 0), 16);
	paddle.Draw();

	// Draw bricks
	std::list<Brick>::iterator brick;
	for (brick = bricks.begin(); brick != bricks.end(); brick++)
	{
		__DrawRectangle(
			DC,
			brick->x, brick->y,
			brick->x + Brick::width, brick->y + Brick::height,
			brick->getColor(),
			4
		);
	}

	// Update window with double buffering
	BitBlt(DC_, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, DC, 0, 0, SRCCOPY);
	SelectObject(DC, hbmOld);
	DeleteObject(hbmMem);
	DeleteDC(DC);
	EndPaint(hWnd, &PaintStruct);
}

void OnMouseMove(HWND hWnd, int x, int y)
{
	// Update paddle X position
	paddle.Move(x);
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	HWND                hWnd;
	MSG                 msg;
	WNDCLASS            wndClass;
	ULONG_PTR           gdiplusToken;

	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = TEXT("Arkanoid");

	RegisterClass(&wndClass);

	hWnd = CreateWindow(
		TEXT("Arkanoid"),         // window class name
		TEXT("Arkanoid"),         // window caption
		WS_OVERLAPPEDWINDOW,      // window style
		CW_USEDEFAULT,            // initial x position
		CW_USEDEFAULT,            // initial y position
		windowWidth,              // initial x size
		windowHeight,             // initial y size
		NULL,                     // parent window handle
		NULL,                     // window menu handle
		hInstance,                // program instance handle
		NULL);                    // creation parameters

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		OnCreate(hWnd);
		break;
	case WM_TIMER:
		OnTimer(hWnd);
		break;
	case WM_PAINT:
		OnUpdate(hWnd);
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}