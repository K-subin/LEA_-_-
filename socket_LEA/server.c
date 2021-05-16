#include <lea.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define MAX_BUF_SIZE	1024

int main()
{
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;

	int connect_sock = 0; 
	int comm_sock = 0;
	int client_addr_len = 0;
	int ret = 0;
	int i, Nk, Nr;
	BYTE recvBuf[MAX_BUF_SIZE] = { 0, };
	WORD RoundKey[144] = { 0, };
	BYTE K[16] =
	{ 0x0f, 0x1e, 0x2d, 0x3c, 0x4b, 0x5a, 0x69, 0x78, 0x87, 0x96, 0xa5, 0xb4, 0xc3, 0xd2, 0xe1, 0xf0 };
	BYTE P[16] = { 0 };
	Nk = 16;
	Nr = 24;

	client_addr_len = sizeof(client_addr);

	connect_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect_sock == -1)
	{
		printf("SOCKET CREATE ERROR!!!\n");
		return 1;
	}

	memset(&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(9000);
	ret = bind(connect_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

	listen(connect_sock, 5);

	memset(&client_addr, 0x00, sizeof(client_addr));
	comm_sock = accept(connect_sock, (struct sockaddr *)&client_addr, &client_addr_len);

	printf("New Client : %s\n\n", inet_ntoa(client_addr.sin_addr));

	KeySchedule_128(K, RoundKey);

	memset(recvBuf, 0x00, MAX_BUF_SIZE);
	if (read(comm_sock, recvBuf, MAX_BUF_SIZE) <= 0)
	{
		printf("read error : \n");
		close(comm_sock);
	}

	printf("Ciphertext :");
	for (i = 0; i < 16; i++)
	{
		printf("0x%02x ", recvBuf[i]);
	}
	printf("\n\n");

	Decrypt(Nr, RoundKey, P, recvBuf);

	printf("Plaintext : ");
	for (i = 0; i < 16; i++)
	{
		printf("0x%02x ", P[i]);
	}
	printf("\n");

	close(comm_sock);
	close(connect_sock);

	return 0;


}