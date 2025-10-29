/**
 * =============================================================================
 * STUDENT ASSIGNMENT: CRYPTO-SERVER.C
 * =============================================================================
 * 
 * ASSIGNMENT OBJECTIVE:
 * Implement a TCP server that accepts client connections and processes
 * encrypted/plaintext messages. Your focus is on socket programming, connection
 * handling, and the server-side protocol implementation.
 * 
 * =============================================================================
 * WHAT YOU NEED TO IMPLEMENT:
 * =============================================================================
 * 
 * 1. SERVER SOCKET SETUP (start_server function):
 *    - Create a TCP socket using socket()
 *    - Set SO_REUSEADDR socket option (helpful during development)
 *    - Configure server address structure (struct sockaddr_in)
 *    - Bind the socket to the address using bind()
 *    - Start listening with listen()
 *    - Call your server loop function
 *    - Close socket on shutdown
 * 
 * 2. SERVER MAIN LOOP:
 *    - Create a function that handles multiple clients sequentially
 *    - Loop to:
 *      a) Accept incoming connections using accept()
 *      b) Get client's IP address for logging (inet_ntop)
 *      c) Call your client service function
 *      d) Close the client socket when done
 *      e) Return to accept next client (or exit if shutdown requested)
 * 
 * 3. CLIENT SERVICE LOOP:
 *    - Create a function that handles communication with ONE client
 *    - Allocate buffers for sending and receiving
 *    - Maintain session keys (client_key and server_key)
 *    - Loop to:
 *      a) Receive a PDU from the client using recv()
 *      b) Handle recv() return values (0 = closed, <0 = error)
 *      c) Parse the received PDU
 *      d) Check for special commands (exit, server shutdown)
 *      e) Build response PDU based on message type
 *      f) Send response using send()
 *      g) Return appropriate status code when client exits
 *    - Free buffers before returning
 * 
 * 4. RESPONSE BUILDING:
 *    - Consider creating a helper function to build response PDUs
 *    - Handle different message types:
 *      * MSG_KEY_EXCHANGE: Call gen_key_pair(), send client_key to client
 *      * MSG_DATA: Echo back with "echo " prefix
 *      * MSG_ENCRYPTED_DATA: Decrypt, add "echo " prefix, re-encrypt
 *      * MSG_CMD_CLIENT_STOP: No response needed (client will exit)
 *      * MSG_CMD_SERVER_STOP: No response needed (server will exit)
 *    - Set proper direction (DIR_RESPONSE)
 *    - Return total PDU size
 * 
 * =============================================================================
 * ONE APPROACH TO SOLVE THIS PROBLEM:
 * =============================================================================
 * 
 * FUNCTION STRUCTURE:
 * 
 * void start_server(const char* addr, int port) {
 *     // 1. Create TCP socket
 *     // 2. Set SO_REUSEADDR option (for development)
 *     // 3. Configure server address (sockaddr_in)
 *     //    - Handle "0.0.0.0" specially (use INADDR_ANY)
 *     // 4. Bind socket to address
 *     // 5. Start listening (use BACKLOG constant)
 *     // 6. Call your server loop function
 *     // 7. Close socket
 * }
 * 
 * int server_loop(int server_socket, const char* addr, int port) {
 *     // 1. Print "Server listening..." message
 *     // 2. Infinite loop:
 *     //    a) Accept connection (creates new client socket)
 *     //    b) Get client IP using inet_ntop()
 *     //    c) Print "Client connected..." message
 *     //    d) Call service_client_loop(client_socket)
 *     //    e) Check return code:
 *     //       - RC_CLIENT_EXITED: close socket, accept next client
 *     //       - RC_CLIENT_REQ_SERVER_EXIT: close sockets, return
 *     //       - Error: close socket, continue
 *     //    f) Close client socket
 *     // 3. Return when server shutdown requested
 * }
 * 
 * int service_client_loop(int client_socket) {
 *     // 1. Allocate send/receive buffers
 *     // 2. Initialize keys to NULL_CRYPTO_KEY
 *     // 3. Loop:
 *     //    a) Receive PDU from client
 *     //    b) Check recv() return:
 *     //       - 0: client closed, return RC_CLIENT_EXITED
 *     //       - <0: error, return RC_CLIENT_EXITED
 *     //    c) Cast buffer to crypto_msg_t*
 *     //    d) Check for MSG_CMD_SERVER_STOP -> return RC_CLIENT_REQ_SERVER_EXIT
 *     //    e) Build response PDU (use helper function)
 *     //    f) Send response
 *     //    g) Loop back
 *     // 4. Free buffers before returning
 * }
 * 
 * int build_response(crypto_msg_t *request, crypto_msg_t *response, 
 *                    crypto_key_t *client_key, crypto_key_t *server_key) {
 *     // 1. Set response->header.direction = DIR_RESPONSE
 *     // 2. Set response->header.msg_type = request->header.msg_type
 *     // 3. Switch on request type:
 *     //    MSG_KEY_EXCHANGE:
 *     //      - Call gen_key_pair(server_key, client_key)
 *     //      - Copy client_key to response->payload
 *     //      - Set payload_len = sizeof(crypto_key_t)
 *     //    MSG_DATA:
 *     //      - Format: "echo <original message>"
 *     //      - Copy to response->payload
 *     //      - Set payload_len
 *     //    MSG_ENCRYPTED_DATA:
 *     //      - Decrypt request->payload using decrypt_string()
 *     //      - Format: "echo <decrypted message>"
 *     //      - Encrypt result using encrypt_string()
 *     //      - Copy encrypted data to response->payload
 *     //      - Set payload_len
 *     //    MSG_CMD_*:
 *     //      - Set payload_len = 0
 *     // 4. Return sizeof(crypto_pdu_t) + payload_len
 * }
 * 
 * =============================================================================
 * IMPORTANT PROTOCOL DETAILS:
 * =============================================================================
 * 
 * SERVER RESPONSIBILITIES:
 * 1. Generate encryption keys when client requests (MSG_KEY_EXCHANGE)
 * 2. Send the CLIENT'S key to the client (not the server's key!)
 * 3. Keep both keys: server_key (for decrypting client messages)
 *                    client_key (to send to client)
 * 4. Echo messages back with "echo " prefix
 * 5. Handle encrypted data: decrypt -> process -> encrypt -> send
 * 
 * KEY GENERATION:
 *   crypto_key_t server_key, client_key;
 *   gen_key_pair(&server_key, &client_key);
 *   // Send client_key to the client in MSG_KEY_EXCHANGE response
 *   memcpy(response->payload, &client_key, sizeof(crypto_key_t));
 * 
 * DECRYPTING CLIENT DATA:
 *   // Client encrypted with their key, we decrypt with server_key
 *   uint8_t decrypted[MAX_SIZE];
 *   decrypt_string(server_key, decrypted, request->payload, request->header.payload_len);
 *   decrypted[request->header.payload_len] = '\0'; // Null-terminate
 * 
 * ENCRYPTING RESPONSE:
 *   // We encrypt with server_key for client to decrypt with their key
 *   uint8_t encrypted[MAX_SIZE];
 *   int encrypted_len = encrypt_string(server_key, encrypted, plaintext, plaintext_len);
 *   memcpy(response->payload, encrypted, encrypted_len);
 *   response->header.payload_len = encrypted_len;
 * 
 * RETURN CODES:
 *   RC_CLIENT_EXITED          - Client disconnected normally
 *   RC_CLIENT_REQ_SERVER_EXIT - Client requested server shutdown
 *   RC_OK                     - Success
 *   Negative values           - Errors
 * 
 * =============================================================================
 * SOCKET PROGRAMMING REMINDERS:
 * =============================================================================
 * 
 * CREATING AND BINDING:
 *   int sockfd = socket(AF_INET, SOCK_STREAM, 0);
 *   
 *   struct sockaddr_in addr;
 *   memset(&addr, 0, sizeof(addr));
 *   addr.sin_family = AF_INET;
 *   addr.sin_port = htons(port);
 *   addr.sin_addr.s_addr = INADDR_ANY;  // or use inet_pton()
 *   
 *   bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
 *   listen(sockfd, BACKLOG);
 * 
 * ACCEPTING CONNECTIONS:
 *   struct sockaddr_in client_addr;
 *   socklen_t addr_len = sizeof(client_addr);
 *   int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
 * 
 * GETTING CLIENT IP:
 *   char client_ip[INET_ADDRSTRLEN];
 *   inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
 * 
 * =============================================================================
 * DEBUGGING TIPS:
 * =============================================================================
 * 
 * 1. Use print_msg_info() to display received and sent PDUs
 * 2. Print client IP when connections are accepted
 * 3. Check all socket operation return values
 * 4. Test with plaintext (MSG_DATA) before trying encryption
 * 5. Verify keys are generated correctly (print key values)
 * 6. Use telnet or netcat to test basic connectivity first
 * 7. Handle partial recv() - though for this assignment, assume full PDU arrives
 * 
 * =============================================================================
 * TESTING RECOMMENDATIONS:
 * =============================================================================
 * 
 * 1. Start simple: Accept connection and echo plain text
 * 2. Test key exchange: Client sends '#', server generates and returns key
 * 3. Test encryption: Client sends '!message', server decrypts, echoes, encrypts
 * 4. Test multiple clients: Connect, disconnect, connect again
 * 5. Test shutdown: Client sends '=', server exits gracefully
 * 6. Test error cases: Client disconnects unexpectedly
 * 
 * Good luck! Server programming requires careful state management!
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdint.h>
#include "crypto-server.h"
#include "crypto-lib.h"
#include "protocol.h"

// Global buffers
char send_buffer[BUFFER_SIZE];
char recv_buffer[BUFFER_SIZE];

//helpers

//recv_pdu() - this assists in ensuring all bytes are fully recieved from recv() before continuing.

ssize_t recv_pdu(int sockfd, uint8_t *buff){
    size_t bytes_received = 0;
    ssize_t result;
    
    //get the header
    while (bytes_received < sizeof(crypto_pdu_t)) {
        result = recv(sockfd, buff + bytes_received, sizeof(crypto_pdu_t) - bytes_received, 0);
        
        //check the recv status
        if (result <= 0) {
            return result; 
        }
        bytes_received += result;
    }

    crypto_msg_t *pdu = (crypto_msg_t*) buff;

    //check payload length
    if (pdu->header.payload_len > MAX_MSG_DATA_SIZE) {
        perror("Message too large");
        return -1;
    }

    //if cmd server stop, no need to continue and process payload
    if (pdu->header.msg_type == MSG_CMD_SERVER_STOP){
        return MSG_CMD_SERVER_STOP;
    }
    
    
    // get the message data fully

    bytes_received = 0;
    while (bytes_received < pdu->header.payload_len) {
        result = recv(sockfd, pdu->payload + bytes_received, pdu->header.payload_len - bytes_received, 0);
        
        //check recv status
        if (result <= 0) {
            return result;
        }
        bytes_received += result;
    }
    
    return sizeof(crypto_pdu_t) + pdu->header.payload_len;
}

// send_all() - makes sure all bytes are sent

ssize_t send_all(int sockfd, const char* buffer, size_t length) {
    size_t bytes_sent = 0;
    ssize_t result;
    
    while (bytes_sent < length) {
        result = send(sockfd, buffer + bytes_sent, length - bytes_sent, 0);
        
        //check successful send
        if (result < 0) {
            return -1;
        }
        bytes_sent += result;
    }
    
    return bytes_sent;
}

// send_pdu() - send a message as a PDU
ssize_t send_pdu(int sockfd, crypto_msg_t *pdu) {
    size_t pdu_len = sizeof(crypto_pdu_t) + pdu->header.payload_len;
    memcpy(send_buffer, pdu, pdu_len);
    
    return send_all(sockfd, send_buffer, pdu_len);
}



/*
 * int build_response(crypto_msg_t *request, crypto_msg_t *response, 
 *                    crypto_key_t *client_key, crypto_key_t *server_key) {
 *     // 1. Set response->header.direction = DIR_RESPONSE
 *     // 2. Set response->header.msg_type = request->header.msg_type
 *     // 3. Switch on request type:
 *     //    MSG_KEY_EXCHANGE:
 *     //      - Call gen_key_pair(server_key, client_key)
 *     //      - Copy client_key to response->payload
 *     //      - Set payload_len = sizeof(crypto_key_t)
 *     //    MSG_DATA:
 *     //      - Format: "echo <original message>"
 *     //      - Copy to response->payload
 *     //      - Set payload_len
 *     //    MSG_ENCRYPTED_DATA:
 *     //      - Decrypt request->payload using decrypt_string()
 *     //      - Format: "echo <decrypted message>"
 *     //      - Encrypt result using encrypt_string()
 *     //      - Copy encrypted data to response->payload
 *     //      - Set payload_len
 *     //    MSG_CMD_*:
 *     //      - Set payload_len = 0
 *     // 4. Return sizeof(crypto_pdu_t) + payload_len
*/

