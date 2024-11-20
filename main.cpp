#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cerrno>
#include <sys/_types/_socklen_t.h>
#include <netdb.h>
#include <arpa/inet.h>

int main() {
	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo("127.0.0.1", "6969", &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	for(p = res; p != NULL; p = p->ai_next) {
		void *addr;
		const char *ipver;

		if (p->ai_family == AF_INET) {
			struct sockaddr_in *ipv4 = (struct sockaddr_in*) p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4";
		} else {
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*) p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}

		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf(" %s: %s\n", ipver, ipstr);
	}

	printf("Creating socket... \n");

	int serverSockfd = socket(PF_INET, SOCK_STREAM, 0);

	if (serverSockfd == -1) {
		fprintf(stderr, "Socket creation error\n");
		exit(1);
	}

	int boundError = bind(serverSockfd, res->ai_addr, res->ai_addrlen);

	if (boundError == -1) {
		fprintf(stderr, "File bind error\n");
		exit(1);
	}

	int listErr = listen(serverSockfd, 10);

	if (listErr == -1) {
		fprintf(stderr, "Server error listening");
		exit(1);
	}

	sockaddr clientSockAddr;
	socklen_t clientSize = sizeof (struct sockaddr_storage);
	int clientSock = accept(serverSockfd, &clientSockAddr, &clientSize);

	if (clientSock == -1) {
		fprintf(stderr, "Error connecting to client");
		exit(1);
	} else {
		printf("Client connected successfully\n");
	}

	 const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!";

	size_t size = send(clientSock, response, strlen(response), 0);

	if (size == -1) {
		fprintf(stderr, "Error sending data\n");
		exit(1);
	}

	freeaddrinfo(res);

	return 0;
}
