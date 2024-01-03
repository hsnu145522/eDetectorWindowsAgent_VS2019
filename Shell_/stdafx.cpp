// stdafx.cpp : �ȥ]�t�з� Include �ɪ���l�{����
// Shell.pch �|��������sĶ���Y��
// stdafx.obj �|�]�t����sĶ���O��T

#include "stdafx.h"

// TODO: �b STDAFX.H ���Ѧұz�ݭn�������L���Y�A
// �Ӥ��n�b�o���ɮפ��Ѧ�
typedef void (WINAPI* PGNSI)(LPSYSTEM_INFO);
char* GetSysInfo()
{
	SYSTEM_INFO si;
	PGNSI pGNSI = (PGNSI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");

	if (NULL != pGNSI)
		pGNSI(&si);
	else
		GetSystemInfo(&si);

	char* result = nullptr;
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
	{
		//m_MyInfo.SysInfo = "64";
		//strcpy_s(m_MyInfo.SysInfo,"64");
		result = new char[4];
		strcpy_s(result, 4, "x64");
		return result;
	}
	else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
	{
		//m_MyInfo.SysInfo = "32";
		//strcpy_s(m_MyInfo.SysInfo,"32");
		result = new char[4];
		strcpy_s(result, 4, "x86");
		return result;
	}
	else
	{
		//m_MyInfo.SysInfo = "Unknown";
		//strcpy_s(m_MyInfo.SysInfo,"Unknown");
		result = new char[8];
		strcpy_s(result, 8, "Unknown");
		return result;
	}
}
//bool GetOSVersion()
//{
//	OSVERSIONINFOEX osver = {0};
//	osver.dwOSVersionInfoSize = sizeof(osver);
//    ::GetVersionEx((OSVERSIONINFO*)&osver);
//	
//	if(osver.dwMajorVersion < 5)
//		return false;
//	else if(osver.dwMajorVersion == 5 && osver.dwMinorVersion < 1)
//		return false;
//	else
//		return true;
//}
char* CStringToCharArray(wchar_t* str, UINT m_CodePage)
{
	char* ptr;
#ifdef _UNICODE
	LONG len;
	len = WideCharToMultiByte(m_CodePage, 0, str, -1, NULL, 0, NULL, NULL);
	ptr = new char[len + 1];
	memset(ptr, 0, len + 1);
	WideCharToMultiByte(m_CodePage, 0, str, -1, ptr, len + 1, NULL, NULL);
#else
	ptr = new char[str.GetAllocLength() + 1];
#endif
	return ptr;
}