int build_response(crypto_msg_t *request, crypto_msg_t *response, crypto_key_t *client_key, crypto_key_t *server_key){

    // 1. Set response->header.direction = DIR_RESPONSE
    // 2. Set response->header.msg_type = request->header.msg_type

    response->header.direction = DIR_RESPONSE;
    response->header.msg_type = request->header.msg_type;

    /*
    Switch on request type:
 *         MSG_KEY_EXCHANGE:
 *           - Call gen_key_pair(server_key, client_key)
 *           - Copy client_key to response->payload
 *           - Set payload_len = sizeof(crypto_key_t)
 *         MSG_DATA:
 *           - Format: "echo <original message>"
 *           - Copy to response->payload
 *           - Set payload_len
 *         MSG_ENCRYPTED_DATA:
 *           - Decrypt request->payload using decrypt_string()
 *           - Format: "echo <decrypted message>"
 *           - Encrypt result using encrypt_string()
 *           - Copy encrypted data to response->payload
 *           - Set payload_len
 *         MSG_CMD_*:
 *           - Set payload_len = 0
 */

    switch (request->header.msg_type)
    {
    
    case MSG_DATA:

        char msg_buff[BUFFER_SIZE];
        
        //format with echo
        int length_echo = snprintf(msg_buff, sizeof(msg_buff), "echo %.*s", request->header.payload_len, request->payload);

        //check if write was successful:
        if (length_echo < 0){
            return length_echo;
        }

        //copy to resposnse->payload
        memcpy(response->payload, msg_buff, length_echo);
        response->header.payload_len = length_echo;
        break;

    case MSG_KEY_EXCHANGE:

        int key_return = gen_key_pair(server_key, client_key);

        //check for successful key gen
        if (key_return < 0){
            return RC_CRYPTO_ERR;
        }

        //copy to payload
        memcpy(response->payload, client_key, sizeof(crypto_key_t));
        response->header.payload_len = sizeof(crypto_key_t);
        break;
        

    case MSG_ENCRYPTED_DATA:

        char clear_string[BUFFER_SIZE];
        //decrypt req payload
        int decrypted_return = decrypt_string(*server_key, (uint8_t*)clear_string, request->payload, request->header.payload_len);
        
        //check for successful decryption
        if (decrypted_return < 0){
            return RC_DECRYPTION_ERR;
        }

        char send_encrypted[BUFFER_SIZE];
        // format with echo
        int echo_length = snprintf(send_encrypted, sizeof(send_encrypted), "echo %.*s", decrypted_return, clear_string);

        //re-encrypt now with echo added
        int encrypted_return = encrypt_string(*server_key, response->payload, (uint8_t*)send_encrypted, (size_t) echo_length);

        //check re-encryption
        if (encrypted_return < 0){
            return RC_ENCRYPTION_ERR;
        }

        //copy to payload
        response->header.payload_len = (uint16_t)encrypted_return;
        break;

    
        //all other commands the payload length is 0
    default:
        response->header.payload_len = 0;
        break;
    }
    
    return sizeof(crypto_pdu_t) + response->header.payload_len;
}



