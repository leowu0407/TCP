#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctime>
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
int seq_num,ack_num;
string s;
unsigned int getSeed() {
    pid_t pid = getpid();
    time_t now = time(NULL);
    unsigned int seed = static_cast<unsigned int>(pid) ^ static_cast<unsigned int>(now);
    return seed;
}
int sizeat=1;
int main()
{
    string ip_address = "127.0.0.1";
    unsigned int seed = getSeed();
    srand(seed);
    seq_num = rand()%10000+1;
    int sock_fd;
    struct sockaddr_in server;
    int status;
    string s;
    
    // 創建socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);   //使用tcp
    if (sock_fd == -1) {         //創建失敗
        cout << "Socket creation error" << endl;
        return 1;
    }
    Packet send_packet, receive_packet;
    // server address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;  // 使用 INADDR_ANY 表示任意 IP 地址
    server.sin_port = htons(1234);
    status = connect(sock_fd, reinterpret_cast<struct sockaddr*>(&server), sizeof(server));
    if (status == -1) {
        perror("Connection error");
        return 1;
    }
    cout << "Server's IP Address: " << ip_address << endl;
    cout << "Server's Port: " << htons(server.sin_port) << endl;
    struct sockaddr_in client;
    client.sin_addr.s_addr = inet_addr(ip_address.c_str());
    client.sin_port = htons(1234);
    send_packet.h.seq_num = seq_num;
    send_packet.h.ack_num = 0;
    memset(send_packet.data, '\0', sizeof(send_packet.data));
    //three-way handshake
    cout << "=====start a three-way handshake=====" << endl;
    cout << "Send a packet(SYN) to " << ip_address << " : " << htons(server.sin_port) << endl;
    send(sock_fd, &send_packet, sizeof(Header) + strlen(send_packet.data) + 1, 0);
    cout << "       Send a packet at : " << sizeat << " byte" << endl;
    sizeat += sizeof(send_packet.data);
    recv(sock_fd, &receive_packet, sizeof(receive_packet), 0);
    client.sin_port = htons(receive_packet.h.port);
    server.sin_port = htons(receive_packet.h.port);
    //ack_num+=strlen(indata);
    cout << "Receive a packet(SYN/ACK) from " << ip_address << " : " << htons(server.sin_port) << endl;
    cout << "Receive a packet(seq_num = " << send_packet.h.seq_num << ", ack_num = " << receive_packet.h.seq_num+1 << ")" << endl;
    memset(send_packet.data, '\0', sizeof(send_packet.data));
    strcpy(send_packet.data,"receive");
    send(sock_fd, &send_packet, sizeof(Header) + strlen(send_packet.data) + 1, 0);
    //seq_num += (strlen(outdata)+1);
    //ack_num += (strlen(indata)+1);
    cout << "Send a packet(ACK) to " << ip_address << " : " << htons(server.sin_port) << endl;
    cout << "       Send a packet at : " << sizeat << " byte" << endl;
    sizeat += sizeof(send_packet.data);
    cout << "=====complete the three-way handshake=====" << endl;
    
    while (1) {
        cout << "Enter command: ";
        getline(cin,s);
        memset(send_packet.data, '\0', sizeof(send_packet.data));
        strcpy(send_packet.data,s.c_str());
        cout << "send a command." << endl;
        send(sock_fd, &send_packet, sizeof(Header) + strlen(send_packet.data) + 1, 0);
        cout << "       Send a packet at : " << sizeat << " byte" << endl;
        sizeat += sizeof(send_packet.data);
        //dns
        if(send_packet.data[0] == 'd'){
            s = send_packet.data+4;
            cout << "finding IP address of " << s << " by " << ip_address << " : " << htons(server.sin_port) << endl;
            recv(sock_fd, &receive_packet, sizeof(receive_packet), 0);
            send_packet.h.seq_num += 1024;
            cout << "Receive a packet(seq_num = " << send_packet.h.seq_num << ", ack_num = " << receive_packet.h.seq_num+1 << ")" << endl;
            cout << "Result: " << receive_packet.data << endl;
            memset(send_packet.data, '\0', sizeof(send_packet.data));
            strcpy(send_packet.data,"receive");
            send(sock_fd, &send_packet, sizeof(Header) + strlen(send_packet.data) + 1, 0);
            cout << "Send a packet(ACK) to " << ip_address << " : " << htons(server.sin_port) << endl;
            cout << "       Send a packet at : " << sizeat << " byte" << endl;
            sizeat += sizeof(send_packet.data);
            cout << "Finish finding" << endl << s << endl;
        }
        //calculation
        else if(send_packet.data[0] == 'c'){
            s = send_packet.data+4;
            cout << "caculating " << s << " by " << ip_address << " : " << htons(server.sin_port) << endl;
            recv(sock_fd, &receive_packet, sizeof(receive_packet), 0);
            send_packet.h.seq_num += 1024;
            cout << "Receive a packet(seq_num = " << send_packet.h.seq_num << ", ack_num = " << receive_packet.h.seq_num+1 << ")" << endl;
            cout << "Result: " << receive_packet.data << endl;
            memset(send_packet.data, '\0', sizeof(send_packet.data));
            strcpy(send_packet.data,"receive");
            send(sock_fd, &send_packet, sizeof(Header) + strlen(send_packet.data) + 1, 0);
            cout << "Send a packet(ACK) to " << ip_address << " : " << htons(server.sin_port) << endl;
            cout << "       Send a packet at : " << sizeat << " byte" << endl;
            sizeat += sizeof(send_packet.data);
            cout << "Finish caculation" << endl;
        }
        //get video
        else{
            s = send_packet.data+4;
            /*FILE *fp;
            fp = fopen(s.c_str(), "wb");
            fseek(fp, 0, SEEK_SET);
            fclose(fp);*/
            
            cout << "receive video: " << s << " from " << ip_address << " : " << htons(server.sin_port) << endl;
            while(1){
                recv(sock_fd, &receive_packet, sizeof(receive_packet), 0);
                send_packet.h.seq_num += 1024;
                cout << "Receive a packet(seq_num = " << send_packet.h.seq_num << ", ack_num = " << receive_packet.h.seq_num+1 << ")" << endl;
                if(receive_packet.h.end == 1){
                    break;
                }
                send(sock_fd, &send_packet, sizeof(send_packet), 0);
                cout << "Send a packet(ACK) to " << ip_address << " : " << htons(server.sin_port) << endl;
                cout << "       Send a packet at : " << sizeat << " byte" << endl;
                sizeat += sizeof(send_packet.data);
            }
            cout << "receive video " << endl << s << endl;
        }
        
    }

    return 0;
}