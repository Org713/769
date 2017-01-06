#include "data_dlc_rxfilter.h"
enum {IDLE};

unsigned short test_data[]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
//测试
static int TestInd(Signal *sig)
{
	Data_Dlc_RxFilter *proc = (Data_Dlc_RxFilter*)sig->dst;

	proc->test_net_pkt.da = 0xff;
	proc->test_net_pkt.len = sizeof(test_data);//数据的长度
	proc->test_net_pkt.pkt_sn = 2;
	proc->test_net_pkt.pri = 1;
	proc->test_net_pkt.sa = 1;
	proc->test_net_pkt.svc_type = 1;
	memcpy(proc->test_net_pkt.net_payload, test_data, sizeof(test_data));

	sig = proc->data_nrx_ind[proc->test_net_pkt.pri][proc->test_net_pkt.sa];
	((T_Data_Nrx_Ind_Param*)sig->param)->net_pkt = (unsigned short*)&proc->test_net_pkt;
	((T_Data_Nrx_Ind_Param*)sig->param)->len = sizeof(Net_Pkt);//网络包的长度
	((T_Data_Nrx_Ind_Param*)sig->param)->sa = proc->test_net_pkt.sa;
	AddSignal(sig);
	

	return 0;
}


void DataDlcRxFilterInit(Data_Dlc_RxFilter* proc)
{
	proc->state = IDLE;
}

void DataDlcRxFilterSetup(Data_Dlc_RxFilter* proc)
{
	Signal *sig;
	sig = &proc->test_ind;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=TestInd;
	sig->pri=SDL_PRI_NORM;
	sig->param=0;
}
