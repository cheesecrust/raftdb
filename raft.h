#ifndef RAFT_H
#define RAFT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define NUM_NODES 3  // 노드 수를 3으로 설정 (필요에 따라 변경 가능)

typedef enum { FOLLOWER, CANDIDATE, LEADER } State;

typedef struct {
    int node_id;
    int current_term;
    int voted_for;
    State state;
    int leader_id;
    int socket_fd;
    struct sockaddr_in addr;
    time_t last_heartbeat;
    double election_timeout;
} RaftNode;

void init_node(RaftNode* node, int id, const char* ip, int port);
void* run_node(void* arg);
void follower_behavior(RaftNode* node);
void candidate_behavior(RaftNode* node, RaftNode nodes[]);
void leader_behavior(RaftNode* node);
void send_heartbeat(RaftNode* node, RaftNode nodes[]);
void receive_heartbeat(RaftNode* node, int term, int leader_id);
int request_vote(RaftNode* node, RaftNode nodes[], int term, int candidate_id);

#endif // RAFT_H
