#include "interrupt_handler.h"

enum{IDLE,WAIT_TX_CFM};
enum{NO_REACH,SEND_SUCCESS,NET_BUSY,WAIT_SEND,SENT};

//测试
unsigned short test_data_0[]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
unsigned short test_data_1[]={101,102,103,104,105,106,107,108,109,110,111,112,113,114,115};
//测试 每次发送的 优先级、目的地址、包序号不同
unsigned short test_pri = 0;
unsigned short test_pkt_sn = 1;
unsigned short test_svc_type = 1;
unsigned short test_da = 1;

static int TestInd(Signal *sig)
{
	Interrupt_Handler *proc = (Interrupt_Handler*)sig->dst;
	Test_Ind_Param *param = (Test_Ind_Param*)sig->param;
	u16 pkt_num;

	test_pri = 1;
	test_da = 1;
	if (proc->state == IDLE)
	{
		printf("interrupt_handler：收到测试指示\n");
		sig = proc->svc_data_tx_req[test_pri];
		((T_Svc_Data_Tx_Req_Param*)sig->param)->svc_type = test_svc_type;
		((T_Svc_Data_Tx_Req_Param*)sig->param)->pkt_sn = test_pkt_sn;
		((T_Svc_Data_Tx_Req_Param*)sig->param)->pri = test_pri;
		((T_Svc_Data_Tx_Req_Param*)sig->param)->da = test_da;
		((T_Svc_Data_Tx_Req_Param*)sig->param)->len = sizeof(test_data_0);//以字节为单位
		((T_Svc_Data_Tx_Req_Param*)sig->param)->data = test_data_0;
		AddSignal(sig);
		proc->state = WAIT_TX_CFM;

		//根据da 发送数据统计  
		proc->send_data_statistics[test_da].pkt_num++;
		pkt_num = proc->send_data_statistics[test_da].pkt_num;
		proc->send_data_statistics[test_da].len[pkt_num] = sizeof(test_data_0);
		proc->send_data_statistics[test_da].pri[pkt_num] = test_pri;


	}
	
	return 0;
}

static int SvcDataTxCfm(Signal *sig)
{
	Interrupt_Handler *proc = (Interrupt_Handler*)sig->dst;
	Svc_Data_Tx_Cfm_Param *param = (Svc_Data_Tx_Cfm_Param*)sig->param;

	if (proc->state == WAIT_TX_CFM)
	{
		printf("Interrupt_Handler::SvcDataTxCfm()收到cfm\n");
		if (param->cfm_flag == SEND_SUCCESS)
		{
			test_pkt_sn++;//测试 每次发送的 优先级、目的地址、包序号不同
			test_pri++;
			test_pri %= DATA_PRI_NUM;
			test_da++;
			test_da %= MAX_NODE_CNT;
			sig = proc->svc_data_tx_req[test_pri];
			((T_Svc_Data_Tx_Req_Param*)sig->param)->svc_type = test_svc_type;
			((T_Svc_Data_Tx_Req_Param*)sig->param)->pkt_sn = test_pkt_sn;
			((T_Svc_Data_Tx_Req_Param*)sig->param)->pri = test_pri;
			((T_Svc_Data_Tx_Req_Param*)sig->param)->da = test_da;
			((T_Svc_Data_Tx_Req_Param*)sig->param)->len = sizeof(test_data_0);
			((T_Svc_Data_Tx_Req_Param*)sig->param)->data = test_data_0;
			AddSignal(sig);
			printf("Interrupt_Handler::SvcDataTxCfm(),cfm=send_sucess,继续发送\n");
			proc->state = WAIT_TX_CFM;
		}
		else
		{
			
		}
	}
	return 0;
}
//2016.12.28
static int SvcDataRxInd(Signal *sig)
{
	Interrupt_Handler *proc = (Interrupt_Handler*)sig->dst;
	Svc_Data_Rx_Ind_Param *param = (Svc_Data_Rx_Ind_Param*)sig->param;
	u16 pri,pkt_num,sa;
	u16 i = 0;
	//收到的数据按照优先级存放在buffer中
	pri = param->pri;
	proc->receive_data_pri[pri].pkt_num++;//包数
	pkt_num = proc->receive_data_pri[pri].pkt_num;
	proc->receive_data_pri[pri].buffer[pkt_num].svc_type = param->svc_type;//此包业务类型
	memcpy(proc->receive_data_pri[pri].buffer[pkt_num].data, param->data, param->len);//此包数据 拷贝

	printf("Interrupt_Handler::SvcDataRxInd()收到数据，按照优先级放进buffer\n");

	//接收数据统计
	sa = param->sa;//源节点
	proc->receive_data_statistics[sa].pkt_num++;//包数
	pkt_num = proc->receive_data_statistics[sa].pkt_num;
	proc->receive_data_statistics[sa].len[pkt_num] = param->len;//此包长度
	proc->receive_data_statistics[sa].pri[pkt_num] = param->pri;//此包优先级
	printf("Interrupt_Handler::SvcDataRxInd()收到数据，根据da统计\n");

	//打印收到数据2016.12.30
	printf("收到的数据为: ");
	for ( i = 0; i < ((param->len)/sizeof(unsigned short)); i++)
	{
		printf("%u,",*(param->data++));
	}
	printf("\n");
	return 0;
}
void InterruptHandlerInit(Interrupt_Handler* proc)
{
	proc->state = IDLE;
}

void InterruptHandlerSetup(Interrupt_Handler* proc)
{
	unsigned short i;
	proc->test_ind.next=0;
	proc->test_ind.src=0;
	proc->test_ind.dst=proc;
	proc->test_ind.func=TestInd;
	proc->test_ind.pri=SDL_PRI_NORM;
	proc->test_ind.param=&proc->test_ind_param;

	for ( i = 0; i < DATA_PRI_NUM; i++)
	{
		proc->svc_data_tx_cfm[i].next=0;
		proc->svc_data_tx_cfm[i].src=0;
		proc->svc_data_tx_cfm[i].dst=proc;
		proc->svc_data_tx_cfm[i].func=SvcDataTxCfm;
		proc->svc_data_tx_cfm[i].pri=SDL_PRI_NORM;
		proc->svc_data_tx_cfm[i].param=&proc->svc_data_tx_cfm_param[i];

		proc->svc_data_rx_ind[i].next=0;
		proc->svc_data_rx_ind[i].src=0;
		proc->svc_data_rx_ind[i].dst=proc;
		proc->svc_data_rx_ind[i].func=SvcDataRxInd;
		proc->svc_data_rx_ind[i].pri=SDL_PRI_NORM;
		proc->svc_data_rx_ind[i].param=&proc->svc_data_rx_ind_param[i];
	}


}