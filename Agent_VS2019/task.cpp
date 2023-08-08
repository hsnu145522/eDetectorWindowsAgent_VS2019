#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <cstring>
#include <future>

#include "task.h"




Task::Task(Info* infoInstance, SocketSend* socketSendInstance) {
    functionMap["GiveInfo"] = std::bind(&Task::GiveInfo, this);
    functionMap["GiveDetectInfoFirst"] = std::bind(&Task::GiveDetectInfoFirst, this);
    functionMap["GiveDetectInfo"] = std::bind(&Task::GiveDetectInfo, this);
    functionMap["GiveDriveInfo"] = std::bind(&Task::GiveDriveInfo, this);
    functionMap["Explorer"] = std::bind(&Task::Explorer, this);
    functionMap["GiveExplorerData"] = std::bind(&Task::GiveExplorerData, this);
    functionMap["GiveExplorerEnd"] = std::bind(&Task::GiveExplorerEnd, this);
    functionMap["CollectInfo"] = std::bind(&Task::CollectInfo, this);
    functionMap["GiveCollectProgress"] = std::bind(&Task::GiveCollectProgress, this);
    functionMap["GiveCollectDataInfo"] = std::bind(&Task::GiveCollectDataInfo, this);
    functionMap["GiveCollectData"] = std::bind(&Task::GiveCollectData, this);
    functionMap["GiveCollectDataEnd"] = std::bind(&Task::GiveCollectDataEnd, this);

    // packet from server
    functionFromServerMap["OpenCheckthread"] = &Task::OpenCheckthread;
    functionFromServerMap["UpdateDetectMode"] = &Task::UpdateDetectMode;
    functionFromServerMap["GetScanInfoData"] = &Task::GetScanInfoData_;
    functionFromServerMap["GetProcessInfo"] = &Task::GetProcessInfo;
    functionFromServerMap["GetDrive"] = &Task::GetDrive;
    functionFromServerMap["TransportExplorer"] = &Task::TransportExplorer;
    functionFromServerMap["GetCollectInfo"] = &Task::GetCollectInfo;
    functionFromServerMap["GetCollectInfoData"] = &Task::GetCollectInfoData;
    functionFromServerMap["DataRight"] = &Task::DataRight;

    info = infoInstance;
    socketsend = socketSendInstance;
}

void Task::startThread(const std::string& key, std::string functionName) {
    std::any argument;
    auto functionIter = threadMap.find(functionName);
    if (functionIter == threadMap.end()) {
        threadMap[functionName] = std::thread(functionMap[functionName], this, argument);
    }
}

int Task::GiveInfo() {
    // getSystemInfo();
    char* buffer = new char[STRINGMESSAGELEN];
    char* SysInfo = tool.GetSysInfo();
    char* OsStr = tool.GetOSVersion();
    char* cComputerName = tool.GetComputerNameUTF8();
    char* cUserName = tool.GetUserNameUTF8();
    char* FileVersion = new char[10];
    unsigned long long BootTime = tool.GetBootTime();
    char* Key = new char[10];
    char* DigitalSignatureHash = new char[10];
    char* functionName = new char[24];

    strcpy_s(FileVersion, sizeof(FileVersion), "0.0.0.0");
	strcpy_s(Key, sizeof(Key), "");
	strcpy_s(DigitalSignatureHash, sizeof(DigitalSignatureHash), "123456");
	strcpy_s(functionName, 24, "GiveInfo");

    //if (strcpy_s(Key, sizeof(Key), "") == 0) printf("copy key success\n");
    //else printf("copy key failed\n");
    //if (strcpy_s(DigitalSignatureHash, sizeof(DigitalSignatureHash), "123456") == 0) printf("copy sign success\n");
    //else printf("copy sign failed\n");
    //if (strcpy_s(functionName, 24, "GiveInfo") == 0) printf("copy function success\n");
    //else printf("copy function failed\n");



    snprintf(buffer, STRINGMESSAGELEN, "%s|%s|%s|%s|%s,%d,%d|%d|%s|%lu", SysInfo, OsStr, cComputerName, cUserName, FileVersion, 1988, 1989, BootTime, Key, DigitalSignatureHash);
    
    return socketsend->SendMessageToServer(functionName, buffer);
}

int Task::CheckConnect() {

     //while(true){
     //    std::this_thread::sleep_for(std::chrono::seconds(2));
     //    if (!socketsend->SendMessageToServer("CheckConnect", "")) {
     //        printf("CheckConnect sent failed\n");
     //    } else {
     //        printf("CheckConnect sent\n");
     //    }
     //}

    // to do
    // open a thread to send it forever
    // check kill time

    return 0;
}

int Task::GiveDetectInfoFirst() {
    char* buff = new char[STRINGMESSAGELEN];
    char* functionName = new char[24];
    strcpy_s(functionName, 24, "GiveDetectInfoFirst\0");
    snprintf(buff, STRINGMESSAGELEN, "%d|%d", info->DetectProcess, info->DetectNetwork);
    return socketsend->SendMessageToServer(functionName, buff);
}

int Task::GiveDetectInfo() {
    char* buff = new char[STRINGMESSAGELEN];
    char* functionName = new char[24];
    strcpy_s(functionName, 24, "GiveDetectInfo");
    snprintf(buff, STRINGMESSAGELEN, "%d|%d", info->DetectProcess, info->DetectNetwork);
	int ret = socketsend->SendMessageToServer(functionName, buff);

	// test
	//GiveProcessData();
	DetectProcess();
	//GiveDriveInfo();
	//GiveExplorerData();

	//StrPacket* tmp = new StrPacket;
	//GetScan(tmp);

    return 1;
}

int Task::DetectProcess() {

	printf("sending DetectProcess\n");
	MemProcess* m_MemPro = new MemProcess;
	set<DWORD> m_ApiName;
	tool.LoadApiPattern(&m_ApiName);
	DWORD pMainProcessid = GetCurrentProcessId();

	try {
		DetectProcessRisk(pMainProcessid, false, &m_ApiName, info->tcpSocket);
	}
	catch (...) {}
	m_ApiName.clear();
	delete m_MemPro;
	return 1;
}


