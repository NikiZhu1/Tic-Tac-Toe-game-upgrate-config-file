#include <windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "configFileMethods.h"
#include "json.hpp"
using json = nlohmann::json;

const wchar_t* configFile = L"config.json"; //���������������� ����
const size_t configFileSize = 1024;

extern int minWindowWidth;
extern int minWindowHeight;
extern int baseWindowWidth;
extern int baseWindowHeight;
extern int gridSize;
extern COLORREF lineColor;
extern COLORREF backColor;
extern HBRUSH hBrushBackground;

// ������� ��� ����������� wchar_t* � char*
std::string WideStringToUtf8(const wchar_t* wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    std::string str(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], size_needed, NULL, NULL);
    return str;
}

void ReadJson(json config) {
    // �������� gridSize
    if (config.contains("gridSize") && config["gridSize"].is_number_integer() && config["gridSize"] > 0) {
        gridSize = config["gridSize"];
    }

    // �������� winSize 
    if (config.contains("winSize") && config["winSize"].is_array() && config["winSize"].size() == 2) {
        if (config["winSize"][0].is_number_integer() &&
            config["winSize"][1].is_number_integer() &&
            config["winSize"][0] > minWindowWidth &&
            config["winSize"][1] > minWindowHeight) {
            baseWindowWidth = config["winSize"][0];
            baseWindowHeight = config["winSize"][1];
        }
    }

    // �������� backColor
    if (config.contains("backColor") && config["backColor"].is_array() && config["backColor"].size() == 3) {
        if (config["backColor"][0].is_number_integer() &&
            config["backColor"][1].is_number_integer() &&
            config["backColor"][2].is_number_integer()) {
            int r = config["backColor"][0],
                g = config["backColor"][1],
                b = config["backColor"][2];
            if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
                backColor = RGB(r, g, b);
                hBrushBackground = CreateSolidBrush(backColor);
            }
        }
    }

    // �������� lineColor
    if (config.contains("lineColor") && config["lineColor"].is_array() && config["lineColor"].size() == 3) {
        if (config["lineColor"][0].is_number_integer() &&
            config["lineColor"][1].is_number_integer() &&
            config["lineColor"][2].is_number_integer()) {
            int r = config["lineColor"][0],
                g = config["lineColor"][1],
                b = config["lineColor"][2];
            if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
                lineColor = RGB(r, g, b);
            }
        }
    }
}

//�������� �������� ��������� �������
void LoadSettings(int method) {
    switch (method) {
    case 1: 
        FileMap_ReadConfig();
        break;
    case 2:
        C_ReadConfig();
        break;
    case 3:
        Fstream_ReadConfig();
        break;
    case 4: 
        WinApi_ReadConfig();
        break;
    default:
        C_ReadConfig();
        break;
    }
}

//���������� �������� ��������� �������
void SaveSettings(int method, HWND hwnd) {
    switch (method) {
    case 1:
        FileMap_WriteConfig(hwnd);
        break;
    case 2:
        C_WriteConfig(hwnd);
        break;
    case 3:
        Fstream_WriteConfig(hwnd);
        break;
    case 4:
        WinApi_WriteConfig(hwnd);
        break;
    default:
        C_WriteConfig(hwnd);
        break;
    }
}

//����� ������ ������� ����������� �� ������
void FileMap_WriteConfig(HWND hwnd) {
    HANDLE hFile = CreateFile(
        configFile, 
        GENERIC_READ | GENERIC_WRITE, 
        0, 
        NULL, 
        CREATE_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) { 
        MessageBoxW(NULL, L"������ �������� ����������������� �����", L"������", MB_OK | MB_ICONERROR); 
        return;
    }

    RECT winrect;
    GetWindowRect(hwnd, &winrect);
    int winWidth = winrect.right - winrect.left;
    int winHeight = winrect.bottom - winrect.top;

    json config = {
        {"gridSize", gridSize},
        {"winSize", {winWidth, winHeight}},
        {"backColor", {GetRValue(backColor), GetGValue(backColor), GetBValue(backColor)}},
        {"lineColor", {GetRValue(lineColor), GetGValue(lineColor), GetBValue(lineColor)}}
    };

    std::string jsonStr = config.dump(4);
    DWORD fileSize = static_cast<DWORD>(jsonStr.size());

    HANDLE hMapping = CreateFileMapping(
        hFile, 
        NULL, 
        PAGE_READWRITE, 
        0, 
        fileSize, 
        NULL
    );

    if (hMapping == NULL) {
        MessageBoxW(NULL, L"������ �������� ����������� �����", L"������", MB_OK | MB_ICONWARNING);
        CloseHandle(hFile);
        return;
    }

    LPVOID mapView = MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, fileSize);
    if (mapView == NULL) {
        MessageBoxW(NULL, L"������ ����������� ����� � ������", L"������", MB_OK | MB_ICONWARNING); 
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    CopyMemory(mapView, jsonStr.c_str(), fileSize);

    UnmapViewOfFile(mapView);
    CloseHandle(hMapping);
    CloseHandle(hFile);
}