/*
 * int service_client_loop(int client_socket);
 *   - Handles communication with one client
 *   - Receives requests, builds responses, sends replies
 *   - Returns RC_CLIENT_EXITED or RC_CLIENT_REQ_SERVER_EXIT
*/

int service_client_loop(int client_socket){

    ssize_t pdu_len;

    char recv_buff[BUFFER_SIZE];
    char send_buff[BUFFER_SIZE];

    crypto_key_t server_key = NULL_CRYPTO_KEY;
    crypto_key_t client_key = NULL_CRYPTO_KEY;


    while(1){

        //call recv_pdu() to ensure all bytes are recieved
        pdu_len = recv_pdu(client_socket, (uint8_t*)recv_buff);
        
        //check the recv() ensure successfull, also check for stop commands
        if (pdu_len < 0) {
            printf("Error receiving message from client.\n");
            return RC_CLIENT_EXITED; // Close this client, wait for next one
        } else if (pdu_len == 0) {
            printf("Client disconnected.\n");
            return RC_CLIENT_EXITED; // Close this client, wait for next one
        } else if (pdu_len == MSG_CMD_SERVER_STOP){
            return RC_CLIENT_REQ_SERVER_EXIT;
        } else if (pdu_len == MSG_CMD_CLIENT_STOP){
            return RC_CLIENT_EXITED;
        }

        crypto_msg_t *req_pdu = (crypto_msg_t*)recv_buff;
        crypto_msg_t *response_pdu = (crypto_msg_t*)send_buff;


        //build the response packet, should return packet len
        int packet_len = build_response(req_pdu, response_pdu, &client_key, &server_key);

        //check to ensure packet was build successfully;
        if (packet_len < 0){
            perror("Error building packet");
            return RC_CLIENT_EXITED;
        }

        int ret = send_pdu(client_socket, response_pdu);

        //print out the server responses
        printf("Echoed: %.*s\n", response_pdu->header.payload_len, (char*)response_pdu->payload);

        //check for exit commands
        if (packet_len == RC_CLIENT_EXITED || packet_len == RC_CLIENT_REQ_SERVER_EXIT){
            return packet_len;
        }
    }

    return RC_CLIENT_EXITED;
    

}

