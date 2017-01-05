#include "data_dlctxpump.h"
enum{IDLE};

static int DlcTxpumpTxReq(Signal* sig)
{
	Data_Dlctxpump *proc = (Data_Dlctxpump*)sig->dst;
	Dlc_Txpump_Tx_Req_Param *param = (Dlc_Txpump_Tx_Req_Param*)sig->param;

	unsigned short data_pri_id,module_id,cfm_nsn;
	/***************************test begin************************/
	unsigned short test_svc_type;//data_type
	unsigned short test_pkt_sn;
	unsigned short test_da;
	unsigned short test_sa;
	unsigned short test_pri;
	unsigned short test_len;
	unsigned short test_net_payload[NET_PAYLOAD_SIZE_TEST];//¾»ºÉ  750

	memset(test_net_payload, 0, NET_PAYLOAD_SIZE_TEST);
	test_svc_type = ((Net_Pkt*)param->net_pkt)->svc_type;
	test_pkt_sn = ((Net_Pkt*)param->net_pkt)->pkt_sn;
	test_da = ((Net_Pkt*)param->net_pkt)->da;
	test_sa = ((Net_Pkt*)param->net_pkt)->sa;
	test_pri = ((Net_Pkt*)param->net_pkt)->pri;
	test_len = ((Net_Pkt*)param->net_pkt)->len;
	memcpy(test_net_payload, ((Net_Pkt*)param->net_pkt)->net_payload, test_len);

	/***************************test end************************/


	data_pri_id = param->data_pri_id;
	module_id = param->module_id;

	cfm_nsn = ((Net_Pkt*)param->net_pkt)->pkt_sn;

	sig = proc->dlc_txpump_tx_cfm[data_pri_id][module_id];
	((T_Dlc_Txpump_Tx_Cfm_Param*)sig->param)->cfm_flag = SUCC;
	((T_Dlc_Txpump_Tx_Cfm_Param*)sig->param)->nsn = cfm_nsn;
	AddSignal(sig);

	return 0;
}


void DataDlcTxPumpInit(Data_Dlctxpump* proc)
{
	proc->state = IDLE;
}

void DataDlcTxPumpSetup(Data_Dlctxpump* proc)
{
	unsigned short i,j;
	Signal* sig;
	for (i = 0; i < DATA_PRI_NUM; i++)
	{
		for (j = 0; j < MAX_NODE_CNT; j++)
		{
			sig = &proc->dlc_txpump_tx_req[i][j];
			sig->next=0;
			sig->src=0;
			sig->dst=proc;
			sig->func=DlcTxpumpTxReq;
			sig->pri=SDL_PRI_NORM;
			sig->param = &proc->dlc_txpump_tx_req_param[i][j];
		}
	}
}
