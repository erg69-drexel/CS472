/**
 * =============================================================================
 * STUDENT ASSIGNMENT: CRYPTO-CLIENT.C
 * =============================================================================
 * 
 * ASSIGNMENT OBJECTIVE:
 * Implement a TCP client that communicates with a server using an encrypted
 * protocol. Your focus is on socket programming and network communication.
 * The cryptographic functions are provided for you in crypto-lib.
 * 
 * =============================================================================
 * WHAT YOU NEED TO IMPLEMENT:
 * =============================================================================
 * 
 * 1. SOCKET CONNECTION (start_client function):
 *    - Create a TCP socket using socket()
 *    - Configure the server address structure (struct sockaddr_in)
 *    - Connect to the server using connect()
 *    - Handle connection errors appropriately
 *    - Call your communication loop function
 *    - Close the socket when done
 * 
 * 2. CLIENT COMMUNICATION LOOP:
 *    - Create a function that handles the request/response cycle
 *    - Allocate buffers for sending and receiving data
 *    - Maintain a session key (crypto_key_t) for encryption
 *    - Loop to:
 *      a) Get user command using get_command() (provided below)
 *      b) Build a PDU (Protocol Data Unit) from the command
 *      c) Send the PDU to the server using send()
 *      d) Receive the server's response using recv()
 *      e) Process the response (extract key, decrypt data, etc.)
 *      f) Handle exit commands and connection closures
 *    - Free allocated buffers before returning
 * 
 * 3. PDU CONSTRUCTION:
 *    - Consider creating a helper function to build PDUs
 *    - Fill in the PDU header (msg_type, direction, payload_len)
 *    - For MSG_DATA: copy plaintext to payload
 *    - For MSG_ENCRYPTED_DATA: use encrypt_string() to encrypt before copying
 *    - For MSG_KEY_EXCHANGE: no payload needed
 *    - For command messages: no payload needed
 *    - Return the total PDU size (header + payload)
 * 
 * =============================================================================
 * ONE APPROACH TO SOLVE THIS PROBLEM:
 * =============================================================================
 * 
 * FUNCTION STRUCTURE:
 * 
 * void start_client(const char* addr, int port) {
 *     // 1. Create TCP socket
 *     // 2. Configure server address (sockaddr_in)
 *     // 3. Connect to server
 *     // 4. Print connection confirmation
 *     // 5. Call your communication loop function
 *     // 6. Close socket
 *     // 7. Print disconnection message
 * }
 * 
 * int client_loop(int socket_fd) {
 *     // 1. Allocate buffers (send, receive, input)
 *     // 2. Initialize session_key to NULL_CRYPTO_KEY
 *     // 3. Loop:
 *     //    a) Call get_command() to get user input
 *     //    b) Build PDU from command (use helper function)
 *     //    c) Send PDU using send()
 *     //    d) If exit command, break after sending
 *     //    e) Receive response using recv()
 *     //    f) Handle recv() return values (0 = closed, <0 = error)
 *     //    g) Process response:
 *     //       - If MSG_KEY_EXCHANGE: extract key from payload
 *     //       - If MSG_ENCRYPTED_DATA: decrypt using decrypt_string()
 *     //       - Print results
 *     //    h) Loop back
 *     // 4. Free buffers
 *     // 5. Return success/error code
 * }
 * 
 * int build_packet(const msg_cmd_t *cmd, crypto_msg_t *pdu, crypto_key_t key) {
 *     // 1. Set pdu->header.msg_type = cmd->cmd_id
 *     // 2. Set pdu->header.direction = DIR_REQUEST
 *     // 3. Based on cmd->cmd_id:
 *     //    - MSG_DATA: copy cmd->cmd_line to payload, set length
 *     //    - MSG_ENCRYPTED_DATA: encrypt cmd->cmd_line, set length
 *     //    - MSG_KEY_EXCHANGE: set length to 0
 *     //    - Command messages: set length to 0
 *     // 4. Return sizeof(crypto_pdu_t) + payload_len
 * }
 * 
 * =============================================================================
 * IMPORTANT PROTOCOL DETAILS:
 * =============================================================================
 * 
 * PDU STRUCTURE:
 *   typedef struct crypto_pdu {
 *       uint8_t  msg_type;      // MSG_DATA, MSG_ENCRYPTED_DATA, etc.
 *       uint8_t  direction;     // DIR_REQUEST or DIR_RESPONSE
 *       uint16_t payload_len;   // Length of payload in bytes
 *   } crypto_pdu_t;
 * 
 *   typedef struct crypto_msg {
 *       crypto_pdu_t header;
 *       uint8_t      payload[]; // Flexible array
 *   } crypto_msg_t;
 * 
 * MESSAGE TYPES (from protocol.h):
 *   MSG_KEY_EXCHANGE     - Request/send encryption key
 *   MSG_DATA             - Plain text message
 *   MSG_ENCRYPTED_DATA   - Encrypted message (requires session key)
 *   MSG_CMD_CLIENT_STOP  - Client exit command
 *   MSG_CMD_SERVER_STOP  - Server shutdown command
 * 
 * TYPICAL MESSAGE FLOW:
 *   1. Client sends MSG_KEY_EXCHANGE request
 *   2. Server responds with MSG_KEY_EXCHANGE + key in payload
 *   3. Client extracts key: memcpy(&session_key, response->payload, sizeof(crypto_key_t))
 *   4. Client can now send MSG_ENCRYPTED_DATA
 *   5. Server responds with MSG_ENCRYPTED_DATA
 *   6. Client decrypts using decrypt_string()
 * 
 * =============================================================================
 * CRYPTO LIBRARY FUNCTIONS YOU'LL USE:
 * =============================================================================
 * 
 * int encrypt_string(crypto_key_t key, uint8_t *out, uint8_t *in, size_t len)
 *   - Encrypts a string before sending
 *   - Returns number of encrypted bytes or negative on error
 * 
 * int decrypt_string(crypto_key_t key, uint8_t *out, uint8_t *in, size_t len)
 *   - Decrypts received data
 *   - Returns number of decrypted chars or negative on error
 *   - NOTE: Output is NOT null-terminated, you must add '\0'
 * 
 * void print_msg_info(crypto_msg_t *msg, crypto_key_t key, int mode)
 *   - Prints PDU details for debugging
 *   - Use CLIENT_MODE for the mode parameter
 *   - VERY helpful for debugging your protocol!
 * 
 * =============================================================================
 * DEBUGGING TIPS:
 * =============================================================================
 * 
 * 1. Use print_msg_info() before sending and after receiving
 * 2. Check return values from ALL socket operations
 * 3. Verify payload_len matches actual data length
 * 4. Remember: recv() may return less bytes than expected
 * 5. Encrypted data requires a valid session key (check for NULL_CRYPTO_KEY)
 * 6. Use printf() liberally to trace program flow
 * 
 * =============================================================================
 * TESTING RECOMMENDATIONS:
 * =============================================================================
 * 
 * 1. Start simple: Get plain MSG_DATA working first
 * 2. Test key exchange: Send '#' command
 * 3. Test encryption: Send '!message' after key exchange
 * 4. Test exit commands: '-' for client exit, '=' for server shutdown
 * 5. Test error cases: What if server closes unexpectedly?
 * 
 * Good luck! Remember: Focus on the socket operations. The crypto is done!
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdint.h>
#include "crypto-client.h"
#include "crypto-lib.h"
#include "protocol.h"

// Global buffers
char send_buffer_to_server[BUFFER_SIZE];
char recv_buffer_to_server[BUFFER_SIZE];


//recv_pdu_from_server() - same as server recv function, but for client. Ensures all bytes are recv'ed before continuing
ssize_t recv_pdu_from_server(int sockfd, uint8_t *buff, uint16_t max_length){
    size_t bytes_received = 0;
    ssize_t result;
    
    // get header
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
    if (pdu->header.payload_len > max_length) {
        printf("Message is too long!\n");
        return RC_MSG_TOO_LONG;
    }
    
    
    // get message data fully

    bytes_received = 0;
    while(bytes_received < pdu->header.payload_len) {
        result = recv(sockfd, pdu->payload + bytes_received, pdu->header.payload_len - bytes_received, 0);
        
        //check recv status
        if (result <= 0) {
            return result; 
        }
        bytes_received += result;
    }
    
    return sizeof(crypto_pdu_t) + pdu->header.payload_len;
}

// send_all_to_server() - makes sure all bytes are sent to the server
ssize_t send_all_to_server(int sockfd, const char* buffer, size_t length) {
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

// send_pdu_to_server() - send message as a PDU
ssize_t send_pdu_to_server(int sockfd, crypto_msg_t *pdu) {
    size_t pdu_len = sizeof(crypto_pdu_t) + pdu->header.payload_len;
    memcpy(send_buffer_to_server, pdu, pdu_len);
    
    return send_all_to_server(sockfd, send_buffer_to_server, pdu_len);
}


/*
 * int build_packet(const msg_cmd_t *cmd, crypto_msg_t *pdu, crypto_key_t key) {
 *     // 1. Set pdu->header.msg_type = cmd->cmd_id
 *     // 2. Set pdu->header.direction = DIR_REQUEST
 *     // 3. Based on cmd->cmd_id:
 *     //    - MSG_DATA: copy cmd->cmd_line to payload, set length
 *     //    - MSG_ENCRYPTED_DATA: encrypt cmd->cmd_line, set length
 *     //    - MSG_KEY_EXCHANGE: set length to 0
 *     //    - Command messages: set length to 0
 *     // 4. Return sizeof(crypto_pdu_t) + payload_len
 * }
*/
int build_packet(const msg_cmd_t *cmd, crypto_msg_t *pdu, crypto_key_t key) {
/*
*     // 1. Set pdu->header.msg_type = cmd->cmd_id
*     // 2. Set pdu->header.direction = DIR_REQUE
*/
    pdu->header.msg_type = cmd->cmd_id;
    pdu->header.direction = DIR_REQUEST;

    //based on cmd->cmd_id:
    if (cmd->cmd_id == MSG_DATA){

        //need to check if data is null:
        if (!cmd->cmd_line){
            printf("No Message Data\n");
            return RC_MSG_DATA_NOT_FOUND;
        }

        //get size of message
        size_t msg_len = strlen(cmd->cmd_line);

        //check size of message
        if (msg_len > MAX_MSG_DATA_SIZE){

            printf("Message Too Long\n");
            return RC_MSG_TOO_LONG;
        }

        //copy cmd->cmd_line to payload, set length
        memcpy(pdu->payload, cmd->cmd_line, msg_len);
        pdu->header.payload_len = (uint16_t)msg_len;

    } else if (cmd->cmd_id == MSG_ENCRYPTED_DATA){

        //check if there is a valid session key established
        if (key == NULL_CRYPTO_KEY){
            printf("No session key established. Cannot send encrypted data.\n");
            return RC_CRYPTO_ERR;
        }

        size_t msg_len = strlen(cmd->cmd_line);

        //check message size
        if (msg_len > MAX_MSG_DATA_SIZE){

            printf("Message Too Long\n");
            return RC_MSG_TOO_LONG;
        }

        //encrypt
        int encrypt_return = encrypt_string(key, pdu->payload, (uint8_t*)cmd->cmd_line, msg_len);


        //check for error with encryption, else returns encrypted length
        if (encrypt_return < 0){
            printf("An error occured during encryption\n");
            return RC_ENCRYPTION_ERR;
        }

        //no error, can set pdu length
        pdu->header.payload_len = encrypt_return;

    } else if (cmd->cmd_id == MSG_KEY_EXCHANGE){
        pdu->header.payload_len = 0;

    //command messages
    } else{
        pdu->header.payload_len = 0;
    }

    return sizeof(crypto_pdu_t) + pdu->header.payload_len;

 }



