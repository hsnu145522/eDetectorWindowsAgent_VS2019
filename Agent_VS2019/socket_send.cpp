#include "socket_send.h"

SocketSend::SocketSend(Info* infoInstance) {
	info = infoInstance;
}

int SocketSend::SendDataToServer(char* Work, char* Mgs, SOCKET* tcpSocket) {
	StrDataPacket GetServerMessage;
	strcpy_s(GetServerMessage.MAC, sizeof(GetServerMessage.MAC), info->MAC);
	strcpy_s(GetServerMessage.IP, sizeof(GetServerMessage.IP), info->IP);
	strcpy_s(GetServerMessage.UUID, sizeof(GetServerMessage.UUID), info->UUID);

	char WorkNew[24];
	strcpy_s(WorkNew, sizeof(WorkNew), Work);
	WorkNew[strlen(Work)] = '\0';

	strcpy_s(GetServerMessage.DoWorking, sizeof(GetServerMessage.DoWorking), WorkNew);
	memcpy(GetServerMessage.csMsg, Mgs, sizeof(GetServerMessage.csMsg));

	char* buff = (char*)&GetServerMessage;

	SetKeys(BIT128, AESKey);
	EncryptBuffer((BYTE*)buff, STRDATAPACKETSIZE);

	int ret = sendTCP(buff, STRDATAPACKETSIZE, tcpSocket);
	printf("Send Data Packet: %s\n", Work);

	return receiveTCP(tcpSocket);
	

	//delete[] Work;
	//delete[] Mgs;

	//return ret;
}

int SocketSend::SendMessageToServer(char* Work, char* Mgs) {
	StrPacket GetServerMessage;
	strcpy_s(GetServerMessage.MAC, sizeof(GetServerMessage.MAC), info->MAC);
	strcpy_s(GetServerMessage.IP, sizeof(GetServerMessage.IP), info->IP);
	strcpy_s(GetServerMessage.UUID, sizeof(GetServerMessage.UUID), info->UUID);

	char WorkNew[24];
	strcpy_s(WorkNew, sizeof(WorkNew), Work);
	WorkNew[strlen(Work)] = '\0';
	

	strcpy_s(GetServerMessage.DoWorking, sizeof(GetServerMessage.DoWorking), WorkNew);
	strcpy_s(GetServerMessage.csMsg, sizeof(GetServerMessage.csMsg), Mgs);

	char* buff = (char*)&GetServerMessage;

	SetKeys(BIT128, AESKey);
	EncryptBuffer((BYTE*)buff, STRPACKETSIZE);

	int ret = sendTCP(buff, STRPACKETSIZE, info->tcpSocket);


	printf("Send Message Packet: %s\n", Work);

	delete[] Work;
	delete[] Mgs;
	return ret;
}

bool SocketSend::sendTCP(char* data, long len, SOCKET* tcpSocket) {

	int ret = send(*tcpSocket, data, len, 0);
	if (!ret) {

		std::cerr << "Error sending data: " << WSAGetLastError() << std::endl;
	}
	else {
		// std::cout << "Data sent successfully." << std::endl;
	}

	return ret;
}

int SocketSend::receiveTCP(SOCKET* tcpSocket) {
	
	while (true) {
		char buff[STRPACKETSIZE];
		int ret = recv(*tcpSocket, buff, sizeof(buff), 0);

		if (ret == SOCKET_ERROR) {
			std::cerr << "Error receiving ACK: " << WSAGetLastError() << std::endl;
			return 0;
		}

		SetKeys(BIT128, AESKey);
		DecryptBuffer((BYTE*)buff, STRPACKETSIZE);

		StrPacket* udata;
		udata = (StrPacket*)buff;

		printf("Receive: %s\n", udata->DoWorking);
		if (!strcmp(udata->DoWorking, "DataRight")) {
			return 1;
		}
		else {
			return 0;
		}
	}
	

	return 1;


}