#include <windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "configFileMethods.h"
#include "json.hpp"
using json = nlohmann::json;

const wchar_t* configFile = L"config.json"; //Конфигурационный файл
const size_t configFileSize = 1024;

extern int minWindowWidth;
extern int minWindowHeight;
extern int baseWindowWidth;
extern int baseWindowHeight;
extern int gridSize;
extern COLORREF lineColor;
extern COLORREF backColor;
extern HBRUSH hBrushBackground;

// Функция для конвертации wchar_t* в char*
std::string WideStringToUtf8(const wchar_t* wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    std::string str(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], size_needed, NULL, NULL);
    return str;
}

void ReadJson(json config) {
    // Проверка gridSize
    if (config.contains("gridSize") && config["gridSize"].is_number_integer() && config["gridSize"] > 0) {
        gridSize = config["gridSize"];
    }

    // Проверка winSize 
    if (config.contains("winSize") && config["winSize"].is_array() && config["winSize"].size() == 2) {
        if (config["winSize"][0].is_number_integer() &&
            config["winSize"][1].is_number_integer() &&
            config["winSize"][0] > minWindowWidth &&
            config["winSize"][1] > minWindowHeight) {
            baseWindowWidth = config["winSize"][0];
            baseWindowHeight = config["winSize"][1];
        }
    }

    // Проверка backColor
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

    // Проверка lineColor
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

//Загрузка настроек выбранным методом
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

//Сохранение настроек выбранным методом
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

//Метод записи методом отображения на память
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
        MessageBoxW(NULL, L"Ошибка открытия конфигурационного файла", L"Ошибка", MB_OK | MB_ICONERROR); 
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
        MessageBoxW(NULL, L"Ошибка создания отображения файла", L"Ошибка", MB_OK | MB_ICONWARNING);
        CloseHandle(hFile);
        return;
    }

    LPVOID mapView = MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, fileSize);
    if (mapView == NULL) {
        MessageBoxW(NULL, L"Ошибка отображения файла в память", L"Ошибка", MB_OK | MB_ICONWARNING); 
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    CopyMemory(mapView, jsonStr.c_str(), fileSize);

    UnmapViewOfFile(mapView);
    CloseHandle(hMapping);
    CloseHandle(hFile);
}


//Метод чтения методом отображения на память
void FileMap_ReadConfig() {
    // Открываем файл
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
        MessageBoxW(NULL, L"Ошибка открытия конфигурационного файла", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Проверяем размер файла
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        MessageBoxW(NULL, L"Ошибка получения размера файла", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hFile);
        return;
    }

    // Если файл пуст, записываем `{}` (пустой JSON)
    if (fileSize == 0) {
        DWORD written;
        const char* emptyJson = "{}";
        WriteFile(hFile, emptyJson, strlen(emptyJson), &written, NULL);
        fileSize = strlen(emptyJson);
    }

    // Создаем отображение файла на память
    HANDLE hMapping = CreateFileMapping(
        hFile, 
        NULL, 
        PAGE_READONLY, 
        0,
        fileSize,
        NULL 
    );

    if (hMapping == NULL) {
        MessageBoxW(NULL, L"Ошибка создания отображения файла", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(hFile);
        return;
    }

    // Отображаем файл на память
    LPVOID mapView = MapViewOfFile( 
        hMapping, 
        FILE_MAP_READ, 
        0,
        0,
        fileSize
    );

    if (mapView == NULL) 
    {
        MessageBoxW(NULL, L"Ошибка: " + GetLastError(), L"Ошибка чтения отображения с памяти", MB_OK | MB_ICONWARNING);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    // Преобразуем данные в строку
    std::string data(static_cast<const char*>(mapView), 1024);

    try
    {
        json config = json::parse(data);
        ReadJson(config);
    }
    catch (...) {
        MessageBox(NULL, L"Не получилось прочитать существующий файл с настройками, будут используются настройки по умолчанию. \n\nПосле закрытия приложения установленные настройки сохранятся и это ошбика пропадёт.", L"Ошибка чтения", MB_OK | MB_ICONWARNING);
    }

    // Освобождаем ресурсы
    UnmapViewOfFile(mapView);
    CloseHandle(hMapping);
    CloseHandle(hFile);
}

//Метод записи методом файловых функций C
void C_WriteConfig(HWND hwnd) {
    std::string fileName = WideStringToUtf8(configFile);  // Конвертируем имя файла в UTF-8 

    FILE* file; 
    errno_t err = fopen_s(&file, fileName.c_str(), "w"); 
    
    if (err != 0 || file == nullptr) {
        MessageBox(NULL, L"Не получилось записать настройки, будут используются настройки по умолчанию.", L"Ошибка записи", MB_OK | MB_ICONWARNING);
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

//Метод чтения методом файловых функций C
void C_ReadConfig() { 
    std::string fileName = WideStringToUtf8(configFile);

    FILE* file; 
    errno_t err = fopen_s(&file, fileName.c_str(), "r"); 

    if (err != 0 || file == nullptr) {
        MessageBox(NULL, L"Ошибка чтения конфигурационного файла, будут используются настройки по умолчанию.", L"Ошибка чтения", MB_OK | MB_ICONWARNING);
        return;
    }

    // Определяем размер файла
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Читаем файл в строку
    std::string jsonStr(fileSize, '\0');
    fread(&jsonStr[0], 1, fileSize, file);
    fclose(file);

    try
    {
        json config = json::parse(jsonStr);
        ReadJson(config);
    }
    catch (...) {
        MessageBox(NULL, L"Не получилось прочитать существующий файл с настройками, будут используются настройки по умолчанию. \n\nПосле закрытия приложения установленные настройки сохранятся и это ошбика пропадёт.", L"Ошибка чтения", MB_OK | MB_ICONWARNING);
    }
}

//Метод записи методом потоков C++
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

//Метод чтения методом потоков C++
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
        MessageBox(NULL, L"Не получилось прочитать существующий файл с настройками, будут используются настройки по умолчанию. \n\nПосле закрытия приложения установленные настройки сохранятся и это ошбика пропадёт.", L"Ошибка чтения", MB_OK | MB_ICONWARNING);
    }
}

//Метод записи методом файловых функций WinAPI
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

    // Создаем JSON-объект 
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

//Метод чтения методом файловых функций WinAPI
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

    // Выделяем буфер под содержимое файла
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
        delete[] buffer;  // Очищаем буфер
        ReadJson(config); 
    }
    catch (...) {
        MessageBox(NULL, L"Не получилось прочитать существующий файл с настройками, будут используются настройки по умолчанию. \n\nПосле закрытия приложения установленные настройки сохранятся и это ошбика пропадёт.", L"Ошибка чтения", MB_OK | MB_ICONWARNING);
    }
}