int Task::GetScan(StrPacket* udata) {

	SOCKET* tcpSocket = CreateNewSocket();
	if (tcpSocket == nullptr) return 0;

	//std::thread GiveProcessDataThread([&]() { GiveProcessData(tcpSocket); });
	//GiveProcessDataThread.join();

	return GiveProcessData(info->tcpSocket);
	//return 1;

}
int Task::GiveProcessData(SOCKET* tcpSocket) {
	printf("sending GiveProcessData...\n");

	char* Scan = new char[5];
	strcpy_s(Scan, 5, "Scan");

    std::set<DWORD> m_ApiName;
    tool.LoadApiPattern(&m_ApiName);
    std::map<DWORD, ProcessInfoData> m_ProcessInfo;
    std::vector<UnKnownDataInfo> m_UnKnownData;
    MemProcess* m_MemPro = new MemProcess;

	printf("start scan...\n");
    m_MemPro->ScanRunNowProcess(this, &m_ProcessInfo, &m_ApiName, &m_UnKnownData);
	printf("finish scan...\n");

	if (!m_ProcessInfo.empty()) {
		try {
			GiveScanDataSendServer(info->MAC, info->IP, Scan, &m_ProcessInfo, &m_UnKnownData, tcpSocket);
		}
		catch (...) {
			printf("GiveScanDataSendServer has failed.\n");
		}
		
	}

	delete m_MemPro;
    m_UnKnownData.clear();
    m_ProcessInfo.clear();
    m_ApiName.clear();
    int ret = 1;
    return ret;

}
void Task::GiveScanDataSendServer(char* pMAC, char* pIP, char* pMode, map<DWORD, ProcessInfoData>* pFileInfo, vector<UnKnownDataInfo>* pUnKnownData, SOCKET* tcpSocket)
{
	char* buff = new char[DATASTRINGMESSAGELEN];
	map<DWORD, ProcessInfoData>::iterator vit;
	int AllCount = (int)pFileInfo->size();
	int m_Count = 0;

	sprintf_s(buff, DATASTRINGMESSAGELEN, "%d", AllCount);
	if (!GiveScanInfo(buff, tcpSocket)) {
		printf("data info send failed\n");
		return;
	}

	int ret = 0;
	for (vit = pFileInfo->begin(); vit != pFileInfo->end(); vit++)
	{
		if (_tcscmp(vit->second.ProcessHash, _T("null")))
		{
			//std::this_thread::sleep_for(std::chrono::seconds(2));
			wchar_t* wTempStr = new wchar_t[DATASTRINGMESSAGELEN];

			// command line
			HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, vit->first);
			MemProcess* m_MemPro = new MemProcess;
			TCHAR* Comstr = new TCHAR[MAX_PATH_EX];
			DWORD ret1 = m_MemPro->GetRemoteCommandLineW(processHandle, Comstr, MAX_PATH_EX);
			if (ret1 == 0) _tcscpy_s(Comstr, MAX_PATH_EX, _T(""));
			CloseHandle(processHandle);

			// parent name, parent path
			TCHAR* ParentName = new TCHAR[MAX_PATH];
			TCHAR* ParentPath = new TCHAR[MAX_PATH];
			_tcscpy_s(ParentName, 259, _T("null"));
			_tcscpy_s(ParentPath, 259, _T("null"));
			auto it = pFileInfo->find(vit->second.ParentID);
			if (it != pFileInfo->end()) {
				_tcscpy_s(ParentName, MAX_PATH, it->second.ProcessName);
				_tcscpy_s(ParentPath, MAX_PATH, it->second.ProcessPath);
			}

			swprintf_s(wTempStr, DATASTRINGMESSAGELEN, L"%s|%s|%s|%s|%s|%ld|%s|%s|%s|%ld|%d,%d|%d|%d|%d,%d"
				, vit->second.ProcessName, vit->second.ProcessCTime, Comstr, vit->second.ProcessHash, vit->second.ProcessPath,
				vit->second.ParentID, ParentName, ParentPath, vit->second.SignerSubjectName, vit->first, vit->second.InjectionPE, vit->second.InjectionOther
				, vit->second.Injected, vit->second.StartRun, vit->second.HideProcess, vit->second.HideAttribute
			);



			

			// abnormal dll
			char* cTempStr = CStringToCharArray(wTempStr, CP_UTF8);
			strcpy_s(buff, DATASTRINGMESSAGELEN, cTempStr);
			
			//delete[] wTempStr;
			if (!vit->second.Abnormal_dll.empty())
			{
				strcat_s(buff, DATASTRINGMESSAGELEN, "|");
				set<string>::iterator dllit;
				for (dllit = vit->second.Abnormal_dll.begin(); dllit != vit->second.Abnormal_dll.end(); dllit++)
				{
					char* dllstr = new char[4096];
					sprintf_s(dllstr, 4096, "%s;", (*dllit).c_str());
					if ((strlen(dllstr) + strlen(buff)) >= DATASTRINGMESSAGELEN)
					{
						ret = GiveScan(buff, tcpSocket);
						memset(buff, '\0', DATASTRINGMESSAGELEN);
						if (ret <= 0)
						{
							delete[] dllstr;
							break;
						}
					}
					strcat_s(buff, DATASTRINGMESSAGELEN, dllstr);
					delete[] dllstr;
				}
				if (ret <= 0)
					break;
			}
			else
				strcat_s(buff, DATASTRINGMESSAGELEN, "|null");

			// inline hook
			if (!vit->second.InlineHookInfo.empty())
			{
				strcat_s(buff, DATASTRINGMESSAGELEN, "|");
				set<string>::iterator Inlineit;
				for (Inlineit = vit->second.InlineHookInfo.begin(); Inlineit != vit->second.InlineHookInfo.end(); Inlineit++)
				{
					char* Inlinestr = new char[4096];
					sprintf_s(Inlinestr, 4096, "%s;", (*Inlineit).c_str());
					if ((strlen(Inlinestr) + strlen(buff)) >= DATASTRINGMESSAGELEN)
					{
						ret = GiveScan(buff, tcpSocket);
						memset(buff, '\0', DATASTRINGMESSAGELEN);
						if (ret <= 0)
						{
							delete[] Inlinestr;
							break;
						}
					}
					strcat_s(buff, DATASTRINGMESSAGELEN, Inlinestr);
					delete[] Inlinestr;
				}
				if (ret <= 0)
					break;
			}
			else
				strcat_s(buff, DATASTRINGMESSAGELEN, "|null");

			delete[] ParentName;
			delete[] ParentPath;
			delete[] Comstr;
			delete[] cTempStr;



			/*if (!vit->second.NetString.empty())
			{
				strcat_s(TempStr, DATASTRINGMESSAGELEN, "|");
				set<string>::iterator netit;
				for (netit = vit->second.NetString.begin(); netit != vit->second.NetString.end(); netit++)
				{
					char* netstr = new char[4096];
					sprintf_s(netstr, 4096, "%s;", (*netit).c_str());
					if ((strlen(netstr) + strlen(TempStr)) >= DATASTRINGMESSAGELEN)
					{
						ret = socketsend->SendDataToServer(functionName_GiveScanData, TempStr);
						memset(TempStr, '\0', DATASTRINGMESSAGELEN);
						if (ret <= 0)
						{
							delete[] netstr;
							break;
						}
					}
					strcat_s(TempStr, DATASTRINGMESSAGELEN, netstr);
					delete[] netstr;
				}
				if (ret <= 0)
					break;
			}
			else
				strcat_s(TempStr, DATASTRINGMESSAGELEN, "|null");*/


			ret = GiveScan(buff, tcpSocket);
			if (ret <= 0) break;
			else memset(buff, '\0', DATASTRINGMESSAGELEN);

			//break;
		}
		m_Count++;
	}

	//m_Hash.clear();

	//if (!pUnKnownData->empty())
	//{
	//	printf("pUnKnownData\n");
	//	vector<UnKnownDataInfo>::iterator ut;
	//	memset(TempStr, '\0', DATASTRINGMESSAGELEN);
	//	wchar_t* wUnKownInfoStr = new wchar_t[DATASTRINGMESSAGELEN];
	//	int ret = 1;
	//	char* cUnKownInfoStr = NULL;
	//	for (ut = pUnKnownData->begin(); ut != pUnKnownData->end(); ut++)
	//	{
	//		swprintf_s(wUnKownInfoStr, DATASTRINGMESSAGELEN, L"%lu|%s|%d", (*ut).Pid, (*ut).ProcessName, (*ut).SizeInfo);
	//		cUnKownInfoStr = CStringToCharArray(wUnKownInfoStr, CP_UTF8);
	//		sprintf_s(TempStr, DATASTRINGMESSAGELEN, "%s", cUnKownInfoStr);
	//		ret = socketsend->SendMessageToServer(functionName_GiveProcessUnknownInfo, TempStr);
	//		if (ret <= 0)
	//			break;
	//		memset(TempStr, '\0', DATASTRINGMESSAGELEN);
	//		if ((*ut).SizeInfo > DATASTRINGMESSAGELEN /*&& ret != -3*/)
	//		{
	//			int tmplen = (*ut).SizeInfo;
	//			for (DWORD i = 0; i < (*ut).SizeInfo; i += DATASTRINGMESSAGELEN)
	//			{
	//				char* TmpBuffer = new char[DATASTRINGMESSAGELEN];
	//				memset(TmpBuffer, '\x00', DATASTRINGMESSAGELEN);
	//				if (tmplen < DATASTRINGMESSAGELEN)
	//					memcpy(TmpBuffer, (*ut).Data + i, tmplen);
	//				else
	//				{
	//					memcpy(TmpBuffer, (*ut).Data + i, DATASTRINGMESSAGELEN);
	//					tmplen -= DATASTRINGMESSAGELEN;
	//				}
	//				ret = socketsend->SendMessageToServer(functionName_GiveProcessUnknownInfo, TmpBuffer);
	//				//Sendret = m_Client->SendDataBufToServer(pInfo->MAC,pInfo->IP,WorkStr,TmpBuffer);
	//				delete[] TmpBuffer;
	//				if (ret <= 0)
	//				{
	//					break;
	//				}
	//			}
	//			if (ret <= 0)
	//				break;
	//		}
	//		else
	//		{
	//			char* TmpBuffer = new char[DATASTRINGMESSAGELEN];
	//			memset(TmpBuffer, '\x00', DATASTRINGMESSAGELEN);
	//			memcpy(TmpBuffer, (*ut).Data, (*ut).SizeInfo);
	//			ret = socketsend->SendMessageToServer(functionName_GiveProcessUnknownInfo, TmpBuffer);
	//			//Sendret = m_Client->SendDataBufToServer(pInfo->MAC,pInfo->IP,WorkStr,TmpBuffer);
	//			delete[] TmpBuffer;
	//			if (ret <= 0)
	//				break;
	//		}
	//		
	//		ret = socketsend->SendMessageToServer(functionName_GiveProcessUnknownEnd, null);
	//		if (ret <= 0)
	//			break;
	//		delete[] cUnKownInfoStr;
	//		cUnKownInfoStr = NULL;
	//	}
	//	if (cUnKownInfoStr != NULL)
	//		delete[] cUnKownInfoStr;
	//	delete[] wUnKownInfoStr;
	//	for (ut = pUnKnownData->begin(); ut != pUnKnownData->end(); ut++)
	//	{
	//		delete[](*ut).Data;
	//	}
	//}

	delete[] buff;
	ret = GiveScanDataEnd(pMode, tcpSocket);
}
int Task::GiveScanInfo(char* buff, SOCKET* tcpSocket) {
	char* functionName = new char[24];
	strcpy_s(functionName, 24, "GiveScanInfo");
	return socketsend->SendDataToServer(functionName, buff, tcpSocket);
}
int Task::GiveScan(char* buff, SOCKET* tcpSocket) {
	char* functionName = new char[24];
	strcpy_s(functionName, 24, "GiveScan");
	return socketsend->SendDataToServer(functionName, buff, tcpSocket);
}
int Task::GiveScanDataEnd(char* buff, SOCKET* tcpSocket) {
	char* functionName = new char[24];
	strcpy_s(functionName, 24, "GiveScanDataEnd");
	return socketsend->SendDataToServer(functionName, buff, tcpSocket);
}


