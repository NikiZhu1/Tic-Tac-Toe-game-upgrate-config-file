#ifndef CONFIGFILEMETHODS_H
#define CONFIGFILEMETHODS_H

#include <string>

// Отображение на память
void FileMap_ReadConfig();
void FileMap_WriteConfig(HWND hwnd);

// Файловые переменные
void C_ReadConfig();
void C_WriteConfig(HWND hwnd);

// Потоки
void Fstream_ReadConfig();
void Fstream_WriteConfig(HWND hwnd);

// WinAPI
void WinApi_ReadConfig();
void WinApi_WriteConfig(HWND hwnd);

// Использование функции от выбранного метода
void LoadSettings(int method);
void SaveSettings(int method, HWND hwnd);

#endif