/*
 * 2. SERVER MAIN LOOP:
 *    - Create a function that handles multiple clients sequentially
 *    - Loop to:
 *      a) Accept incoming connections using accept()
 *      b) Get client's IP address for logging (inet_ntop)
 *      c) Call your client service function
 *      d) Close the client socket when done
 *      e) Return to accept next client (or exit if shutdown requested)
*/

int server_loop(int sockfd, const char* addr, int port){

    int client_sock;
    char client_ip[INET_ADDRSTRLEN];
    int server_exit_cmd = 0;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);


    while(!server_exit_cmd){
        printf("Waiting for Client to Connect\n");

        //accept the client connection
        client_sock = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            perror("Error accepting connection");
            continue; // Try to accept next connection
        }
        

        //client info for logging
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("Client connected from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        printf("Server ready to process messages from this client...\n");

        int ret = service_client_loop(client_sock);

        close(client_sock);



        //handles client exit and server exit

        if (ret == RC_CLIENT_REQ_SERVER_EXIT){
            server_exit_cmd = 1;
            printf("Client Requested Server Shutdown\n");
        }

        printf("Client connection closed.\n");
        
        if (!server_exit_cmd) {
            printf("Ready for next client connection.\n\n");
        }
        else{
            printf("Exiting Server...\n");
        }
        
    }

    return RC_OK;

}

