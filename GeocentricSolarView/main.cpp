#define WINDOW_CLASS_NAME "GEOCENTRIC_ORBITAL_VIEWER_WINDOW"
#define WINDOW_TITLE "Geocentric Orbital Viewer"
#include "windowSetup.h"
#include "FrameManager.h"

#include <chrono>

#define FPS 120

#define KEY_R 0x52
#define KEY_T 0x54

unsigned int radius = 20;

unsigned int tempWindowWidth;
unsigned int tempWindowHeight;
bool tempWindowSizesUpdated = false;
bool windowMaximized = false;
bool windowResized = false;

bool geocentricView = false;
LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_SPACE: geocentricView = !geocentricView; return 0;
		case KEY_R: if (radius == 0) { return 0; } radius--; return 0;
		case KEY_T: radius++; return 0;
		}
	case WM_SIZE:
		switch (wParam) {
		case SIZE_RESTORED:
			if (windowMaximized) {
				setWindowSize(LOWORD(lParam), HIWORD(lParam));
				windowResized = true;
				windowMaximized = false;
				return 0;
			}
			tempWindowWidth = LOWORD(lParam);
			tempWindowHeight = HIWORD(lParam);
			tempWindowSizesUpdated = true;
			return 0;
		case SIZE_MAXIMIZED:
			setWindowSize(LOWORD(lParam), HIWORD(lParam));
			windowResized = true;
			windowMaximized = true;
			tempWindowSizesUpdated = false;
			return 0;
		}
		return 0;
	case WM_EXITSIZEMOVE:
		if (tempWindowSizesUpdated) {
			setWindowSize(tempWindowWidth, tempWindowHeight);
			windowResized = true;
			tempWindowSizesUpdated = false;
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

unsigned int windowWidth;
unsigned int windowHeight;
unsigned int windowHalfWidth;
unsigned int windowHalfHeight;
float posMultiplier;
unsigned int posOffsetX;
unsigned int posOffsetY;
void setWindowSize(unsigned int windowWidth, unsigned int windowHeight) {
	::windowWidth = windowWidth;
	::windowHeight = windowHeight;
	windowHalfWidth = windowWidth / 2;
	windowHalfHeight = windowHeight / 2;
	if (windowWidth < windowHeight) {
		posOffsetX = 0;
		posOffsetY = (windowHeight - windowWidth) / 2;
		posMultiplier = windowWidth / 1000.0f;
		return;
	}
	posOffsetX = (windowWidth - windowHeight) / 2;
	posOffsetY = 0;
	posMultiplier = windowHeight / 1000.0f;
}

HDC finalG;
HDC g;

struct Body { float x; float y; } firstBody = { 750, 500 }, secondBody = { 100, 500 };
void rotate(Body& body, float angle) {
	float relativeX = body.x - 500;
	float relativeY = body.y - 500;
	body.x = cos(angle) * relativeX - sin(angle) * relativeY + 500;
	body.y = sin(angle) * relativeX + cos(angle) * relativeY + 500;
}
void render(Body body) {
	body.x = body.x * posMultiplier + posOffsetX;
	body.y = body.y * posMultiplier + posOffsetY;
	Ellipse(g, body.x - radius, body.y - radius, body.x + radius, body.y + radius);
}
void rawRender(int x, int y) { Ellipse(g, x - radius, y - radius, x + radius, y + radius); }

void renderFrame() {
	BitBlt(finalG, 0, 0, windowWidth, windowHeight, g, 0, 0, SRCCOPY);
	Rectangle(g, 0, 0, windowWidth, windowHeight);
}

void graphicsLoop(HWND hWnd) {
	debuglogger::out << "setting up DCs and GDI objects..." << debuglogger::endl;
	finalG = GetDC(hWnd);
	g = CreateCompatibleDC(finalG);
	HBITMAP bmp = CreateCompatibleBitmap(finalG, windowWidth, windowHeight);
	SelectObject(g, bmp);

	HGDIOBJ bgBrush = GetStockObject(BLACK_BRUSH);
	HGDIOBJ bgPen = GetStockObject(BLACK_PEN);
	HGDIOBJ firstBodyBrush = GetStockObject(WHITE_BRUSH);
	HGDIOBJ firstBodyPen = GetStockObject(WHITE_PEN);

	HBRUSH secondBodyBrush = CreateSolidBrush(RGB(0, 255, 0));
	HPEN secondBodyPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));

	FrameManager frameMan(FPS);
	float frameMultiplier = 0;
	debuglogger::out << "entering graphics loop..." << debuglogger::endl;
	while (isAlive) {
		frameMan.start();

		SelectObject(g, firstBodyBrush);
		SelectObject(g, firstBodyPen);
		if (geocentricView) { rawRender(windowHalfWidth, windowHalfHeight); }
		else { render(firstBody); }
		rotate(firstBody, 0.04f * frameMultiplier);

		SelectObject(g, secondBodyBrush);
		SelectObject(g, secondBodyPen);
		if (geocentricView) { render({ (secondBody.x - firstBody.x) / 2 + 500, (secondBody.y - firstBody.y) / 2 + 500 }); }
		else { render(secondBody); }
		rotate(secondBody, 0.01f * frameMultiplier);

		SelectObject(g, bgBrush);
		SelectObject(g, bgPen);
		renderFrame();

		if (windowResized) {
			DeleteObject(bmp);
			bmp = CreateCompatibleBitmap(finalG, windowWidth, windowHeight);
			SelectObject(g, bmp);
			windowResized = false;
		}

		frameMultiplier = frameMan.calculateMultiplier();
	}

	ReleaseDC(hWnd, finalG);
	DeleteDC(g);
	DeleteObject(bmp);

	DeleteObject(secondBodyBrush);
	DeleteObject(secondBodyPen);
}
