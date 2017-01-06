#include "data_nrx.h"

enum{IDLE};

static int DataNtxToDataNrxTest(Signal *sig)
{
	Data_Nrx *proc = (Data_Nrx*)sig->dst;
	Data_Ntx_To_Data_Nrx_Test_Param *param = (Data_Ntx_To_Data_Nrx_Test_Param*)sig->param;

	sig = proc->data_interface_rx_ind;
	((T_Data_Interface_Rx_Ind_Param*)sig->param)->data = param->data;
	((T_Data_Interface_Rx_Ind_Param*)sig->param)->len = param->len;
	((T_Data_Interface_Rx_Ind_Param*)sig->param)->sa = param->sa;
	((T_Data_Interface_Rx_Ind_Param*)sig->param)->pri = param->pri;
	((T_Data_Interface_Rx_Ind_Param*)sig->param)->svc_type = param->svc_type;
	AddSignal(sig);
	printf("Data_Nrx[%d]::DataNtxToDataNrxTest()收到数据，发送data_interface_rx_ind。\n",proc->module_id);

	return 0;
}

static int DataNrxInd(Signal *sig)
{
	Data_Nrx *proc = (Data_Nrx*)sig->dst;
	Data_Nrx_Ind_Param *param = (Data_Nrx_Ind_Param*)sig->param;

	unsigned short da,receive_pkt_sn;
	da = ((Net_Pkt*)param->net_pkt)->da;//取出da
	receive_pkt_sn = ((Net_Pkt*)param->net_pkt)->pkt_sn;
	if (proc->state == IDLE)
	{
		if (da == proc->mib->local_id)  //本地单播包
		{
			if (proc->unicast_pkt_sn != receive_pkt_sn)//没收到过
			{
				sig = proc->data_interface_rx_ind;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->data = ((Net_Pkt*)param->net_pkt)->net_payload;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->len = ((Net_Pkt*)param->net_pkt)->len;//数据长度,不是包长度
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->pri = ((Net_Pkt*)param->net_pkt)->pri;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->sa = ((Net_Pkt*)param->net_pkt)->sa;//param->sa??
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->svc_type = ((Net_Pkt*)param->net_pkt)->svc_type;
				AddSignal(sig);
			}
			else  //收到过
			{
				return 0;
			}
		}
		else if ( (da!=proc->mib->local_id) && (da>=0) && (da<MAX_NODE_CNT) )   //其他目的节点单播包
		{
			if (proc->rly_unicast_pkt_sn != receive_pkt_sn)//没收到过
			{
				sig = proc->data_rly_rx_ind;
				((T_Data_Rly_Rx_Ind_Param*)sig->param)->net_pkt = param->net_pkt;
				((T_Data_Rly_Rx_Ind_Param*)sig->param)->len = param->len;//包的长度
				((T_Data_Rly_Rx_Ind_Param*)sig->param)->pri = ((Net_Pkt*)param->net_pkt)->pri;
				AddSignal(sig);
			}
			else  //收到过
			{
				return 0;
			}
		}
		else if (da == 0xFF) //广播包
		{
			if (proc->broadcast_pkt_sn != receive_pkt_sn)//没收到过
			{
				sig = proc->data_rly_rx_ind;//本地没有收到过的广播包也要中继的
				((T_Data_Rly_Rx_Ind_Param*)sig->param)->net_pkt = param->net_pkt;
				((T_Data_Rly_Rx_Ind_Param*)sig->param)->len = param->len;//包的长度
				((T_Data_Rly_Rx_Ind_Param*)sig->param)->pri = ((Net_Pkt*)param->net_pkt)->pri;
				AddSignal(sig);

				sig = proc->data_interface_rx_ind;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->data = ((Net_Pkt*)param->net_pkt)->net_payload;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->len = ((Net_Pkt*)param->net_pkt)->len;//数据长度,不是包长度
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->pri = ((Net_Pkt*)param->net_pkt)->pri;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->sa = ((Net_Pkt*)param->net_pkt)->sa;//param->sa??
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->svc_type = ((Net_Pkt*)param->net_pkt)->svc_type;
				AddSignal(sig);
			}
			else  //收到过
			{
				return 0;
			}
		}
		else if (da == 0xFE) //组播包
		{
			if (proc->multicast_pkt_sn != receive_pkt_sn)//没收到过
			{
				sig = proc->data_interface_rx_ind;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->data = ((Net_Pkt*)param->net_pkt)->net_payload;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->len = ((Net_Pkt*)param->net_pkt)->len;//数据长度,不是包长度
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->pri = ((Net_Pkt*)param->net_pkt)->pri;
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->sa = ((Net_Pkt*)param->net_pkt)->sa;//param->sa??
				((T_Data_Interface_Rx_Ind_Param*)sig->param)->svc_type = ((Net_Pkt*)param->net_pkt)->svc_type;
				AddSignal(sig);
			}
			else  //收到过
			{
				return 0;
			}
		}
	}
	return 0;
}


void DataNrxInit(Data_Nrx* proc)
{
	proc->state = IDLE;
	proc->unicast_pkt_sn = 0xff;		//单播  unicast
	proc->rly_unicast_pkt_sn = 0xff;	//其他目的节点单播 rly_unicast
	proc->broadcast_pkt_sn = 0xff;	//广播  broadcast
	proc->multicast_pkt_sn = 0xff;	//组播  multicast
}

void DataNrxSetup(Data_Nrx* proc)
{
	Signal *sig;
	//unsigned short i;	
	sig = &proc->data_ntx_to_data_nrx_test;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataNtxToDataNrxTest;
	sig->pri=SDL_PRI_NORM;
	sig->param=&proc->data_ntx_to_data_nrx_test_param;

	sig = &proc->data_nrx_ind;
	sig->next = 0;
	sig->src = 0;
	sig->dst = proc;
	sig->func = DataNrxInd;
	sig->pri = SDL_PRI_NORM;
	sig->param = &proc->data_nrx_ind_param;
}