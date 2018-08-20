/*
    Schematic VMCU <--> IPC(omap_cpu) handshake protocol description

                 VMCU                                 IPC
                                 <--- (0x01)   IPC_LinkSetupRequest
        VMCU_LinkSetupRequest    ---> (0x01)
                                 <--- (0x81)   IPC_LinkSetupResponse
        VMCU_LinkSetupResponse   ---> (0x81)
                                 <--- (0x02)   IPC_LinkSetupComplete
        VMCU_LinkSetupComplete   ---> (0x02)

                                 <---          IPC_WdtPacket (g_wdt_cnt = 0)
                                 <---          IPC_WdtPacket (g_wdt_cnt = 1)
                                 ...
                                 <---          IPC_WdtPacket (g_wdt_cnt = N)
*/


static uint8_t g_send_cnt = 0;
static uint8_t g_wdt_cnt = 0;

enum CTRL_MSG_ID: uint8_t {
	LINK_SETUP 		= 0x01,
	LINK_SETUP_COMPLETE	= 0x02,
	LINK_TEAR_DOWN		= 0x03,
	LINK_SETUP_RESPONSE	= 0x81,
	LINK_TEAR_DOWN_COMPLETE	= 0x83,
}

// IPC Link Setup Request (VMCU <--- IPC) (0x1)
// Related strings: SendLinkSetup, Sent a Link Setup Request
struct IPC_LinkSetupRequest {
	uint8_t b_0			= 1;
	uint8_t b_1 			= 0;
	uint8_t b_2_id			= CTRL_MSG_ID::LINK_SETUP;
	uint8_t b_3_sent_cnt		= g_send_cnt++;
	uint8_t b_4			= 1;
	uint8_t b_5			= 0;
	uint32_t ipc_cfg_sign		= FUN_00107a78_GetIpcCfgSignature();
	uint8_t b_10			= 0;
	uint8_t b_11			= 0;
	uint8_t b_12			= 0;
	uint8_t b_13			= 0;
}


// VMCU Link Setup Request (VMCU ---> IPC) (0x1)
// Related strings: Got a Link Setup Request
struct VMCU_LinkSetupRequest {
	uint8_t b_0;
	uint8_t b_1;
	uint8_t b_2_id			= CTRL_MSG_ID::LINK_SETUP;
	uint8_t b_4_recv_counter;
	uint16_t incomingTB;
	uint32_t incomingSignature;
}


// IPC Link Setup Response (VMCU <--- IPC) (0x81)
// Related strings: SendLinkSetupResponse, Sent a Link Setup Response
struct IPC_LinkSetupResponse {
	uint8_t b_0			= 1;
	uint8_t b_1			= 0;
	uint8_t b_2_id 			= CTRL_MSG_ID::LINK_SETUP_RESPONSE;
	uint8_t b_3_recv_counter	= VMCU_LinkSetupRequest::b_4_recv_counter;
	uint8_t b_4;
	uint8_t b_5;
	uint8_t b_6;
	uint8_t b_7;
	uint8_t b_8;
	uint8_t b_9;
	//additional zeroes?}

// VMCU Link Setup Response (VMCU ---> IPC) (0x81)
// Related strings: Got a Link Setup Response
struct VMCU_LinkSetupResponse {
	uint8_t b_0;
	uint8_t b_1;
	uint8_t b_2_id = CTRL_MSG_ID::LINK_SETUP_RESPONSE;
	uint8_t b_4;
	uint16_t u16_5;
}

// IPC Link Setup Complete (VMCU <--- IPC) (0x2)
// Related strings: SendLinkSetupComplete, Sent a Link Setup Complete
struct IPC_LinkSetupComplete {
	uint8_t b_0 		= 1;
	uint8_t b_1 		= 0;
	uint8_t b_2_id 		= CTRL_MSG_ID::LINK_SETUP_COMPLETE;
	uint8_t b_3_sent_cnt 	= g_send_cnt++;
	uint8_t b_4		= 0;
	uint8_t b_5		= 0;
	uint8_t b_6		= 0;
	uint8_t b_7		= 0;
	uint8_t b_8		= 0;
	uint8_t b_9		= 0;
}

// VMCU Link Setup Complete (VMCU ---> IPC) (0x2)
// Related strings: Got a Link Setup Complete
struct VMCU_LinkSetupComplete {
	uint8_t b_0;
	uint8_t b_1;
	uint8_t b_2_id = VMCU_resp_type::LINK_SETUP_COMPLETE;
	uint8_t b_4;
	uint16_t u16_5;
}

// IPC Watchdog reset (VMCU <--- IPC)
struct IPC_WdtPacket {
	uint8_t b_0 = 0x11;
	uint8_t b_1 = 0;
	uint8_t b_2 = 1;
	uint8_t b_3 = g_wdt_cnt++;
}