//����� ������ ������� ����������� �� ������
void FileMap_ReadConfig() {
    // ��������� ����
    HANDLE hFile = CreateFile( 
        configFile, 
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL, 
        OPEN_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL 
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxW(NULL, L"������ �������� ����������������� �����", L"������", MB_OK | MB_ICONERROR);
        return;
    }

    // ��������� ������ �����
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        MessageBoxW(NULL, L"������ ��������� ������� �����", L"������", MB_OK | MB_ICONERROR);
        CloseHandle(hFile);
        return;
    }

    // ���� ���� ����, ���������� `{}` (������ JSON)
    if (fileSize == 0) {
        DWORD written;
        const char* emptyJson = "{}";
        WriteFile(hFile, emptyJson, strlen(emptyJson), &written, NULL);
        fileSize = strlen(emptyJson);
    }

    // ������� ����������� ����� �� ������
    HANDLE hMapping = CreateFileMapping(
        hFile, 
        NULL, 
        PAGE_READONLY, 
        0,
        fileSize,
        NULL 
    );

    if (hMapping == NULL) {
        MessageBoxW(NULL, L"������ �������� ����������� �����", L"������", MB_OK | MB_ICONERROR);
        CloseHandle(hFile);
        return;
    }

    // ���������� ���� �� ������
    LPVOID mapView = MapViewOfFile( 
        hMapping, 
        FILE_MAP_READ, 
        0,
        0,
        fileSize
    );

    if (mapView == NULL) 
    {
        MessageBoxW(NULL, L"������: " + GetLastError(), L"������ ������ ����������� � ������", MB_OK | MB_ICONWARNING);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    // ����������� ������ � ������
    std::string data(static_cast<const char*>(mapView), 1024);

    try
    {
        json config = json::parse(data);
        ReadJson(config);
    }
    catch (...) {
        MessageBox(NULL, L"�� ���������� ��������� ������������ ���� � �����������, ����� ������������ ��������� �� ���������. \n\n����� �������� ���������� ������������� ��������� ���������� � ��� ������ �������.", L"������ ������", MB_OK | MB_ICONWARNING);
    }

    // ����������� �������
    UnmapViewOfFile(mapView);
    CloseHandle(hMapping);
    CloseHandle(hFile);
}

//����� ������ ������� �������� ������� C
void C_WriteConfig(HWND hwnd) {
    std::string fileName = WideStringToUtf8(configFile);  // ������������ ��� ����� � UTF-8 

    FILE* file; 
    errno_t err = fopen_s(&file, fileName.c_str(), "w"); 
    
    if (err != 0 || file == nullptr) {
        MessageBox(NULL, L"�� ���������� �������� ���������, ����� ������������ ��������� �� ���������.", L"������ ������", MB_OK | MB_ICONWARNING);
        return;
    }

    RECT winrect; 
    GetWindowRect(hwnd, &winrect); 
    int winWidth = winrect.right - winrect.left; 
    int winHeight = winrect.bottom - winrect.top; 

    json config =
    {
        {"gridSize", gridSize},
        {"winSize", { winWidth, winHeight }},
        {"backColor", { GetRValue(backColor), GetGValue(backColor), GetBValue(backColor) }},
        {"lineColor", { GetRValue(lineColor), GetGValue(lineColor), GetBValue(lineColor) }}
    };

    std::string jsonStr = config.dump(4);

    fprintf(file, jsonStr.c_str());
    fclose(file);
}