/* =============================================================================
 * STUDENT TODO: IMPLEMENT THIS FUNCTION
 * =============================================================================
 * This is the main server initialization function. You need to:
 * 1. Create a TCP socket
 * 2. Set socket options (SO_REUSEADDR)
 * 3. Bind to the specified address and port
 * 4. Start listening for connections
 * 5. Call your server loop function
 * 6. Clean up when done
 * 
 * Parameters:
 *   addr - Server bind address (e.g., "0.0.0.0" for all interfaces)
 *   port - Server port number (e.g., 1234)
 * 
 * #define BUFFER_SIZE 1024
    #define DEFAULT_PORT 1234
    #define DEFAULT_CLIENT_ADDR "127.0.0.1"
    #define DEFAULT_SERVER_ADDR "0.0.0.0"
    #define BACKLOG 5
 * 
 * NOTE: If addr is "0.0.0.0", use INADDR_ANY instead of inet_pton()
 */

void start_server(const char* addr, int port) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    int reuse = 1;


    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Error setting socket options");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    //configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    //need to figure out the ip
    if (strcmp(addr, "0.0.0.0") == 0) {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, addr, &server_addr.sin_addr) <= 0) {
            fprintf(stderr, "Error: Invalid address %s\n", addr);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }

    //Bind to the specified address and port
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    //Start listening for connections
    if (listen(sockfd, BACKLOG) < 0) {
        perror("Error listening on socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    //print server info
    printf("Server listening on %s:%d\n", addr, port);
    printf("Server will handle multiple clients sequentially.\n");

    int ret = server_loop(sockfd, addr, port);
    
    close(sockfd);

}