int Task::GiveDriveInfo() { 
	char* m_DriveInfo = GetMyPCDrive();
	char* functionName = new char[24];
	strcpy_s(functionName, 24, "GiveDriveInfo");
	int ret = socketsend->SendMessageToServer(functionName, m_DriveInfo);
	return ret;
}

int Task::Explorer() { return 0; }
int Task::GiveExplorerData() {

	char* functionName_GiveExplorerData = new char[24];
	strcpy_s(functionName_GiveExplorerData, 24, "GiveExplorerData");
	char* functionName_GiveExplorerEnd = new char[24];
	strcpy_s(functionName_GiveExplorerEnd, 24, "GiveExplorerEnd");
	char* functionName_GiveExplorerError = new char[24];
	strcpy_s(functionName_GiveExplorerError, 24, "GiveExplorerError");
	char* ErrorLoadingMFTTable = new char[22];
	strcpy_s(ErrorLoadingMFTTable, 22, "ErrorLoadingMFTTable");
	char* ErrorLoadingFATTable = new char[22];
	strcpy_s(ErrorLoadingFATTable, 22, "ErrorLoadingFATTable");
	char* ErrorNotFormat = new char[22];
	strcpy_s(ErrorNotFormat, 22, "ErrorNotFormat");
	char* ErrorNoDrive = new char[22];
	strcpy_s(ErrorNoDrive, 22, "ErrorNoDrive");
	char* null = new char[5];
	strcpy_s(null, 5, "null");

	
	
	//



	//wchar_t* wMgs = CharArrayToWString(Mgs->csMsg, CP_UTF8);
	ExplorerInfo* m_Info = new ExplorerInfo;
	/*CFileSystem* pfat;
	LoadExplorerInfo(wMgs, m_Info);*/
	m_Info->Drive = L'C';
	wcscpy_s(m_Info->DriveInfo, L"NTFS");

	wchar_t* drive = new wchar_t[5];
	swprintf_s(drive, 5, L"%c:\\", m_Info->Drive);
	wchar_t* volname = new wchar_t[_MAX_FNAME];
	wchar_t* filesys = new wchar_t[_MAX_FNAME];
	DWORD VolumeSerialNumber, MaximumComponentLength, FileSystemFlags;
	if (GetVolumeInformation(drive, volname, _MAX_FNAME, &VolumeSerialNumber, &MaximumComponentLength, &FileSystemFlags, filesys, _MAX_FNAME))
	{
		if ((wcsstr(m_Info->DriveInfo, filesys) != 0))
		{
			if (!wcscmp(filesys, L"NTFS"))
			{
				
				NTFSSearchCore* searchCore = new NTFSSearchCore;
				int ret = 0;
				try
				{
					printf("NTFS start\n");
					ret = NTFSSearch(m_Info->Drive, info->MAC, info->IP);
				}
				catch (...)
				{
					ret = 1;
				}
				if (ret == 0)
				{
					int	ret = socketsend->SendMessageToServer(functionName_GiveExplorerEnd, null);
				}
				else
				{
					ret = socketsend->SendMessageToServer(functionName_GiveExplorerError, ErrorLoadingMFTTable);
				}
				delete searchCore;
			}
			else if (!wcscmp(filesys, L"FAT32"))
			{
				//int ret1 = 1;
				//char* TempStr = new char[DATASTRINGMESSAGELEN];
				//memset(TempStr, '\0', DATASTRINGMESSAGELEN);
				//char* m_DataStr = new char[1000];
				//sprintf_s(m_DataStr, 1000, "5|.|5|0|2|1970/01/01 08:00:00|1970/01/01 08:00:00|1970/01/01 08:00:00|null,null,null|0|1\n");
				//strcat_s(TempStr, DATASTRINGMESSAGELEN, m_DataStr);
				//vector<DeleteFATFileInfo> FATDeleteFile;
				//DWORD LastCluster = 0;
				//unsigned int Count = 1;
				//unsigned int ProgressCount = 1;
				//clock_t start;
				//start = clock();
				//bool ret = pfat->initFDT(this, info->MAC, info->IP, TempStr, ProgressCount, Count, LastCluster, &FATDeleteFile, start);
				//if (ret)
				//{
				//	if (!FATDeleteFile.empty())
				//	{
				//		vector<DeleteFATFileInfo>::iterator it;
				//		for (it = FATDeleteFile.begin(); it != FATDeleteFile.end(); it++)
				//		{
				//			LastCluster++;
				//			if (LastCluster == 5)
				//				LastCluster++;
				//			wchar_t* wstr = new wchar_t[1024];
				//			DWORD FirstClister = (*it).FirstDataCluster + 5;
				//			if ((*it).isDirectory == 0)
				//			{
				//				TCHAR* m_MD5Str = new TCHAR[50];
				//				memset(m_MD5Str, '\0', 50);
				//				TCHAR* Signaturestr = new TCHAR[20];
				//				memset(Signaturestr, '\0', 20);
				//				//DWORD FirstCluster = newEntry->GetTheFirstDataCluster()+5;
				//				if (pfat->FileHashAndSignature((*it).FirstDataCluster, (*it).FileSize, (*it).FileName, m_MD5Str, Signaturestr))
				//				{
				//					swprintf_s(wstr, 1024, L"%lu|%s|%lu|1|%d|%02hu/%02hu/%02hu %02hu:%02hu:%02hu|%02hu/%02hu/%02hu %02hu:%02hu:%02hu|%02hu/%02hu/%02hu %02hu:%02hu:%02hu|%s,%s,%lu|%lu|1\n",
				//						LastCluster, (*it).FileName, (*it).ParentFirstDataCluster, (*it).isDirectory
				//						, (*it).CT.wYear, (*it).CT.wMonth, (*it).CT.wDay, (*it).CT.wHour, (*it).CT.wMinute, (*it).CT.wSecond,
				//						(*it).WT.wYear, (*it).WT.wMonth, (*it).WT.wDay, (*it).WT.wHour, (*it).WT.wMinute, (*it).WT.wSecond,
				//						(*it).AT.wYear, (*it).AT.wMonth, (*it).AT.wDay, (*it).AT.wHour, (*it).AT.wMinute, (*it).AT.wSecond, m_MD5Str, Signaturestr, FirstClister, (*it).FileSize);
				//				}
				//				else
				//				{
				//					swprintf_s(wstr, 1024, L"%lu|%s|%lu|1|%d|%02hu/%02hu/%02hu %02hu:%02hu:%02hu|%02hu/%02hu/%02hu %02hu:%02hu:%02hu|%02hu/%02hu/%02hu %02hu:%02hu:%02hu|null,null,%lu|%lu|1\n",
				//						LastCluster, (*it).FileName, (*it).ParentFirstDataCluster, (*it).isDirectory
				//						, (*it).CT.wYear, (*it).CT.wMonth, (*it).CT.wDay, (*it).CT.wHour, (*it).CT.wMinute, (*it).CT.wSecond,
				//						(*it).WT.wYear, (*it).WT.wMonth, (*it).WT.wDay, (*it).WT.wHour, (*it).WT.wMinute, (*it).WT.wSecond,
				//						(*it).AT.wYear, (*it).AT.wMonth, (*it).AT.wDay, (*it).AT.wHour, (*it).AT.wMinute, (*it).AT.wSecond, FirstClister, (*it).FileSize);
				//				}
				//				delete[] Signaturestr;
				//				delete[] m_MD5Str;
				//			}
				//			else
				//			{
				//				swprintf_s(wstr, 1024, L"%lu|%s|%lu|1|%d|%02hu/%02hu/%02hu %02hu:%02hu:%02hu|%02hu/%02hu/%02hu %02hu:%02hu:%02hu|%02hu/%02hu/%02hu %02hu:%02hu:%02hu|null,null,%lu|%lu|1\n",
				//					LastCluster, (*it).FileName, (*it).ParentFirstDataCluster, (*it).isDirectory
				//					, (*it).CT.wYear, (*it).CT.wMonth, (*it).CT.wDay, (*it).CT.wHour, (*it).CT.wMinute, (*it).CT.wSecond,
				//					(*it).WT.wYear, (*it).WT.wMonth, (*it).WT.wDay, (*it).WT.wHour, (*it).WT.wMinute, (*it).WT.wSecond,
				//					(*it).AT.wYear, (*it).AT.wMonth, (*it).AT.wDay, (*it).AT.wHour, (*it).AT.wMinute, (*it).AT.wSecond, FirstClister, (*it).FileSize);
				//			}
				//			char* m_DataStr = CStringToCharArray(wstr, CP_UTF8);
				//			strcat_s(TempStr, DATASTRINGMESSAGELEN, m_DataStr);
				//			ProgressCount++;
				//			Count++;
				//			clock_t endTime = clock();
				//			if ((endTime - start) > 300000)
				//			{
				//				char* ProgressStr = new char[10];
				//				sprintf_s(ProgressStr, 10, "%u", ProgressCount);
				//				strcat_s(TempStr, DATASTRINGMESSAGELEN, ProgressStr);
				//				ret1 = socketsend->SendDataToServer(functionName_GiveExplorerData, TempStr);
				//				if (ret1 <= 0)
				//				{
				//					delete[] ProgressStr;
				//					delete[] m_DataStr;
				//					delete[] wstr;
				//					break;
				//				}
				//				start = clock();
				//				Count = 0;
				//				memset(TempStr, '\0', DATASTRINGMESSAGELEN);
				//				delete[] ProgressStr;
				//			}
				//			else
				//			{
				//				if ((Count % 60) == 0 && Count >= 60)
				//				{
				//					char* ProgressStr = new char[10];
				//					sprintf_s(ProgressStr, 10, "%u", ProgressCount);
				//					strcat_s(TempStr, DATASTRINGMESSAGELEN, ProgressStr);
				//					ret1 = socketsend->SendDataToServer(functionName_GiveExplorerData, TempStr);
				//					if (ret1 <= 0)
				//					{
				//						delete[] ProgressStr;
				//						delete[] m_DataStr;
				//						delete[] wstr;
				//						break;
				//					}
				//					start = clock();
				//					Count = 0;
				//					memset(TempStr, '\0', DATASTRINGMESSAGELEN);
				//					delete[] ProgressStr;
				//				}
				//			}
				//			delete[] m_DataStr;
				//			delete[] wstr;
				//		}
				//	}
				//	if (ret1 > 0)
				//	{
				//		if (TempStr[0] != '\0')
				//		{
				//			char* ProgressStr = new char[10];
				//			sprintf_s(ProgressStr, 10, "%u", ProgressCount);
				//			strcat_s(TempStr, DATASTRINGMESSAGELEN, ProgressStr);
				//			ret1 = socketsend->SendDataToServer(functionName_GiveExplorerData, TempStr);
				//			delete[] ProgressStr;
				//		}
				//	}
				//	if (ret1 > 0)
				//		int	ret = socketsend->SendMessageToServer(functionName_GiveExplorerEnd, null);
				//}
				//else
				//	ret = socketsend->SendMessageToServer(functionName_GiveExplorerError, ErrorLoadingFATTable);
				//FATDeleteFile.clear();
				//delete[] m_DataStr;
				//delete[] TempStr;
			}
			else
			{
				//char* TempStr = new char[DATASTRINGMESSAGELEN];
				//memset(TempStr, '\0', DATASTRINGMESSAGELEN);
				//char* m_DataStr = new char[1000];
				//sprintf_s(m_DataStr, 1000, "5|.|5|0|2|1970/01/01 08:00:00|1970/01/01 08:00:00|1970/01/01 08:00:00|null|0|9\n");
				//strcat_s(TempStr, DATASTRINGMESSAGELEN, m_DataStr);
				////wchar_t * DriveStr = CharArrayToWString(drive,CP_UTF8);
				//unsigned int ProgressCount = 1;
				//unsigned int Index = 5;
				//unsigned int Count = 1;
				//int ret = 1;
				//SysExplorerSearch(drive, 5, Index, TempStr, ProgressCount, Count);
				//if (TempStr[0] != '\0')
				//{
				//	char* ProgressStr = new char[10];
				//	sprintf_s(ProgressStr, 10, "%u", ProgressCount);
				//	strcat_s(TempStr, DATASTRINGMESSAGELEN, ProgressStr);
				//	ret = socketsend->SendDataToServer(functionName_GiveExplorerData, TempStr);
				//	delete[] ProgressStr;
				//}
				////if(Client_Socket->IsOpened())
				//if (ret > 0)
				//	int	ret = socketsend->SendMessageToServer(functionName_GiveExplorerEnd, null);
				//delete[] m_DataStr;
				//delete[] TempStr;
			}
		}
		else
		{	
			int ret;
			ret = socketsend->SendMessageToServer(functionName_GiveExplorerError, ErrorNotFormat);
			
		}
	}
	else
	{
		int ret;
		ret = socketsend->SendMessageToServer(functionName_GiveExplorerError, ErrorNoDrive);
	}
	delete[] filesys;
	delete[] volname;
	delete[] drive;
	delete m_Info;
	//delete[] wMgs;
    return 0;

}
int Task::GiveExplorerEnd() { return 0; }
int Task::CollectInfo() { return 0; }
int Task::GiveCollectProgress() { return 0; }
int Task::GiveCollectDataInfo() { return 0; }
int Task::GiveCollectData() { 
	
	return 0;
}
int Task::GiveCollectDataEnd() { return 0; }


