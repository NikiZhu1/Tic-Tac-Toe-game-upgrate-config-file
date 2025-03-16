#include <Windows.h>
#include <vector>
#include <fstream>
#include <sstream>
#include "configFileMethods.h"

int baseWindowWidth = 320, baseWindowHeight = 240; //������ ���� �� ���������
int minWindowWidth = 200, minWindowHeight = 200; //����������� ������ ����

COLORREF backColor = RGB(45, 73, 255); //����� ���� ���� �� ���������
HBRUSH hBrushBackground = CreateSolidBrush(backColor); //����� ��� �������� ����
COLORREF lineColor = RGB(255, 48, 55); //������� ���� ����� �� ���������
COLORREF xColor = RGB(255, 255, 255); //���� �������� �� ���������
COLORREF oColor = RGB(0, 0, 0); //���� ������ �� ���������

int gridSize = 3; //������ ����� �� ���������
int gridSizeConfig; //������ ����� �� ����������������� �����

int amountOfMethods = 4; //���������� ��������� ������� ��������� ����������������� �����
int currentMethod = 2;

std::vector<std::vector<char>> board; //������� ��� �������� ��������� � � O

//���������, ��� ������ ������� ������ �� ����
bool IsPositiveInteger(LPCWSTR str) {

	for (int i = 0; str[i] != L'\0'; i++) {
		if (!iswdigit(str[i]))
			return false;
	}

	return true;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInt, HINSTANCE hPrev, PWSTR pCmdLine, int nShow) {
	
	//��������� ��������� ��������� ������
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	if (argc > 3) {
		MessageBoxW(NULL, L"������� ������ 2-�� ��������� ����������! \n� �������� ������� ���� ����� ������������ �������� �� ����������������� ����� ���� ����� 3 �� ���������. ������ � ������ ����������������� ����� ����� ����������� 2 ��������.", L"�������� ����", MB_OK | MB_ICONWARNING);
	}
	else if (argc >= 2) {
		if (!IsPositiveInteger(argv[1]))
			MessageBoxW(NULL, L"������� ��������������� ����� ����� � �������� ���������!\n� �������� ������� ���� ����� ������������ �������� �� ����������������� ����� ���� ����� 3 �� ���������.", L"�������� ����", MB_OK | MB_ICONWARNING);
		else if (argv[1] == 0)
			MessageBoxW(NULL, L"� �������� ������� ���� �� ����� ���� ������������ ����� 0.\n ����� ������������ �������� �� ����������������� ����� ���� ����� 3 �� ���������.", L"�������� ����", MB_OK | MB_ICONWARNING);
		else
			gridSizeConfig = _wtoi(argv[1]);
	}
	if (argc == 3) {
		if (!IsPositiveInteger(argv[2]))
			MessageBoxW(NULL, L"������� ��������������� ����� ����� � �������� ���������!\n������ � ������ ����������������� ����� ����� ����������� 2 ��������.", L"�������� ����", MB_OK | MB_ICONWARNING);
		else if (_wtoi(argv[2]) > amountOfMethods || _wtoi(argv[2]) == 0)
			MessageBoxW(NULL, L"����� 4 ������ ������/������ ����������������� �����, ����� ������� ����� �� 1 �� 4. ������ � ������ ����������������� ����� ����� ����������� 2 ��������.", L"�������� ����", MB_OK | MB_ICONWARNING);
		else
			currentMethod = _wtoi(argv[2]); 
	}

	LoadSettings(currentMethod); //�������� �������� �� ����������������� �����
	if (gridSizeConfig != NULL) //�������� ������� ����� �� ���������, ���� ��� �����
		gridSize = gridSizeConfig;

	WNDCLASS SoftwareWindClass = { 0 };
	SoftwareWindClass.hIcon = LoadIcon(NULL, IDI_QUESTION);
	SoftwareWindClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	SoftwareWindClass.hInstance = hInt;
	SoftwareWindClass.lpszClassName = L"MainWindowClass";
	SoftwareWindClass.hbrBackground = hBrushBackground;
	SoftwareWindClass.lpfnWndProc = WindowProc;

	if (!RegisterClassW(&SoftwareWindClass)) {
		return -1;
	}

	//��������� ������� ��� X � O
	board = std::vector<std::vector<char>>(gridSize, std::vector<char>(gridSize, '.'));

	CreateWindowW(
		L"MainWindowClass",
		L"���� �������� ������",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		200, 200, //���������� ���������
		baseWindowWidth, baseWindowHeight, //������ ����
		NULL,
		NULL,
		NULL,
		NULL
	);

	MSG SoftwareMsg = { 0 };
	while (GetMessage(&SoftwareMsg, NULL, NULL, NULL)) {
		TranslateMessage(&SoftwareMsg);
		DispatchMessage(&SoftwareMsg);
	}

	return 0;
}

//��������� �����
void DrawLines(HWND hwnd, HDC hdc, int windowWidth, int windowHeight) {

	if (gridSize == 0)
		return;

	HPEN hPen = CreatePen(PS_DOT, 4, lineColor);
	SelectObject(hdc, hPen);

	for (size_t i = 1; i < gridSize; i++)
	{
		MoveToEx(hdc, windowWidth / gridSize * i, 0, NULL);
		LineTo(hdc, windowWidth / gridSize * i, windowHeight);

		MoveToEx(hdc, 0, windowHeight / gridSize * i, NULL);
		LineTo(hdc, windowWidth, windowHeight / gridSize * i);
	}

	//������� �����
	DeleteObject(hPen);
}

