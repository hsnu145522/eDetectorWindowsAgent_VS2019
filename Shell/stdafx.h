
// stdafx.h : �i�b�����Y�ɤ��]�t�зǪ��t�� Include �ɡA
// �άO�g�`�ϥΫo�ܤ��ܧ�
// �M�ױM�� Include �ɮ�
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: �b���Ѧұz���{���һݭn����L���Y
#include <windows.h>
#include <TlHelp32.h>
#include <ShellAPI.h>

char* GetSysInfo();
//bool GetOSVersion();
char* CStringToCharArray(wchar_t* str, UINT m_CodePage);