//����� ������ ������� �������� ������� C
void C_ReadConfig() { 
    std::string fileName = WideStringToUtf8(configFile);

    FILE* file; 
    errno_t err = fopen_s(&file, fileName.c_str(), "r"); 

    if (err != 0 || file == nullptr) {
        MessageBox(NULL, L"������ ������ ����������������� �����, ����� ������������ ��������� �� ���������.", L"������ ������", MB_OK | MB_ICONWARNING);
        return;
    }

    // ���������� ������ �����
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // ������ ���� � ������
    std::string jsonStr(fileSize, '\0');
    fread(&jsonStr[0], 1, fileSize, file);
    fclose(file);

    try
    {
        json config = json::parse(jsonStr);
        ReadJson(config);
    }
    catch (...) {
        MessageBox(NULL, L"�� ���������� ��������� ������������ ���� � �����������, ����� ������������ ��������� �� ���������. \n\n����� �������� ���������� ������������� ��������� ���������� � ��� ������ �������.", L"������ ������", MB_OK | MB_ICONWARNING);
    }
}

//����� ������ ������� ������� C++
void Fstream_WriteConfig(HWND hwnd) {
    RECT winrect;
    GetWindowRect(hwnd, &winrect);
    int winWidth = winrect.right - winrect.left;
    int winHeight = winrect.bottom - winrect.top;

    json config =
    {
        {"gridSize", gridSize},
        {"winSize", { winWidth, winHeight }},
        {"backColor", { GetRValue(backColor), GetGValue(backColor), GetBValue(backColor) }},
        {"lineColor", { GetRValue(lineColor), GetGValue(lineColor), GetBValue(lineColor) }}
    };

    std::ofstream file(configFile);
    file << config.dump(4);
}

//����� ������ ������� ������� C++
void Fstream_ReadConfig() {
    std::ifstream file(configFile);
    if (!file.is_open()) 
        return;

    try
    {
        json config;
        file >> config;
        ReadJson(config);
    }
    catch (...) {
        MessageBox(NULL, L"�� ���������� ��������� ������������ ���� � �����������, ����� ������������ ��������� �� ���������. \n\n����� �������� ���������� ������������� ��������� ���������� � ��� ������ �������.", L"������ ������", MB_OK | MB_ICONWARNING);
    }
}

//����� ������ ������� �������� ������� WinAPI
void WinApi_WriteConfig(HWND hwnd) {
    HANDLE hFile = CreateFile(
        configFile,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
        return;

    RECT winrect;
    GetWindowRect(hwnd, &winrect);
    int winWidth = winrect.right - winrect.left;
    int winHeight = winrect.bottom - winrect.top;

    // ������� JSON-������ 
    json config = {
        {"gridSize", gridSize},
        {"winSize", {winWidth, winHeight}},
        {"backColor", {GetRValue(backColor), GetGValue(backColor), GetBValue(backColor)}},
        {"lineColor", {GetRValue(lineColor), GetGValue(lineColor), GetBValue(lineColor)}}
    };

    std::string jsonStr = config.dump(4);

    DWORD written;
    WriteFile(hFile, jsonStr.c_str(), jsonStr.size(), &written, NULL);
    CloseHandle(hFile);
}

//����� ������ ������� �������� ������� WinAPI
void WinApi_ReadConfig() {
    HANDLE hFile = CreateFile(
        configFile,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) 
        return;

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0) {
        CloseHandle(hFile);
        return;
    }

    // �������� ����� ��� ���������� �����
    char* buffer = new char[fileSize + 1];
    DWORD bytesRead;
    BOOL success = ReadFile(hFile, buffer, fileSize, &bytesRead, NULL);

    CloseHandle(hFile);

    if (!success || bytesRead != fileSize) {
        delete[] buffer;
        return;
    }

    buffer[fileSize] = '\0';

    try {
        json config = json::parse(buffer);
        delete[] buffer;  // ������� �����
        ReadJson(config); 
    }
    catch (...) {
        MessageBox(NULL, L"�� ���������� ��������� ������������ ���� � �����������, ����� ������������ ��������� �� ���������. \n\n����� �������� ���������� ������������� ��������� ���������� � ��� ������ �������.", L"������ ������", MB_OK | MB_ICONWARNING);
    }
}