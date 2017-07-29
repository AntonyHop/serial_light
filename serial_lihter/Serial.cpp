#include <WinSock2.h>
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <string.h>
#include <sstream>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;
HANDLE Port;
DCB dcb;
DWORD write_size;

void SerialInit() {
	Port = CreateFile(L"\\\\.\\COM4", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (Port == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			cout << "serial port does not exist.\n";
		}
		cout << "some other error occurred.\n";
	}

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(Port, &dcbSerialParams))
	{
		cout << "getting state error\n";
	}
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(Port, &dcbSerialParams))
	{
		cout << "error setting serial port state\n";
	}
}

void SerialDistruct() {
	CloseHandle(Port);
}

void SerialPrint(char data[]) {
	DWORD dwSize = sizeof(data);   // размер этой строки
	DWORD dwBytesWritten;    // тут будет количество собственно переданных байт

	Sleep(200);
	WriteFile(Port, data, dwSize, &dwBytesWritten, NULL);
}
//__________SOCKETS___________________________________

sockaddr_in addr;
SOCKET sender;

void WsaInit() {
	WSADATA wsdata;
	if (WSAStartup(MAKEWORD(2, 2), &wsdata) != 0) {
		cout << "init error " << WSAGetLastError() << endl;
	}

	ZeroMemory(&addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr("31.170.167.196");
	addr.sin_port = htons(80);
}

string CleanResponse(char resp[],int resp_length) {
	string out;
	for (int i = 0; i < resp_length; i++) {
		out += resp[i];
	}
	return out;
}

string SendHTTPReq(string headers) {

	if (INVALID_SOCKET == (sender = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
		cout << "Socket error " << WSAGetLastError() << endl;
	}

	if (SOCKET_ERROR == connect(sender, (SOCKADDR *)&addr, sizeof(addr))) {
		cout << "Connect error " << WSAGetLastError() << endl;
	}
	if (SOCKET_ERROR == (send(sender, headers.c_str(), headers.size(), 0))) {
		cout << " Send err" << WSAGetLastError() << endl;
	};
	char buffer[1024];
	int length = recv(sender, buffer, strlen(buffer),MSG_PUSH_IMMEDIATE);
	//recv(sender, buffer, strlen(buffer), 0);
	return CleanResponse(buffer, length);

	closesocket(sender);

}


int main() {
	setlocale(LC_ALL, "ru");
	SerialInit();
	WsaInit();
	stringstream ss;
	ss << "GET /main/serial HTTP/1.1\r\n"
		<< "Host: relaxmusic.esy.es\r\n"
		<< "\r\n\r\n";

	while (true) {
		string resp = SendHTTPReq(ss.str());
		if (resp.find("off") != string::npos) { 
			cout << "off найден" << endl;
			SerialPrint("0");
		} 
		else if (resp.find("on") != string::npos) {
			cout << "on найден" << endl;
			SerialPrint("1");
		}
		
		Sleep(500);
	}
	SerialDistruct();
	return 0;
}