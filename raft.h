#ifndef RAFT_H
#define RAFT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>

#define NUM_NODES 3  // 노드 수를 3으로 설정 (필요에 따라 변경 가능)
#define MAX_IP_LENGTH 16

typedef enum { FOLLOWER, CANDIDATE, LEADER } State;

typedef struct {
    int node_id;
    int current_term;
    int voted_for;
    int votes;
    State state;
    char* leader_ip;
    int leader_port;
    int socket_fd;
    struct sockaddr_in addr;
    struct sockaddr_in leader;
    time_t last_heartbeat;
    double election_timeout;
} RaftNode;

typedef struct {
    RaftNode* node;
    struct sockaddr_in nodes[3];
} thread_args;

void init_node(RaftNode* node, int id, const char* ip, int port);
void* run_node(void* arg);
void* run_socket(void* arg);
void follower_behavior(RaftNode* node);
void candidate_behavior(RaftNode* node, struct sockaddr_in* nodes);
void leader_behavior(RaftNode* node, struct sockaddr_in* nodes);
void send_heartbeat(RaftNode* node, struct sockaddr_in* nodes);
int request_vote(RaftNode* node, struct sockaddr_in* nodes, int term, int candidate_id);

#endif // RAFT_H