/*
 * int client_loop(int socket_fd) {
 *     // 1. Allocate buffers (send, receive, input)
 *     // 2. Initialize session_key to NULL_CRYPTO_KEY
 *     // 3. Loop:
 *     //    a) Call get_command() to get user input
 *     //    b) Build PDU from command (use helper function)
 *     //    c) Send PDU using send()
 *     //    d) If exit command, break after sending
 *     //    e) Receive response using recv()
 *     //    f) Handle recv() return values (0 = closed, <0 = error)
 *     //    g) Process response:
 *     //       - If MSG_KEY_EXCHANGE: extract key from payload
 *     //       - If MSG_ENCRYPTED_DATA: decrypt using decrypt_string()
 *     //       - Print results
 *     //    h) Loop back
 *     // 4. Free buffers
 *     // 5. Return success/error code
 * }
*/
int client_loop(int socket_fd) {
    uint8_t send_buff[BUFFER_SIZE];
    uint8_t recv_buff[BUFFER_SIZE];
    char input[MAX_MSG_DATA_SIZE];

    //initialize session key to null;
    crypto_key_t session_key = NULL_CRYPTO_KEY;

    while (1) {
        
        msg_cmd_t cmd;

        //get command
        int returned_cmd = get_command(input, MAX_MSG_DATA_SIZE, &cmd);

        //check if we need to execute
        if (returned_cmd == CMD_EXECUTE){
        
            crypto_msg_t* send_pdu = (crypto_msg_t*)send_buff;
            memset(send_buff, 0, sizeof(send_buff));

            int packet_len = build_packet(&cmd, send_pdu, session_key);

            //check if building packet was successful
            if (packet_len < 0){
                printf("Error building packet\n");
                continue;
            }

            //send pdu
            print_msg_info(send_pdu, session_key, CLIENT_MODE);
            int ret = send_pdu_to_server(socket_fd, (crypto_msg_t*)send_buff);

            //check if send was successful
            if (ret < 0){
                perror("Error sending");
                break;
            }

            //d) If exit command, break after sending

            if (cmd.cmd_id == MSG_CMD_CLIENT_STOP || cmd.cmd_id == MSG_CMD_SERVER_STOP){
                printf("Exiting Client...\n");
                return cmd.cmd_id;
            }

            //e) Receive response using recv()
            // f) Handle recv() return values (0 = closed, <0 = error)

            ssize_t resp_pdu_len = recv_pdu_from_server(socket_fd, recv_buff, MAX_MSG_DATA_SIZE);


            if (resp_pdu_len < 0){
                printf("Error receiving message from server.\n");
                break; // Close this client, wait for next one
            } else if (resp_pdu_len == 0) {
                printf("Server Disconnected.\n");
                break; // Close this client, wait for next one
            }

    //g) Process response:
     //       - If MSG_KEY_EXCHANGE: extract key from payload
      //       - If MSG_ENCRYPTED_DATA: decrypt using decrypt_string()
      //       - Print results

            crypto_msg_t *resp_pdu = (crypto_msg_t*)recv_buff;
            print_msg_info(resp_pdu, session_key, CLIENT_MODE);

            switch (resp_pdu->header.msg_type)
            {
            case MSG_DATA:

            //print server response
                printf("Server: %.*s\n", resp_pdu->header.payload_len, resp_pdu->payload);
                break;

            case MSG_KEY_EXCHANGE:

            //extract key from payload
                memcpy(&session_key, resp_pdu->payload, sizeof(crypto_key_t));
                printf("Client: Key Exchange Successful\n");
                break;

            case MSG_ENCRYPTED_DATA:

                char clear_string[BUFFER_SIZE];

                //decrypt string
                int decrypted_return = decrypt_string(session_key, (uint8_t*) clear_string,resp_pdu->payload, resp_pdu->header.payload_len);
                
                //check decryption for error
                if (decrypted_return < 0){
                    perror("Error with decryption");
                    return RC_DECRYPTION_ERR;
                }

                //null-term decrypted string
                clear_string[decrypted_return] = '\0';

                //print decrypted string from server
                printf("Server: %s\n", clear_string);
                break;
            
            default:
                break;
            }

        } else{ //no execution needed, continue to next command
            continue;
        }
    }

    return RC_OK;
}




