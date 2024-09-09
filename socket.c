#include "raft.h"
#include "store.h"

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
        printf("Node %d received: %s\n", node->node_id, buffer);

        if (strncmp(buffer, "REQUEST_VOTE", 12) == 0) {
            int term, candidate_id;
            sscanf(buffer, "REQUEST_VOTE %d %d", &term, &candidate_id);

            // TODO: 투표 요청 처리 나 보다 임기가 작은 아이의 요청

            if (node->voted_for == -1) {
                node->voted_for = candidate_id;

                sendto(node->socket_fd, "VOTE_GRANTED", 12, 0, (const struct sockaddr*)&addr, len);
            }
        } else if (strncmp(buffer, "VOTE_GRANTED", 12) == 0) {
            node->votes++;

            if (node->votes > node->num_nodes / 2) {
                node->state = LEADER;
                node->leader_ip = inet_ntoa(node->addr.sin_addr);
                node->leader_port = ntohs(node->addr.sin_port);

                printf("Node %d is now the leader\n", node->node_id);

                for (int i = 1; i < node->num_nodes; i++) {
                    sendto(node->socket_fd, "LEADER_ELECTED", 14, 0, (const struct sockaddr*)&nodes[i], len);
                }
            }
        } else if (strncmp(buffer, "LEADER_ELECTED", 14) == 0) {
            node->state = FOLLOWER;

            printf("Node %d is now a follower\n", node->node_id);
        } else if (strncmp(buffer, "HEARTBEAT", 9) == 0) {
            char leader_ip[MAX_IP_LENGTH];
            int term, leader_port;
            sscanf(buffer,  "HEARTBEAT %d IP %s PORT %d", &term, leader_ip, &leader_port);

            // 나보다 임기가 작은 아이의 heartbeat는 무시
            if (term < node->current_term) {
                continue;
            }

            node->current_term = term;
            node->last_heartbeat = time(NULL);
            node->voted_for = -1;
            node->votes = 0;
            node->state = FOLLOWER;
            node->leader_ip = strdup(leader_ip);
            node->leader_port = leader_port;
            node->election_timeout = (double)(rand() % 10) / 10 + 2.0;
        } else if (strncmp(buffer, "get", 3) == 0) {
            char key[MAX_KEY_LENGTH];
            sscanf(buffer, "get %s", key);

            get(key);
        } else if (strncmp(buffer, "put", 3) == 0) {
            char key[MAX_KEY_LENGTH];
            char value[MAX_VALUE_LENGTH];
            sscanf(buffer, "put %s %s", key, value);

            put(key, value);

            if (node->state == LEADER) {
                for (int i = 1; i < node->num_nodes; i++) {
                    sendto(node->socket_fd, buffer, strlen(buffer), 0, (const struct sockaddr*)&nodes[i], len);
                }
            }
        } else if (strncmp(buffer, "member add", 10) == 0) {
            char ip[MAX_IP_LENGTH];
            int port;
            sscanf(buffer, "member add %[^:]:%d", ip, &port);

            struct sockaddr_in new_node;
            memset(&new_node, 0, sizeof(struct sockaddr_in));
            new_node.sin_family = AF_INET;
            new_node.sin_port = htons(port);
            int n = inet_pton(AF_INET, ip, &new_node.sin_addr);

            nodes[node->num_nodes] = new_node;
            node->num_nodes++;

            if (node->state == LEADER) {
                for (int i = 1; i < node->num_nodes; i++) {
                    sendto(node->socket_fd, buffer, strlen(buffer), 0, (const struct sockaddr*)&nodes[i], len);
                }
            }
        }
    }

    return NULL;
}
