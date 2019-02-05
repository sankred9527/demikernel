#include <dmtr/annot.h>
#include <dmtr/libos.h>
#include <dmtr/mem.h>

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>

#define ITERATION_COUNT 10000
#define BUFFER_SIZE 10
#define FILL_CHAR 0xab
static const uint16_t PORT = 12345;

int main()
{
    char *argv[] = {};
    DMTR_OK(dmtr_init(0, argv));

    int qd = 0;
    DMTR_OK(dmtr_socket(&qd, AF_INET, SOCK_STREAM, 0));
    printf("client qd:\t%d\n", qd);

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr) != 1) {
        printf("Address not supported!\n");
        return -1;
    }
    saddr.sin_port = htons(PORT);
    DMTR_OK(dmtr_connect(qd, (struct sockaddr*)&saddr, sizeof(saddr)));

    dmtr_sgarray_t sga = {};
    void *p = NULL;
    DMTR_OK(dmtr_malloc(&p, BUFFER_SIZE));
    memset(p, FILL_CHAR, BUFFER_SIZE);
    sga.sga_numsegs = 1;
    sga.sga_segs[0].sgaseg_len = BUFFER_SIZE;
    sga.sga_segs[0].sgaseg_buf = p;

    for (int i = 0; i < ITERATION_COUNT; i++) {
        dmtr_qtoken_t qt;
        DMTR_OK(dmtr_push(&qt, qd, &sga));
        DMTR_OK(dmtr_wait(NULL, qt));

        dmtr_sgarray_t recvd;
        DMTR_OK(dmtr_pop(&qt, qd));
        DMTR_OK(dmtr_wait(&recvd, qt));
        DMTR_TRUE(EPERM, recvd.sga_numsegs == 1);
        DMTR_TRUE(EPERM, reinterpret_cast<uint8_t *>(recvd.sga_segs[0].sgaseg_buf)[0] == FILL_CHAR);
        //fprintf(stderr, "client: rcvd\t%s\tbuf size:\t%d\n", (char*)recvd.bufs[0].buf, recvd.bufs[0].len);
        free(recvd.sga_buf);
    }

    DMTR_OK(dmtr_close(qd));

    return 0;
}