int Task::OpenCheckthread(StrPacket* udata) {
    // strcpy(UUID,udata->csMsg);
    // GiveDetectInfoFirst();

     //std::thread CheckConnectThread(&Task::CheckConnect, this);
     //CheckConnectThread.join();

    // store key into registry

    return GiveDetectInfoFirst();

}

int Task::UpdateDetectMode(StrPacket* udata) {

    std::vector<std::string>DetectMode = tool.SplitMsg(udata->csMsg);
    for (int i = 0; i < DetectMode.size(); i++) {
        if (i == 0) info->DetectProcess = DetectMode[i][0] - '0';
        else if (i == 1) info->DetectNetwork = DetectMode[i][0] - '0';
        else printf("UpdateDetectMode parse failed\n");
    }
    return GiveDetectInfo();

}

int Task::GetProcessInfo(StrPacket* udata) { return 0; }
int Task::GetDrive(StrPacket* udata) { return 0; }
int Task::GetScanInfoData_(StrPacket* udata) { return 1; }




//int Task::ExplorerInfo(StrPacket* udata) { return 0; }
int Task::TransportExplorer(StrPacket* udata) { return 0; }
int Task::GetCollectInfo(StrPacket* udata) { return 0; }
int Task::GetCollectInfoData(StrPacket* udata) { return 0; }
int Task::DataRight(StrPacket* udata) { return 0; }

