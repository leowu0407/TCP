#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <thread>
#include <cmath>
#include <fstream>
#include <netdb.h>
#include <sstream>
#include <random>
#include <sys/time.h>

using namespace std;
typedef struct header{
    int ack_num;
    int seq_num;
    int port;
    int end;
}Header;
typedef struct packet{
    Header h;
    char data[1024];
}Packet;

int serverPort = 1234;
int client_num = 0;
string s;

unsigned int getSeed() {
    pid_t pid = getpid();
    time_t now = time(NULL);
    unsigned int seed = static_cast<unsigned int>(pid) ^ static_cast<unsigned int>(now);
    return seed;
}
void handleClient(int new_fd) {
    int cwnd = 1, RTT = 20, MSS = 1024, threshold = 64*1024, rwnd = 512*1024;
    int sizeat = 1;
    int seq_num, ack_num;
    int send_size, file_size, already_send;
    unsigned int seed = getSeed();
    srand(seed);
    seq_num = rand() % 10000 + 1;
    ack_num = 0;
    Packet send_packet, receive_packet;
    send_packet.h.seq_num = seq_num;
    send_packet.h.ack_num = receive_packet.h.seq_num;
    send_packet.h.port = serverPort + client_num;
    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(serverPort+client_num);
    recv(new_fd, &receive_packet, sizeof(receive_packet), 0);
    cout << "Receive a packet (seq_num = " << send_packet.h.seq_num << ", ack_num = 0)" << endl;
    //seq_num++;
    //ack_num += (strlen(indata) + 1);
    cout << "cwnd = " << cwnd << ", rwnd = " << rwnd << ", threshold = " << threshold << endl;
    cwnd *= 2;
    memset(send_packet.data, '\0', sizeof(send_packet.data));
    send(new_fd, &send_packet, sizeof(Header) + strlen(send_packet.data) + 1, 0);
    send_packet.h.seq_num++;
    cout << "       Send a packet at : " << sizeat << " byte" << endl;
    sizeat += sizeof(send_packet.data);
    recv(new_fd, &receive_packet, sizeof(receive_packet), 0);
    cout << "Receive a packet (seq_num = " << send_packet.h.seq_num << ", ack_num = " << receive_packet.h.seq_num+1 << ")" << endl;
    //seq_num++;
    //ack_num += strlen(indata);
    while (1) {
        cout << ">>Waiting command<<" << endl;
        while(1){
            struct timeval timeout;
            timeout.tv_sec = (int)0.3;
            timeout.tv_usec = (int)((0.3 - timeout.tv_sec) * 1e6);

            if (setsockopt(new_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
                perror("setsockopt failed");
                exit(EXIT_FAILURE);
            }
            ssize_t bytes_received = recv(new_fd, &receive_packet, sizeof(receive_packet), 0);
            if (bytes_received < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;
                } 
                else {
                    perror("recv failed");
                }
            } 
            else if (bytes_received == 0) {
                cout << "client already left." << endl;
                exit(0);
            } 
            else {
                break;
            }
        }
        cout << "Receive a packet (seq_num = " << send_packet.h.seq_num << ", ack_num = " << receive_packet.h.seq_num+1 << ")" << endl;
        cout << receive_packet.data << endl;
        if (receive_packet.data[0] == 'd') {
            s = receive_packet.data + 4;
            cout << "==Start finding: " << s << " to 127.0.0.1" << " : " << htons(serverPort) << endl;
            cout << "cwnd = " << cwnd << ", rwnd = " << rwnd << ", threshold = " << threshold << endl;
            struct hostent* hostInfo = gethostbyname(s.c_str());
            if (hostInfo != nullptr && hostInfo->h_addr_list[0] != nullptr) {
                struct in_addr address;
                memcpy(&address, hostInfo->h_addr_list[0], sizeof(struct in_addr));
                string ipAddress = inet_ntoa(address);
                memset(send_packet.data, '\0', sizeof(send_packet.data));
                strcpy(send_packet.data,ipAddress.c_str());
                send(new_fd, &send_packet, sizeof(Header) + strlen(send_packet.data) + 1, 0);
                send_packet.h.seq_num++;
                cout << "       Send a packet at : " << sizeat << " byte" << endl;
                sizeat += sizeof(send_packet.data);
                recv(new_fd, &receive_packet, sizeof(receive_packet), 0);
                cout << "Receive a packet (seq_num = " << send_packet.h.seq_num << ", ack_num = " << receive_packet.h.seq_num+1 << ")" << endl;
                cout << "==Finish finding==" << endl;
            }
            else {
                string errorMessage = "can't find : " + s;
                memset(send_packet.data, '\0', sizeof(send_packet.data));
                strcpy(send_packet.data,errorMessage.c_str());
                send(new_fd, &send_packet, sizeof(Header) + strlen(send_packet.data) + 1, 0);
                send_packet.h.seq_num++;
                recv(new_fd, &receive_packet, sizeof(receive_packet), 0);
            }
        }
        else if (receive_packet.data[0] == 'c') {
            s = receive_packet.data + 4;
            int num1, num2;
            string operation = s;
            istringstream iss(operation);
            int result = 0;
            cout << "==Start calculation to 127.0.0.1 : " << htons(send_packet.h.port) << endl;
            cout << "cwnd = " << cwnd << ", rwnd = " << rwnd << ", threshold = " << threshold << endl;
            if (!(iss >> num1 >> operation >> num2)) {
                if (operation == "sqrt") {
                    result = sqrt(num1);
                }
                else
                    cout << "Error: Invalid input format." << endl;

            }
            else {
                if (operation == "add") {
                    result = num1 + num2;
                }
                else if (operation == "sub") {
                    result = num1 - num2;
                }
                else if (operation == "mul") {

                    result = num1 * num2;
                }
                else if (operation == "div") {
                    if (num2 != 0)
                        result = num1 / num2;
                    else
                        cout << "Error: Division by zero." << endl;
                }
                else if (operation == "pow") {
                    result = pow(num1, num2);
                }
                else if (operation == "sqrt") {
                    result = sqrt(num1);
                }
                else {
                    cout << "Error: Invalid operation." << endl;
                }
            }
            string resultStr = to_string(result);
            memset(send_packet.data, '\0', sizeof(send_packet.data));
            strcpy(send_packet.data,resultStr.c_str());
            send(new_fd, &send_packet, sizeof(Header) + strlen(send_packet.data) + 1, 0);
            send_packet.h.seq_num++;
            cout << "       Send a packet at : " << sizeat << " byte" << endl;
            sizeat += sizeof(send_packet.data);
            recv(new_fd, &receive_packet, sizeof(receive_packet), 0);
            cout << "Receive a packet (seq_num = " << send_packet.h.seq_num << ", ack_num = " << receive_packet.h.seq_num << ")" << endl;
            cout << "==Finish calculation==" << endl;
        }
        else if(receive_packet.data[0] == 'g') {
            send_packet.h.end = 0;
            s = receive_packet.data + 4;
            FILE* file;
            file = fopen(s.c_str(), "rb");
            fseek(file, 0, SEEK_END); //pointer移到最後
            file_size = ftell(file);  //計算檔案大小
            cout << "==Sending video : " << s << " to 127.0.0.1 : " << htons(serverPort + client_num) << endl;
            cout << "file size : " << file_size << endl;
            memset(send_packet.data, '\0', sizeof(send_packet.data));
            already_send = 0;
            while (file_size > 0) {
                cout << "cwnd = " << cwnd << ", rwnd = " << rwnd << ", threshold = " << threshold << endl;
                for(int i=0;i<cwnd/2;i++){
                    if (file_size > MSS) {
                        send_size = MSS;
                    } 
                    else {
                        send_packet.h.end = 1;
                        send_size = file_size;
                    }
                    
                    
                    memset(send_packet.data, '\0', sizeof(send_packet.data));
                    fseek(file, already_send, SEEK_SET);
                    fread(send_packet.data, sizeof(char), send_size, file);
                    send_packet.h.seq_num++;
                    send(new_fd, &send_packet, sizeof(Header) + strlen(send_packet.data) + 1, 0);
                    cout << "       Send a packet at : " << sizeat << " byte" << endl;
                    sizeat += sizeof(send_packet.data);
                    rwnd -= send_size;
                    file_size -= send_size;
                    already_send += send_size;
                    if (!file_size) {
                        break;
                    }
                    if (file_size > MSS) {
                        send_size = MSS;
                    } 
                    else {
                        send_packet.h.end = 1;
                        send_size = file_size;
                    }
                    
                    memset(send_packet.data, '\0', sizeof(send_packet.data));
                    fseek(file, already_send, SEEK_SET);
                    fread(send_packet.data, sizeof(char), send_size, file);
                    send(new_fd, &send_packet, sizeof(Header) + strlen(send_packet.data) + 1, 0);
                    rwnd -= send_size;
                    file_size -= send_size;
                    already_send += send_size;
                    if (!file_size) {
                        break;
                    }
                    
                    while(1){
                        struct timeval timeout;
                        timeout.tv_sec = (int)0.3;
                        timeout.tv_usec = (int)((0.3 - timeout.tv_sec) * 1e6);

                        if (setsockopt(new_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
                            perror("setsockopt failed");
                            exit(EXIT_FAILURE);
                        }
                        ssize_t bytes_received = recv(new_fd, &receive_packet, sizeof(receive_packet), 0);
                        if (bytes_received < 0) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                send(new_fd, &send_packet, sizeof(Header) + strlen(send_packet.data) + 1, 0);
                            } 
                            else {
                                perror("recv failed");
                            }
                        } 
                        else if (bytes_received == 0) {
                            break;
                        } 
                        else {
                            cout << "       Receive an ACK packet (seq_num = " << send_packet.h.seq_num << ", ack_num = " << receive_packet.h.seq_num-sizeof(send_packet.data)+1 << ")" << endl;
                            cout << "       Send a packet at : " << sizeat << " byte" << endl;
                            sizeat += sizeof(send_packet.data);
                            break;
                        }
                    }
                
                    if (!file_size) {
                        break;
                    }
                }
                cwnd*=2;
            }
        }
        
    }

    close(new_fd);
}

int main() {
    int sock_fd;
    struct sockaddr_in my_addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        cout << "Socket creation error" << endl;
        return 1;
    }

    my_addr.sin_family = AF_INET;
    inet_aton("0.0.0.0", &my_addr.sin_addr);
    my_addr.sin_port = htons(serverPort);
    int optval = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    int status = bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(my_addr));
    if (status == -1) {
        perror("Binding error");
        exit(1);
    }

    status = listen(sock_fd, 5);
    if (status == -1) {
        perror("Listening error");
        exit(1);
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (new_fd == -1) {
            perror("Accept error");
            continue;
        }
        client_num++;
        thread clientThread(handleClient, new_fd);
        clientThread.detach();
    }

    close(sock_fd);
    return 0;
}