/* =============================================================================
 * STUDENT TODO: IMPLEMENT THIS FUNCTION
 * =============================================================================
 * This is the main client function. You need to:
 * 1. Create a TCP socket
 * 2. Connect to the server
 * 3. Call your communication loop
 * 4. Clean up and close the socket
 * 
 * Parameters:
 *   addr - Server IP address (e.g., "127.0.0.1")
 *   port - Server port number (e.g., 1234)
 */
void start_client(const char* addr, int port) {

    int sockfd;
    struct sockaddr_in server_addr;

    //Create a TCP Socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, addr, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Error: Invalid address %s\n", addr);
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    //connect to the server

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%d\n", addr, port);
    printf("Type messages to send to server.\n");

    int ret = client_loop(sockfd);

    close(sockfd);
    printf("Client Disconnected\n");

    

}


/* =============================================================================
 * PROVIDED HELPER FUNCTION: get_command()
 * =============================================================================
 * This function is FULLY IMPLEMENTED for you. It handles user input and
 * interprets special command characters.
 * 
 * HOW TO USE:
 *   char input_buffer[MAX_MSG_DATA_SIZE];
 *   msg_cmd_t command;
 *   int result = get_command(input_buffer, MAX_MSG_DATA_SIZE, &command);
 *   if (result == CMD_EXECUTE) {
 *       // command.cmd_id contains the message type
 *       // command.cmd_line contains the message text (or NULL)
 *   } else {
 *       // CMD_NO_EXEC means skip this command (like '?' for help)
 *   }
 * 
 * COMMAND FORMAT:
 *   Regular text      -> MSG_DATA (plain text message)
 *   !<message>        -> MSG_ENCRYPTED_DATA (encrypt the message)
 *   #                 -> MSG_KEY_EXCHANGE (request encryption key)
 *   -                 -> MSG_CMD_CLIENT_STOP (exit client)
 *   =                 -> MSG_CMD_SERVER_STOP (shutdown server)
 *   ?                 -> Show help (returns CMD_NO_EXEC)
 * 
 * RETURN VALUES:
 *   CMD_EXECUTE  - Command should be sent to server (use cmd_id and cmd_line)
 *   CMD_NO_EXEC  - Command was handled locally (like help), don't send
 * 
 * IMPORTANT NOTES:
 *   - The returned cmd_line is a pointer into cmd_buff (no need to free)
 *   - For commands without data (like '#'), cmd_line will be NULL
 *   - For '!' commands, cmd_line points to text AFTER the '!' character
 */
