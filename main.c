#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <stdint.h>
#include <stdlib.h>

int DAT_00120828_uart_fd;

int FUN_001158f0_getBoardType()
{
    return 1;
}

void FUN_0010bf5c_IpcUartConfig(void)
{
    int fd;
    int iVar1;
    struct termios local_68;

    fd = open("/dev/ser2", 0x802);
    if (fd < 0) {
        fd = -1;
        printf("Cannot open UART\n");
        return;
    }

    tcflush(fd, 2);
    iVar1 = tcgetattr(fd, &local_68);
    if (iVar1 < 0) {
        printf("Cannot get port atr\n");
        close(fd);
        fd = -1;
        return;
    }

    cfmakeraw(&local_68);
    iVar1 = FUN_001158f0_getBoardType();
    if (iVar1 == 1) {
        printf (" Configuring UART of 500000 Kpbs Speed\n");
        cfsetospeed(&local_68,500000);
        cfsetispeed(&local_68,500000);
    }
    else {
        printf (" Configuring UART of 250000 Kpbs Speed");
        cfsetospeed(&local_68,0x3e800);
        cfsetispeed(&local_68,0x3e800);
    }
    local_68.c_cflag = local_68.c_cflag & 0xfffffe8c | 0x30;
    local_68.c_iflag = local_68.c_iflag & 0xfffffa14;
    local_68.c_cc[15] = '\x01';
    local_68.c_cc[16] = '\0';
    iVar1 = tcsetattr(fd, 1, &local_68);
    if (iVar1 < 0) {
        printf("Cannot change port atr\n");
        close(fd);
        fd = -1;
        return;
    }

    tcflush(fd, 2);
    DAT_00120828_uart_fd = fd;
}
#if 0
void IpcLlDrvReceiveTask(void *param_1)

{
    uint uVar1;
    uint8_t *puVar2;
    int iVar3;
    ssize_t sVar4;
    undefined auStack4180 [36];
    __fd_mask local_1030;
    uint8_t local_102c [4096];

    std::ios_base::Init::Init((Init *)local_102c);
    iVar3 = setprio(0,0x3c);
    if (iVar3 == -1) {
        printf("IpcLIDrv Recv task Set prio err\n");
    }
    iVar3 = FUN_001075a0_sem_wait_wrp(*(undefined4 *)PTR_g_phy_recv_trigger_00120814);
    if (iVar3 == -1) {
        printf("g_phy_recv_trigger semaphore fail \n");
    }
    auStack4180._32_4_ = 0;
    local_1030 = 2000;
    do {
        while( 1 ) {
            while( 1 ) {
                std::ios_base::Init::Init((Init *)(fd_set *)auStack4180);
                iVar3 = DAT_00120828_uart_fd + 0x1f;
                if (-1 < DAT_00120828_uart_fd) {
                    iVar3 = DAT_00120828_uart_fd;
                }
                uVar1 = (uint)(DAT_00120828_uart_fd >> 0x1f) >> 0x1b;
                *(uint *)(auStack4180 + (iVar3 >> 5) * 4) =
                        *(uint *)(auStack4180 + (iVar3 >> 5) * 4) |
                        1 << ((DAT_00120828_uart_fd + uVar1 & 0x1f) - uVar1 & 0xff);
                iVar3 = select(DAT_00120828_uart_fd + 1,(fd_set *)auStack4180,(fd_set *)0x0,(fd_set *)0x0,
                               (timeval *)(auStack4180 + 0x20));
                if (iVar3 != -1) break;
                if (*(int *)(PTR_g_PasLogParams_001207a8 + 0x2f14) < 0) {
                    NsLog(0x4f7,0x1f,"IpcLlDrvReceiveTask","Error in poll");
                }
            }
            if (0 < iVar3) break;
            LAB_0010d828:
            FUN_0010d640_COBS_decode_cond();
        }
        iVar3 = DAT_00120828_uart_fd + 0x1f;
        if (-1 < DAT_00120828_uart_fd) {
            iVar3 = DAT_00120828_uart_fd;
        }
        uVar1 = (uint)(DAT_00120828_uart_fd >> 0x1f) >> 0x1b;
        if ((*(int *)(auStack4180 + (iVar3 >> 5) * 4) >>
                                                      ((DAT_00120828_uart_fd + uVar1 & 0x1f) - uVar1 & 0xff) & 1U) == 0) goto LAB_0010d828;
        sVar4 = read(DAT_00120828_uart_fd,local_102c,0x1000);
        if ((1 < sVar4 + 1U) &&
            (*(ssize_t *)PTR_g_iUart_receive_byte_001207e0 =
                     sVar4 + *(int *)PTR_g_iUart_receive_byte_001207e0, 0 < sVar4)) {
            iVar3 = 0;
            do {
                puVar2 = local_102c + iVar3;
                iVar3 = iVar3 + 1;
                FUN_0010d3fc_COBS_decode(*puVar2);
            } while (iVar3 < sVar4);
        }
    } while( 1 );
}
#endif

#define RX_BUFF_SZ (10240)

