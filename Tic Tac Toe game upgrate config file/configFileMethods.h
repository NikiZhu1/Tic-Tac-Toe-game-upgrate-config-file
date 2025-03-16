#ifndef CONFIGFILEMETHODS_H
#define CONFIGFILEMETHODS_H

#include <string>

// ����������� �� ������
void FileMap_ReadConfig();
void FileMap_WriteConfig(HWND hwnd);

// �������� ����������
void C_ReadConfig();
void C_WriteConfig(HWND hwnd);

// ������
void Fstream_ReadConfig();
void Fstream_WriteConfig(HWND hwnd);

// WinAPI
void WinApi_ReadConfig();
void WinApi_WriteConfig(HWND hwnd);

// ������������� ������� �� ���������� ������
void LoadSettings(int method);
void SaveSettings(int method, HWND hwnd);

#endif