SOCKET* Task::CreateNewSocket() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "Failed to initialize Winsock." << std::endl;
		return nullptr;
	}

	SOCKET tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tcpSocket == INVALID_SOCKET) {
		std::cerr << "Error creating TCP socket: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return nullptr;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(info->Port);
	serverAddr.sin_addr.s_addr = inet_addr(info->ServerIP);
	//serverAddr.sin_addr.s_addr = inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

	if (connect(tcpSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Error connecting to server: " << WSAGetLastError() << std::endl;
		closesocket(tcpSocket);
		WSACleanup();
		return nullptr;
	}

	return &tcpSocket;
}

int Task::NTFSSearch(wchar_t vol_name, char* pMAC, char* pIP) {

	//char* functionName_GiveExplorerData = new char[24];
	//strcpy_s(functionName_GiveExplorerData, 24, "GiveExplorerData");

	//CNTFSVolume* m_curSelectedVol = new CNTFSVolume(vol_name);
	//if (m_curSelectedVol == NULL)
	//{
	//	printf("Error when getVolumeByName\n");
	//	delete m_curSelectedVol;
	//	return 1;
	//}

	//if (!m_curSelectedVol->IsVolumeOK())
	//{
	//	printf("Not a valid NTFS volume or NTFS version < 3.0\n");
	//	delete m_curSelectedVol;
	//	return 1;
	//}

	//unsigned int m_progressIdx;
	//unsigned int m_Count = 0;
	//char* TempStr = new char[DATASTRINGMESSAGELEN];
	//memset(TempStr, '\0', DATASTRINGMESSAGELEN);
	//printf("GetRecordsCount\n");
	//for (m_progressIdx = MFT_IDX_MFT; m_progressIdx < m_curSelectedVol->GetRecordsCount(); m_progressIdx++)
	//{
	//	printf("for loop start\n");
	//	CFileRecord* fr = new CFileRecord(m_curSelectedVol);
	//	if (fr == NULL) {
	//		printf("CFileRecord is null\n");
	//		continue;	// skip to next
	//	}

	//	// Only parse Standard Information and File Name attributes
	//	printf("SetAttrMask\n");
	//	fr->SetAttrMask(MASK_FILE_NAME | MASK_DATA);	// StdInfo will always be parsed
	//	printf("ParseFileRecord\n");
	//	if (!fr->ParseFileRecord(m_progressIdx))
	//	{
	//		delete fr;
	//		continue;	// skip to next
	//	}

	//	printf("ParseFileAttrs\n");
	//	if (!fr->ParseFileAttrs())
	//	{
	//		delete fr;
	//		continue;	// skip to next
	//	}

	//	printf("GetFileName\n");
	//	TCHAR fn[MAX_PATH];
	//	if (fr->GetFileName(fn, MAX_PATH) <= 0)
	//	{
	//		delete fr;
	//		continue;	// skip to next
	//	}

	//	ULONGLONG datalen = 0;

	//	printf("IsDirectory\n");
	//	if (!fr->IsDirectory())
	//	{
	//		const CAttrBase* data = fr->FindStream();

	//		//if(data)
	//		//{
	//		//	datalen = data->GetDataSize();
	//		//		//delete data;
	//		//}
	//		if (data)
	//		{
	//			datalen = data->GetDataSize();
	//			if (fr->IsCompressed() && datalen == 0)
	//				datalen = fr->GetFileSize();
	//		}
	//		else
	//		{
	//			if (fr->IsCompressed() && datalen == 0)
	//				datalen = fr->GetFileSize();
	//		}
	//	}
	//	ULONGLONG ParentId = 0;
	//	ParentId = fr->GetParentRef();
	//	if (ParentId == 0)
	//		ParentId = 5;
	//	else
	//		ParentId = ParentId & 0x0000FFFFFFFFFFFF;
	//	FILETIME	FileCreateTime;		// File creation time
	//	FILETIME	FileWriteTime;		// File altered time
	//	FILETIME	FileAccessTime;		// File read time
	//	FILETIME	EntryModifiedTime;
	//	fr->GetFileCreateTime(&FileCreateTime);
	//	fr->GetFileWriteTime(&FileWriteTime);
	//	fr->GetFileAccessTime(&FileAccessTime);
	//	fr->GetEntryModifiedTime(&EntryModifiedTime);
	//	SYSTEMTIME systemCreateTime;
	//	SYSTEMTIME systemWriteTime;
	//	SYSTEMTIME systemAccessTime;
	//	SYSTEMTIME systemModifiedTime;
	//	FileTimeToSystemTime(&FileCreateTime, &systemCreateTime);
	//	FileTimeToSystemTime(&FileWriteTime, &systemWriteTime);
	//	FileTimeToSystemTime(&FileAccessTime, &systemAccessTime);
	//	FileTimeToSystemTime(&EntryModifiedTime, &systemModifiedTime);
	//	wchar_t CreateTimeWstr[50];
	//	wchar_t WriteTimeWstr[50];
	//	wchar_t AccessTimeWstr[50];
	//	wchar_t EntryModifiedTimeWstr[50];
	//	swprintf_s(CreateTimeWstr, 50, L"%02hu/%02hu/%02hu %02hu:%02hu:%02hu", systemCreateTime.wYear, systemCreateTime.wMonth, systemCreateTime.wDay, systemCreateTime.wHour, systemCreateTime.wMinute, systemCreateTime.wSecond);
	//	swprintf_s(WriteTimeWstr, 50, L"%02hu/%02hu/%02hu %02hu:%02hu:%02hu", systemWriteTime.wYear, systemWriteTime.wMonth, systemWriteTime.wDay, systemWriteTime.wHour, systemWriteTime.wMinute, systemWriteTime.wSecond);
	//	swprintf_s(AccessTimeWstr, 50, L"%02hu/%02hu/%02hu %02hu:%02hu:%02hu", systemAccessTime.wYear, systemAccessTime.wMonth, systemAccessTime.wDay, systemAccessTime.wHour, systemAccessTime.wMinute, systemAccessTime.wSecond);
	//	if (EntryModifiedTime.dwLowDateTime != 0)
	//		swprintf_s(EntryModifiedTimeWstr, 50, L"%02hu/%02hu/%02hu %02hu:%02hu:%02hu", systemModifiedTime.wYear, systemModifiedTime.wMonth, systemModifiedTime.wDay, systemModifiedTime.wHour, systemModifiedTime.wMinute, systemModifiedTime.wSecond);
	//	else
	//		swprintf_s(EntryModifiedTimeWstr, 50, L"1");
	//	wchar_t* wstr = new wchar_t[1024];
	//	swprintf_s(wstr, 1024, L"%u|%s|%llu|%d|%d|%s|%s|%s|%s|%llu|0\n", m_progressIdx, fn, ParentId, fr->IsDeleted(), fr->IsDirectory(), CreateTimeWstr, WriteTimeWstr, AccessTimeWstr, EntryModifiedTimeWstr, datalen);
	//	//wprintf(L"%s\n",wstr);
	//	char* m_DataStr = CStringToCharArray(wstr, CP_UTF8);
	//	strcat_s(TempStr, DATASTRINGMESSAGELEN, m_DataStr);
	//	//int ret = m_Client->SendDataMsgToServer(pMAC,pIP,"GiveExplorerData",m_DataStr);
	//	delete[] wstr;
	//	if ((m_Count % 60) == 0 && m_Count >= 60)
	//	{
	//		char* ProgressStr = new char[10];
	//		sprintf_s(ProgressStr, 10, "%u", m_progressIdx);
	//		strcat_s(TempStr, DATASTRINGMESSAGELEN, ProgressStr);
	//		int	ret = socketsend->SendDataToServer(functionName_GiveExplorerData, TempStr);
	//		if (ret == 0 || ret == -1)
	//		{
	//			delete[] ProgressStr;
	//			delete[] m_DataStr;
	//			delete[] TempStr;
	//			delete fr;
	//			delete m_curSelectedVol;
	//			return 1;
	//		}
	//		memset(TempStr, '\0', DATASTRINGMESSAGELEN);
	//		delete[] ProgressStr;
	//	}
	//	//if(ret == 0 || ret == -1)
	//	//{
	//	//	delete [] m_DataStr;
	//	//	delete fr;
	//	//	delete m_curSelectedVol;
	//	//	return 1;
	//	//}
	//	m_Count++;
	//	delete[] m_DataStr;
	//	delete fr;
	//}
	//if (TempStr[0] != '\0')
	//{
	//	char* ProgressStr = new char[10];
	//	sprintf_s(ProgressStr, 10, "%u", m_progressIdx);
	//	strcat_s(TempStr, DATASTRINGMESSAGELEN, ProgressStr);
	//	int	ret = socketsend->SendDataToServer(functionName_GiveExplorerData, TempStr);
	//	if (ret == 0 || ret == -1)
	//	{
	//		delete[] ProgressStr;
	//		delete[] TempStr;
	//		delete m_curSelectedVol;
	//		return 1;
	//	}
	//	delete[] ProgressStr;
	//}
	//delete[] TempStr;
	//delete m_curSelectedVol;

	return 0;
}

int Task::DetectProcessRisk(int pMainProcessid, bool IsFirst, set<DWORD>* pApiName, SOCKET* tcpSocket)
{
	char* functionName_GiveDetectProcessEnd = new char[24];
	strcpy_s(functionName_GiveDetectProcessEnd, 24, "GiveDetectProcessEnd");
	char* End = new char[4];
	strcpy_s(End, 4, "End");

	MemProcess* m_MemPro = new MemProcess;
	TCHAR* MyPath = new TCHAR[MAX_PATH_EX];
	GetModuleFileName(GetModuleHandle(NULL), MyPath, MAX_PATH_EX);
	clock_t start, end;
	clock_t m_BootStart, m_BootEnd;
	m_MemPro->m_RiskArray1.clear();
	m_MemPro->m_RiskArray2.clear();
	m_MemPro->m_UnKnownData1.clear();
	m_MemPro->m_UnKnownData2.clear();
	m_MemPro->pRiskArray = &m_MemPro->m_RiskArray1;
	m_MemPro->RiskArrayNum = 1;
	m_MemPro->pUnKnownData = &m_MemPro->m_UnKnownData1;
	m_MemPro->UnKnownDataNum = 1;

	//char* cBootTime = CStringToCharArray(pBootTime, CP_UTF8);
	bool IsWin10 = false;
	char* OSstr = GetOSVersion();
	if ((strstr(OSstr, "Windows 10") != 0) || (strstr(OSstr, "Windows Server 2016") != 0))
		IsWin10 = true;
	map<DWORD, process_info_Ex> StartProcessID;
	map<DWORD, process_info_Ex> NewProcessID;
	map<DWORD, process_info_Ex>::iterator st;
	map<DWORD, process_info_Ex>::iterator nt;
	//map<DWORD,process_info_Ex>::iterator ft;
	printf("load now process info start\n");
	m_MemPro->LoadNowProcessInfoDetect(&StartProcessID);
	printf("load now process info end\n");
	if (IsFirst)
	{
		for (st = StartProcessID.begin(); st != StartProcessID.end(); st++)
		{
			if (!m_MemPro->IsWindowsProcessNormal(&StartProcessID, st->first))
			{
				m_MemPro->ParserProcessRisk(&st->second, pApiName, MyPath, m_MemPro->pUnKnownData);
			}
		}
	}
	start = clock();
	m_BootStart = clock();
	m_BootEnd = clock();
	printf("for loop start\n");
	for (;;)
	{
		NewProcessID.clear();
		printf("load now process info start\n");
		m_MemPro->LoadNowProcessInfoDetect(&NewProcessID);
		printf("load now process info end\n");
		for (nt = NewProcessID.begin(); nt != NewProcessID.end(); nt++)
		{
			st = StartProcessID.find(nt->first);
			if (st == StartProcessID.end())
			{
				printf("parse new process ID start\n");
				m_MemPro->ParserProcessRisk(&nt->second, pApiName, MyPath, m_MemPro->pUnKnownData);
				printf("parse new process ID end\n");
			}
		}
		end = clock();

		if ((end - start) > 20000)
		{
			if (!m_MemPro->pRiskArray->empty())
			{
				if (m_MemPro->RiskArrayNum == 1)
				{
					m_MemPro->ChangeRiskArrayNum(1);
					vector<ProcessInfoData>* pRiskArray = m_MemPro->GetRiskArray1();
					if (!pRiskArray->empty())
					{
						SendProcessDataToServer(pRiskArray, tcpSocket);
					}
					//if (m_MemPro->UnKnownDataNum == 1)
					//{
					//	vector<UnKnownDataInfo>* pUnKnownData = m_MemPro->GetUnKnownData1();
					//	if (!pUnKnownData->empty())
					//	{
					//		m_MemPro->ChangeUnKnownDataNum(1);
					//		m_MemPro->SendUnKnownDataToServer(pUnKnownData);
					//		pUnKnownData->clear();
					//	}
					//}
					int ret = socketsend->SendMessageToServer(functionName_GiveDetectProcessEnd, End);
					pRiskArray->clear();
				}
				else if (m_MemPro->RiskArrayNum == 2)
				{
					m_MemPro->ChangeRiskArrayNum(2);
					vector<ProcessInfoData>* pRiskArray = m_MemPro->GetRiskArray2();
					if (!pRiskArray->empty())
					{
						SendProcessDataToServer(pRiskArray, tcpSocket);
					}
					//if (m_MemPro->UnKnownDataNum == 2)
					//{
					//	vector<UnKnownDataInfo>* pUnKnownData = m_MemPro->GetUnKnownData2();
					//	if (!pUnKnownData->empty())
					//	{
					//		m_MemPro->ChangeUnKnownDataNum(2);
					//		SendUnKnownDataToServer(pUnKnownData);
					//		pUnKnownData->clear();
					//	}
					//}
					int ret = socketsend->SendMessageToServer(functionName_GiveDetectProcessEnd, End);
					pRiskArray->clear();
				}
			}
			//if(!pUnKnownData->empty())
			//{
			//}
			start = clock();
		}
		StartProcessID.clear();
		StartProcessID = NewProcessID;
		if (!IsHavePID(pMainProcessid))
			break;
		if (IsWin10)
		{
			if ((m_BootEnd - m_BootStart) > 60000)
				Sleep(200);
			else
			{
				m_BootEnd = clock();
				Sleep(10);
			}
		}
		else
		{
			if ((m_BootEnd - m_BootStart) > 60000)
				Sleep(50);
			else
			{
				m_BootEnd = clock();
				Sleep(10);
			}
		}
	}
	NewProcessID.clear();
	StartProcessID.clear();
	m_MemPro->m_RiskArray1.clear();
	m_MemPro->m_RiskArray2.clear();
	delete[] MyPath;
	return 1;
}

void Task::SendProcessDataToServer(vector<ProcessInfoData>* pInfo, SOCKET* tcpSocket)
{
	char* functionName_GiveDetectProcessData = new char[24];
	strcpy_s(functionName_GiveDetectProcessData, 24, "GiveDetectProcessData");
	char* functionName_GiveDetectProcessOver = new char[24];
	strcpy_s(functionName_GiveDetectProcessOver, 24, "GiveDetectProcessOver");

	char* TempStr = new char[DATASTRINGMESSAGELEN];
	vector<ProcessInfoData>::iterator it;
	printf("for loop start\n");
	for (it = pInfo->begin(); it != pInfo->end(); it++)
	{
		wchar_t* wTempStr = new wchar_t[DATASTRINGMESSAGELEN];
		swprintf_s(wTempStr, DATASTRINGMESSAGELEN, L"%lu|Detect|%s|%s|%s|%s|%s|%lu|%s|%s|%d,%s|%d|%d|%d|%s|%d,%d"
			, (*it).ProcessID, (*it).ProcessCTime, (*it).ProcessTime, (*it).ProcessName, (*it).ProcessPath, (*it).ProcessHash,
			(*it).ParentID, (*it).ParentCTime, (*it).ParentPath, (*it).Injected, (*it).UnKnownHash, (*it).StartRun, (*it).HideAttribute, (*it).HideProcess, (*it).SignerSubjectName,
			(*it).InjectionPE, (*it).InjectionOther);
		char* cTempStr = CStringToCharArray(wTempStr, CP_UTF8);
		strcpy_s(TempStr, DATASTRINGMESSAGELEN, cTempStr);
		delete[] cTempStr;
		delete[] wTempStr;
		int ret = 1;

		printf("abnormal dll\n");
		if (!(*it).Abnormal_dll.empty())
		{
			strcat_s(TempStr, DATASTRINGMESSAGELEN, "|");
			set<string>::iterator dllit;
			for (dllit = (*it).Abnormal_dll.begin(); dllit != (*it).Abnormal_dll.end(); dllit++)
			{
				char* dllstr = new char[4096];
				sprintf_s(dllstr, 4096, "%s;", (*dllit).c_str());
				if ((strlen(dllstr) + strlen(TempStr)) >= DATASTRINGMESSAGELEN)
				{
					ret = socketsend->SendDataToServer(functionName_GiveDetectProcessData, TempStr, tcpSocket);
					memset(TempStr, '\0', DATASTRINGMESSAGELEN);
					if (ret <= 0)
					{
						delete[] dllstr;
						break;
					}
				}
				strcat_s(TempStr, DATASTRINGMESSAGELEN, dllstr);
				delete[] dllstr;
			}
			if (ret <= 0)
				break;
		}
		else
			strcat_s(TempStr, DATASTRINGMESSAGELEN, "|null");

		printf("inline hook\n");
		if (!(*it).InlineHookInfo.empty())
		{
			strcat_s(TempStr, DATASTRINGMESSAGELEN, "|");
			set<string>::iterator Inlineit;
			for (Inlineit = (*it).InlineHookInfo.begin(); Inlineit != (*it).InlineHookInfo.end(); Inlineit++)
			{
				char* Inlinestr = new char[4096];
				sprintf_s(Inlinestr, 4096, "%s;", (*Inlineit).c_str());
				if ((strlen(Inlinestr) + strlen(TempStr)) >= DATASTRINGMESSAGELEN)
				{
					ret = socketsend->SendDataToServer(functionName_GiveDetectProcessData, TempStr, tcpSocket);
					memset(TempStr, '\0', DATASTRINGMESSAGELEN);
					if (ret <= 0)
					{
						delete[] Inlinestr;
						break;
					}
				}
				strcat_s(TempStr, DATASTRINGMESSAGELEN, Inlinestr);
				delete[] Inlinestr;
			}
			if (ret <= 0)
				break;
		}
		else
			strcat_s(TempStr, DATASTRINGMESSAGELEN, "|null");

		//printf("net string\n");
		//if (!(*it).NetString.empty())
		//{
		//	strcat_s(TempStr, DATASTRINGMESSAGELEN, "|");
		//	set<string>::iterator netit;
		//	for (netit = (*it).NetString.begin(); netit != (*it).NetString.end(); netit++)
		//	{
		//		char* netstr = new char[4096];
		//		sprintf_s(netstr, 4096, "%s;", (*netit).c_str());
		//		if ((strlen(netstr) + strlen(TempStr)) >= DATASTRINGMESSAGELEN)
		//		{
		//			ret = socketsend->SendDataToServer(functionName_GiveDetectProcessData, TempStr);
		//			memset(TempStr, '\0', DATASTRINGMESSAGELEN);
		//			if (ret <= 0)
		//			{
		//				delete[] netstr;
		//				break;
		//			}
		//		}
		//		strcat_s(TempStr, DATASTRINGMESSAGELEN, netstr);
		//		delete[] netstr;
		//	}
		//	if (ret <= 0)
		//		break;
		//}
		//else
		//	strcat_s(TempStr, DATASTRINGMESSAGELEN, "|null");
		
		ret = socketsend->SendDataToServer(functionName_GiveDetectProcessOver, TempStr, tcpSocket);

		if (ret <= 0)
			break;
		else
			memset(TempStr, '\0', DATASTRINGMESSAGELEN);
	}
	delete[] TempStr;



	/*senddatamsgtoserver(mymac,myip,"givedetectprocessend","end");
	pinfo->clear();*/
}

char* Task::GetMyPCDrive()
{
	char* driveStr = new char[STRINGMESSAGELEN];
	driveStr[0] = '\0';
	for (int i = 2; i < 26; i++)
	{
		char* drive = new char[10];
		strcpy_s(drive, 10, GetDriveStr(i));
		UINT type = GetDriveTypeA(drive);
		if (!(type == DRIVE_FIXED || type == DRIVE_REMOVABLE))
		{
			delete[] drive;
			continue;
		}
		char* volname = new char[_MAX_FNAME];
		char* filesys = new char[_MAX_FNAME];
		DWORD VolumeSerialNumber, MaximumComponentLength, FileSystemFlags;
		if (GetVolumeInformationA(drive, volname, _MAX_FNAME, &VolumeSerialNumber, &MaximumComponentLength, &FileSystemFlags, filesys, _MAX_FNAME))
		{
			//drive.Remove(L'\\');
			for (int j = (int)strlen(drive) - 1; j >= 0; j--)
			{
				if (drive[j] == ':')
				{
					drive[j] = '\x0';
					break;
				}
			}
			if (type == DRIVE_FIXED)
			{
				strcat_s(driveStr, STRINGMESSAGELEN, drive);
				strcat_s(driveStr, STRINGMESSAGELEN, "-");
				strcat_s(driveStr, STRINGMESSAGELEN, filesys);
				strcat_s(driveStr, STRINGMESSAGELEN, ",HDD");
				strcat_s(driveStr, STRINGMESSAGELEN, "|");
			}
			else if (type == DRIVE_REMOVABLE)
			{
				strcat_s(driveStr, STRINGMESSAGELEN, drive);
				strcat_s(driveStr, STRINGMESSAGELEN, "-");
				strcat_s(driveStr, STRINGMESSAGELEN, filesys);
				strcat_s(driveStr, STRINGMESSAGELEN, ",USB");
				strcat_s(driveStr, STRINGMESSAGELEN, "|");
			}
		}
		delete[] filesys;
		delete[] volname;
		delete[] drive;
	}
	return driveStr;
}