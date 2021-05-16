#include <lea.h>
#include <unistd.h>

#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define MAX_BUF_SIZE	1024

int main()
{
	struct sockaddr_in server_addr;
	int comm_sock = 0;
	int server_addr_len = 0;
	int i, Nk, Nr;
	BYTE recvBuf[MAX_BUF_SIZE] = { 0, };
	BYTE sendBuf[MAX_BUF_SIZE] = { 0, };
	WORD RoundKey[144] = { 0, };
	BYTE K[16] =
	{ 0x0f, 0x1e, 0x2d, 0x3c, 0x4b, 0x5a, 0x69, 0x78, 0x87, 0x96, 0xa5, 0xb4, 0xc3, 0xd2, 0xe1, 0xf0 };
	BYTE P[16] = { 0 };
	Nk = 16;
	Nr = 24;

	/*통신 소켓 만들기*/
	comm_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (comm_sock == -1)
	{
		printf("error :\n");
		return 1;
	}

	/*server_addr 구조체 선언*/
	memset(&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("192.168.0.10");
	server_addr.sin_port = htons(9000);
	server_addr_len = sizeof(server_addr);

	/*서버 연결 시도*/
	if (connect(comm_sock, (struct sockaddr *)&server_addr, server_addr_len) == -1)
	{
		printf("connect error :\n");
		return 1;
	}

	KeySchedule_128(K, RoundKey);

	/*평문 입력*/
	WORD tmp = 0;
	printf("Write Plaintext : ");
	for (i = 0; i < 16; i++)
	{
		scanf("%x", &tmp);
		P[i] = tmp & 0xff;
	}
	printf("\n");

	/*평문 출력*/
	printf("Plaintext : ");
	for (i = 0; i < 16; i++)
	{
		printf("0x%02x ", P[i]);
	}
	printf("\n\n");

	/*암호화*/
	memset(sendBuf, 0x00, MAX_BUF_SIZE);
	Encrypt(Nr, RoundKey, P, sendBuf);
	printf("\n");

	/*write*/
	if (write(comm_sock, sendBuf, MAX_BUF_SIZE) <= 0)
	{
		printf("write error\n");
		return 1;
	}

	/*소켓 종료*/
	close(comm_sock);

	return 0;
}



