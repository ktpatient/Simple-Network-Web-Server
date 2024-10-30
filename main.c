#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    recv(client_socket, buffer, BUFFER_SIZE, 0);

    // Open the HTML file
    FILE *html_file = fopen("index.html", "r");
    if (html_file == NULL) {
        printf("Could not open HTML file\n");
        closesocket(client_socket);
        return;
    }

    // Read the HTML file content
    fseek(html_file, 0, SEEK_END);
    long file_size = ftell(html_file);
    fseek(html_file, 0, SEEK_SET);

    char *html_content = malloc(file_size + 1);
    fread(html_content, file_size, 1, html_file);
    fclose(html_file);
    html_content[file_size] = '\0';  // Null-terminate the HTML content

    // Create HTTP response
    char http_header[BUFFER_SIZE];
    sprintf(http_header,
	    "HTTP/1.1 200 OK\n"
	    "Content-Type: text/html\n"
	    "Content-Length: %ld\n"
	    "\n", file_size);

    // Send the HTTP header
    send(client_socket, http_header, strlen(http_header), 0);

    // Send the HTML content
    send(client_socket, html_content, file_size, 0);

    // Clean up
    free(html_content);
    closesocket(client_socket);
}

int main() {
    WSADATA wsaData;
    SOCKET server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock initialization failed. Error: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Set up the address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("192.168.0.69");
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("Bind failed. Error: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        printf("Listen failed. Error: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        // Accept incoming connection
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET) {
            printf("Accept failed. Error: %d\n", WSAGetLastError());
            closesocket(server_fd);
            WSACleanup();
            return 1;
        }

        // Handle client request
        handle_client(client_socket);
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}
