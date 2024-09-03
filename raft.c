#include "raft.h"

void* run_node(void* arg) {
    RaftNode* node = (RaftNode*)arg;
    RaftNode nodes[NUM_NODES];  // 클러스터에 있는 다른 노드들

    while (1) {
        switch (node->state) {
            case FOLLOWER:
                follower_behavior(node);
                break;
            case CANDIDATE:
                candidate_behavior(node, nodes);
                break;
            case LEADER:
                leader_behavior(node);
                break;
        }
        usleep(100000); // 0.1초 간격으로 실행
    }
    return NULL;
}

void follower_behavior(RaftNode* node) {
    if (difftime(time(NULL), node->last_heartbeat) > node->election_timeout) {
        node->state = CANDIDATE;
        printf("Node %d timed out, becoming candidate\n", node->node_id);
    }
}

void candidate_behavior(RaftNode* node, RaftNode nodes[]) {
    node->current_term++;
    node->voted_for = node->node_id;
    int votes = 1; // 자신에게 투표

    printf("Node %d is starting an election for term %d\n", node->node_id, node->current_term);

    // 투표 요청 전송
    for (int i = 0; i < NUM_NODES; i++) {
        if (i != node->node_id) {
            if (request_vote(node, nodes, node->current_term, node->node_id)) {
                votes++;
            }
        }
    }

    if (votes > NUM_NODES / 2) {
        node->state = LEADER;
        node->leader_id = node->node_id;
        printf("Node %d is elected as leader for term %d\n", node->node_id, node->current_term);
    } else {
        node->state = FOLLOWER;
    }
}

void leader_behavior(RaftNode* node) {
    printf("Node %d is sending heartbeats to followers\n", node->node_id);
    send_heartbeat(node, node);  // 이 부분을 수정해서 전체 노드에게 보냅니다.
    sleep(1); // heartbeat 간격
}

int request_vote(RaftNode* node, RaftNode nodes[], int term, int candidate_id) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    char buffer[1024];

    sprintf(buffer, "REQUEST_VOTE %d %d", term, candidate_id);
    sendto(node->socket_fd, buffer, strlen(buffer), 0, (const struct sockaddr*)&node->addr, sizeof(node->addr));

    int n = recvfrom(node->socket_fd, (char*)buffer, 1024, MSG_WAITALL, (struct sockaddr*)&addr, &len);
    buffer[n] = '\0';

    if (strcmp(buffer, "VOTE_GRANTED") == 0) {
        return 1;
    }

    return 0;
}

void send_heartbeat(RaftNode* node, RaftNode nodes[]) {
    char buffer[1024];
    sprintf(buffer, "HEARTBEAT %d %d", node->current_term, node->node_id);

    for (int i = 0; i < NUM_NODES; i++) {
        if (i != node->node_id) {
            sendto(node->socket_fd, buffer, strlen(buffer), 0, (const struct sockaddr*)&nodes[i].addr, sizeof(nodes[i].addr));
        }
    }
}

void receive_heartbeat(RaftNode* node, int term, int leader_id) {
    if (term >= node->current_term) {
        node->current_term = term;
        node->leader_id = leader_id;
        node->state = FOLLOWER;
        node->last_heartbeat = time(NULL);
    }
}