int get_command(char *cmd_buff, size_t cmd_buff_sz, msg_cmd_t *msg_cmd)
{
    if ((cmd_buff == NULL) || (cmd_buff_sz == 0)) return CMD_NO_EXEC;

    printf("> ");
    fflush(stdout);
    
    // Get input from user
    if (fgets(cmd_buff, cmd_buff_sz, stdin) == NULL) {
        printf("[WARNING] Error reading input command.\n\n");
        return CMD_NO_EXEC;
    }
    
    // Remove trailing newline
    cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

    // Interpret the command based on first character
    switch (cmd_buff[0]) {
        case '!':
            // Encrypted message - everything after '!' is the message
            msg_cmd->cmd_id = MSG_ENCRYPTED_DATA;
            msg_cmd->cmd_line = cmd_buff + 1; // Skip the '!' character
            return CMD_EXECUTE;
            
        case '#':
            // Key exchange request - no message data
            msg_cmd->cmd_id = MSG_KEY_EXCHANGE;
            msg_cmd->cmd_line = NULL;
            return CMD_EXECUTE;
            
        case '$':
            // Digital signature (not implemented in this assignment)
            msg_cmd->cmd_id = MSG_DIG_SIGNATURE;
            msg_cmd->cmd_line = NULL;
            printf("[INFO] Digital signature command not implemented yet.\n\n");
            return CMD_NO_EXEC;
            
        case '-':
            // Client exit command
            msg_cmd->cmd_id = MSG_CMD_CLIENT_STOP;
            msg_cmd->cmd_line = NULL;
            return CMD_EXECUTE;
            
        case '=':
            // Server shutdown command
            msg_cmd->cmd_id = MSG_CMD_SERVER_STOP;
            msg_cmd->cmd_line = NULL;
            return CMD_EXECUTE;
            
        case '?':
            // Help - display available commands
            msg_cmd->cmd_id = MSG_HELP_CMD;
            msg_cmd->cmd_line = NULL;
            printf("Available commands:\n");
            printf("  <message>  : Send plain text message\n");
            printf("  !<message> : Send encrypted message (requires key exchange first)\n");
            printf("  #          : Request key exchange from server\n");
            printf("  ?          : Show this help message\n");
            printf("  -          : Exit the client\n");
            printf("  =          : Exit the client and request server shutdown\n\n");
            return CMD_NO_EXEC;
            
        default:
            // Regular text message
            msg_cmd->cmd_id = MSG_DATA;
            msg_cmd->cmd_line = cmd_buff;
            return CMD_EXECUTE;
    }
    
    return CMD_NO_EXEC;
}