#include "raft.h"

void* run_socket(void* arg) {
    thread_args* args = (thread_args*) arg;
    RaftNode* node = args->node;
    struct sockaddr_in* nodes = args->nodes;

    while (1) {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);

        recvfrom(node->socket_fd, (char*)buffer, 1024, MSG_WAITALL, (struct sockaddr*)&addr, &len);

        if (strncmp(buffer, "REQUEST_VOTE", 12) == 0) {
            int term, candidate_id;
            sscanf(buffer, "REQUEST_VOTE %d %d", &term, &candidate_id);
            if (term > node->current_term) {
                node->current_term = term;
                node->state = FOLLOWER;
                node->voted_for = -1;
            }
            if (node->voted_for == -1) {
                node->voted_for = candidate_id;
                sendto(node->socket_fd, "VOTE_GRANTED", 12, 0, (const struct sockaddr*)&addr, len);
            }
        } else if (strncmp(buffer, "HEARTBEAT", 9) == 0) {
            int term, leader_id;
            sscanf(buffer, "HEARTBEAT %d %d", &term, &leader_id);
            if (term >= node->current_term) {
                node->current_term = term;
                node->leader_id = leader_id;
                node->state = FOLLOWER;
            }
        }
    }
}