//��������� ��������
void DrawX(HWND hwnd, HDC hdc, int x, int y, int cellWidth, int cellHeight) {

	//���������� ������
	int cellX = (x / cellWidth) * cellWidth;
	int cellY = (y / cellHeight) * cellHeight;

	HPEN hPen = CreatePen(PS_SOLID, 8, xColor);
	SelectObject(hdc, hPen);

	MoveToEx(hdc, cellX + 10, cellY + 10, NULL);
	LineTo(hdc, cellX + cellWidth - 10, cellY + cellHeight - 10);

	MoveToEx(hdc, cellX + 10, cellY + cellHeight - 10, NULL);
	LineTo(hdc, cellX + cellWidth - 10, cellY + 10);

	//������� �����
	DeleteObject(hPen);
}

//��������� ������
void DrawO(HWND hwnd, HDC hdc, int x, int y, int cellWidth, int cellHeight) {

	//���������� ������
	int cellX = (x / cellWidth) * cellWidth;
	int cellY = (y / cellHeight) * cellHeight;

	HPEN hPen = CreatePen(PS_SOLID, 8, oColor);
	HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH); // ��������� �������

	SelectObject(hdc, hPen);
	SelectObject(hdc, hBrush);

	Ellipse(hdc, cellX + 10, cellY + 10, cellX + cellWidth - 10, cellY + cellHeight - 10);

	//������� �����
	DeleteObject(hPen);
	DeleteObject(hBrush);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	//�������� ������� ���������� ������� ����
	RECT rect;
	GetClientRect(hwnd, &rect);
	int winWidth = rect.right;
	int winHeight = rect.bottom;

	int cellWidth = winWidth / gridSize; //������ ������
	int cellHeight = winHeight / gridSize; //������ ������

	switch (uMsg) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	{
		//�������� ���������� �����
		POINT point = { 0 };
		point.x = LOWORD(lParam);
		point.y = HIWORD(lParam);

		//�������� ���������� ������, ���� ��������
		int boardX = point.x / cellWidth;
		int boardY = point.y / cellHeight;
		if (boardX >= gridSize - 1)
			boardX = gridSize - 1;
		if (boardY >= gridSize - 1)
			boardY = gridSize - 1;

		//���������� �������� � �������
		if (uMsg == WM_LBUTTONDOWN)
			board[boardY][boardX] = 'O';
		else
			board[boardY][boardX] = 'X';

		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		DrawLines(hwnd, hdc, winWidth, winHeight);

		for (int y = 0; y < gridSize; y++) {
			for (int x = 0; x < gridSize; x++) {

				int left = x * cellWidth;
				int top = y * cellHeight;
				int right = left + cellWidth;
				int bottom = top + cellHeight;

				if (board[y][x] == 'O')
					DrawO(hwnd, hdc, left, top, cellWidth, cellHeight);

				else if (board[y][x] == 'X') {
					DrawX(hwnd, hdc, left, top, cellWidth, cellHeight);
				}
			}
		}

		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {

		case VK_ESCAPE:
		{
			SaveSettings(currentMethod, hwnd); //��������� ��������� 
			PostQuitMessage(0); 
			break;
		}
		case 'Q':
		{
			if ((GetKeyState(VK_CONTROL) & 0x8000))
				SaveSettings(currentMethod, hwnd); //��������� ��������� 
			PostQuitMessage(0);
			break;
		}
		case 'C':
			if ((GetKeyState(VK_SHIFT) & 0x8000)) {
				STARTUPINFOW si = { 0 };
				PROCESS_INFORMATION pi = { 0 };
				CreateProcessW(L"c:\\windows\\system32\\notepad.exe",
					NULL, 0, 0, 0, 0, 0, 0, &si, &pi);
			}
			break;
		case VK_RETURN:
			backColor = RGB(rand() % 256, rand() % 256, rand() % 256); // ��������� ����

			DeleteObject(hBrushBackground);
			hBrushBackground = CreateSolidBrush(backColor);

			SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrushBackground);
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		}
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		int delta = GET_WHEEL_DELTA_WPARAM(wParam);

		int r = GetRValue(lineColor);
		int g = GetGValue(lineColor);
		int b = GetBValue(lineColor);

		int step = 12;
		// �������� ���� � ����������� �� ����������� ���������
		if (delta > 0) {
			r = (r + step) % 256;
			g = (g + step / 2) % 256;
			b = (b + step / 3) % 256;
		}
		else {
			r = (r - step + 256) % 256;
			g = (g - step / 2 + 256) % 256;
			b = (b - step / 3 + 256) % 256;
		}

		lineColor = RGB(r, g, b);
		InvalidateRect(hwnd, NULL, TRUE);

		return 0;
	}
	case WM_GETMINMAXINFO: {
		MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;
		pMinMax->ptMinTrackSize.x = minWindowWidth; // ����������� ������ 
		pMinMax->ptMinTrackSize.y = minWindowHeight; // ����������� ������ 
		return 0;
	}
	case WM_SIZE: //��� ��������� ������� �������������� ��
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
	case WM_DESTROY:
		RECT winrect;
		GetWindowRect(hwnd, &winrect);
		SaveSettings(currentMethod, hwnd); //��������� ���������
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}