struct R {
    int curr_idx;
    uint8_t rx_buffer[RX_BUFF_SZ]; //10K
};

struct R gR = {
        .curr_idx = 0,
        .rx_buffer = {0}
};

void write_uart_delim()
{
    uint8_t buff = 0x0;
    int res = write(DAT_00120828_uart_fd, &buff, 1);
    if(res < 0) {
        printf("write delim error\n");
    }
}

void write_packet(uint8_t *buff, int sz)
{
	int i;

    printf("Write: ");
    for(i = 0; i < sz; ++i)
        printf("0x%02x ", buff[i]);
    printf("\n");

    write_uart_delim();

    int res = write(DAT_00120828_uart_fd, buff, sz);
    if(res < 0) {
        printf("write error\n");
    }

    write_uart_delim();
}

void send_link_setup_request()
{
    static int req_num = 0;
    uint8_t ipc_link_setup_req[] =   {0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0xe4, 0xf3, 0x12, 0x3a, 0x21, 0xe0, 0x03, 0x63, 0x01};
    uint8_t ipc_link_setup_req_2[] = {0x02, 0x01, 0x04, 0x01, 0x00, 0x01, 0xe4, 0xf3, 0x12, 0x3a, 0x21, 0xe0, 0x03, 0x63, 0x01};

	printf("Sent a Link Setup Request\n");

    if(!req_num) {
        write_packet(ipc_link_setup_req, sizeof(ipc_link_setup_req));
    } else {
        ipc_link_setup_req_2[4] = ipc_link_setup_req_2[4] + req_num;
        ipc_link_setup_req_2[13] = ipc_link_setup_req_2[13] + req_num;
        write_packet(ipc_link_setup_req_2, sizeof(ipc_link_setup_req_2));
    }

    req_num++;
}

void send_link_setup_response(uint8_t req_num, uint8_t req_num_2)
{
    uint8_t ipc_link_resp[] = {0x02, 0x01, 0xe2, 0x81, 0xAA, 0xd4, 0x03, 0xAA, 0x01};
    ipc_link_resp[4] = req_num;
    ipc_link_resp[7] = req_num_2 + 0x20;
    write_packet(ipc_link_resp, sizeof(ipc_link_resp));
}

void send_wtd()
{
    static int wtd_cnt = 0;
    uint8_t wdt_msg[] = {0x02, 0x11, 0x02, 0x01, 0xe1, 0x12};
    uint8_t wdt_msg_2[] = {0x02, 0x11, 0xe3, 0x01, 0x00, 0x00};

    while (1) {
        if (!wtd_cnt) {
            write_packet(wdt_msg, sizeof(wdt_msg));
        } else {
            wdt_msg_2[4] = wtd_cnt;
            wdt_msg_2[5] = 0x12 + wtd_cnt;
            write_packet(wdt_msg_2, sizeof(wdt_msg_2));
        }
        wtd_cnt++;
        sleep(10);
    }
}

void process_packet(uint8_t *buff, int sz)
{
    static int rx_p_cnt = 0;

    rx_p_cnt++;

    printf("Read: ");
    int i;
    for(i = 0; i < sz; ++i)
        printf("0x%02x ", buff[i]);
    printf("\n");

    printf("GeT packet(# %d): 0x%02x\n", rx_p_cnt,buff[3]);

    switch (buff[3]) {
        case 0x1:
            printf("Got a link setup request. VMCU Req IDX: %d\n", buff[4]);
            send_link_setup_response(buff[4], buff[12]);
            break;
        case 0x2:
            printf("Got a link setup complete!\n");
            send_wtd();
            break;
    }
}

void put_byte(struct R *r, uint8_t byte)
{
    if (byte) {
        r->rx_buffer[r->curr_idx++] = byte;
        if(r->curr_idx >= sizeof(r->rx_buffer)) {
            printf("rq_q overrun\n");
            r->curr_idx = 0;
        }
    } else {
        if(r->curr_idx) {
            process_packet(r->rx_buffer, r->curr_idx);
            r->curr_idx = 0;
        }
    }
}

void read_uart()
{
    uint8_t local_102c [0x1000];
    fd_set auStack4180;
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    FD_ZERO( &auStack4180 );
    FD_SET( DAT_00120828_uart_fd, &auStack4180 );

    do {
        while( 1 ) {
            send_link_setup_request();
            int iVar3 = select(DAT_00120828_uart_fd + 1, &auStack4180, 0x0, 0x0, &tv);

            if (iVar3 > 0) {
                /* read UART */
                if( FD_ISSET( DAT_00120828_uart_fd, &auStack4180 ) ) {
                    int i;
                    int sVar4 = read(DAT_00120828_uart_fd, local_102c, 0x1000);
                    for(i=0; i < sVar4; ++i) {
                        put_byte(&gR,local_102c[i]);
                    }
                }
            } else if (iVar3 == 0) {
                printf("timeout occur\n");
            } else {
                printf("Error in poll\n");
            }
        }
    } while( 1 );
}

int main() {
    FUN_0010bf5c_IpcUartConfig();
    read_uart();
    return 0;
}
