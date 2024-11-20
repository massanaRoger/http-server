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
#include <cstddef>
#include <pthread.h>

struct ServerData {
	struct addrinfo *data;
	int socket;
};
ServerData start_server() {
	ServerData res;
	struct addrinfo hints, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo("127.0.0.1", "6969", &hints, &res.data)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	for(p = res.data; p != NULL; p = p->ai_next) {
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

	res.socket = socket(PF_INET, SOCK_STREAM, 0);

	if (res.socket == -1) {
		fprintf(stderr, "Socket creation error\n");
		exit(1);
	}

	int boundError = bind(res.socket, res.data->ai_addr, res.data->ai_addrlen);

	if (boundError == -1) {
		fprintf(stderr, "File bind error\n");
		exit(1);
	}

	int listErr = listen(res.socket, 10);

	if (listErr == -1) {
		fprintf(stderr, "Server error listening");
		exit(1);
	}

	return res;
}

void* handle_request(void* ptr) {
	int clientSock = *(int*)ptr;
	free(ptr);

	printf("ClientSock: %d\n", clientSock);
	fflush(stdout);

	const char *response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 13\r\n"
		"\r\n"
		"Hello, World!";

	int size = send(clientSock, response, strlen(response), 0);

	if (size == -1) {
		fprintf(stderr, "Error sending data\n");
		shutdown(clientSock, 0);
		exit(1);
	}

	shutdown(clientSock, 0);
	return NULL;
}

void start_listening(int serverSockfd) {
	sockaddr clientSockAddr;
	socklen_t clientSize = sizeof (struct sockaddr_storage);

	while (true) {
		int clientSock = accept(serverSockfd, &clientSockAddr, &clientSize);
		if (clientSock < 0) {
			fprintf(stderr, "Error in accepty \n");
			continue;
		}
		int* clientSockPtr = (int*) malloc(sizeof(int));

		if (clientSockPtr == NULL) {
			fprintf(stderr, "Could not assign memory \n");
			exit(1);
		}
		*clientSockPtr = clientSock;

		pthread_t thread;
		if (pthread_create(&thread, NULL, handle_request, (void*)clientSockPtr) != 0) {
			fprintf(stderr, "Error creating thread\n");
			free(clientSockPtr);
			shutdown(clientSock, 0);
			continue;
		}

		if (pthread_detach(thread) != 0) {
			fprintf(stderr, "Error detaching thread \n");
			free(clientSockPtr);
			shutdown(clientSock, 0);
			exit(1);
		}
	}
}

int main() {
	ServerData serverData = start_server();

	start_listening(serverData.socket);

	freeaddrinfo(serverData.data);

	return 0;
}
