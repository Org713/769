#include "data_interface_tx.h"
#include <string.h>
enum{IDLE,WAIT_TX_CFM};
enum{NO_REACH,SEND_SUCCESS,NET_BUSY,WAIT_SEND,SENT};

unsigned short wait_cfm_pkt_cnt = 0;//要等待cfm包序号
unsigned short receive_cfm_pkt_cnt = 0;//接收到的cfm包序号

//写队列 2016.12.27 这里与优先级没关系，直接放在队列的后面
int DataQueWt(Data_Interface_Tx* proc,unsigned short svc_type,unsigned short pkt_sn,unsigned short pri,unsigned short da,unsigned short len, unsigned short *data)
{
	unsigned short btm,top,size;
	top = proc->data_que_list.top;
	btm = proc->data_que_list.btm;
	size = proc->data_que_list.size; 
	if(size >= DATA_QUE_MAX_NUM)  //20
	{
		printf("data_que_list() 队列满\n");
		return 0;
	}
	else
	{
		proc->data_que_list.data_que_elmt[top].svc_type = svc_type;
		proc->data_que_list.data_que_elmt[top].pkt_sn = pkt_sn;
		proc->data_que_list.data_que_elmt[top].pri = pri;
		proc->data_que_list.data_que_elmt[top].da = da;
		proc->data_que_list.data_que_elmt[top].len = len;
		memcpy(proc->data_que_list.data_que_elmt[top].data, data, len);

		proc->data_que_list.top++;
		proc->data_que_list.top %= DATA_QUE_MAX_NUM;
		proc->data_que_list.size++;
		return 1;
	}	
}

//读队列
int DataQueRd(Data_Interface_Tx* proc,unsigned short **data,unsigned short *len,unsigned short *da,unsigned short *pri,unsigned short *svc_type, unsigned short *pkt_sn)
{	
	unsigned short btm;
	if (proc->data_que_list.size != 0)
	{
		btm = proc->data_que_list.btm;

		*data	  = proc->data_que_list.data_que_elmt[btm].data;
		*len	  = proc->data_que_list.data_que_elmt[btm].len;
		*da		  = proc->data_que_list.data_que_elmt[btm].da;
		*pri	  = proc->data_que_list.data_que_elmt[btm].pri;
		*svc_type = proc->data_que_list.data_que_elmt[btm].svc_type;
		*pkt_sn	  = proc->data_que_list.data_que_elmt[btm].pkt_sn;
		return 1;
	}
	return 0;
}
//删除队列中的元素  
void DeleteDataQueElmt(Data_Interface_Tx* proc)
{
	unsigned short btm=proc->data_que_list.btm;
	if(proc->data_que_list.size)
	{
		printf("DeleteDataQueElmt() 删除队列中数据包\n");
		memset(proc->data_que_list.data_que_elmt[btm].data, 0, sizeof(proc->data_que_list.data_que_elmt[btm].data));
		//proc->data_que_list.data_que_elmt[btm].data = NULL;
		proc->data_que_list.data_que_elmt[btm].len = 0;
		proc->data_que_list.data_que_elmt[btm].svc_type = 0;
		proc->data_que_list.data_que_elmt[btm].da = 0;
		proc->data_que_list.data_que_elmt[btm].pkt_sn = 0;
		proc->data_que_list.data_que_elmt[btm].pri = 0;

		proc->data_que_list.btm ++;
		proc->data_que_list.btm %= DATA_QUE_MAX_NUM;
		proc->data_que_list.size --;
	}

}

static int SvcDataTxReq(Signal *sig)
{
	Data_Interface_Tx *proc = (Data_Interface_Tx*)sig->dst;
	Svc_Data_Tx_Req_Param *param = (Svc_Data_Tx_Req_Param*)sig->param;

	unsigned short svc_type;
	unsigned short pkt_sn;
	unsigned short pri;
	unsigned short da;
	unsigned short len;
	unsigned short *data;//数据地址
	svc_type = param->svc_type;
	pkt_sn = param->pkt_sn;
	pri = param->pri;
	da = param->da;
	len = param->len;
	printf("Data_Interface_Tx[%d]::SvcDataTxReq()收到数据请求,\n",proc->data_pri_id);
	//这里是不是应该先写队列，再判断IDLE？？？2016.12.27
	if (proc->state == IDLE)
	{
		DataQueWt(proc, svc_type, pkt_sn, pri, da, len, param->data);//写队列

		DataQueRd(proc,&data,&len,&da,&pri,&svc_type,&pkt_sn);//读队列

		proc->pri = pri;//保存要发送的这包几个参数，待cfm时使用
		proc->pkt_sn = pkt_sn;
		proc->data = data;
		proc->da = da;
		proc->len = len;
		proc->svc_type = svc_type;
		//是否是单播,根据目的地址获知
		if ( (MAX_NODE_CNT > da) && (da >= 0) )	//单播
		{
			sig = proc->data_interface_tx_req[da];
			((T_Data_Interface_Tx_Req_Param*)sig->param)->data = proc->data;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->len = proc->len;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->da = proc->da;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->pri = proc->pri;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->svc_type = proc->svc_type;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->pkt_sn = proc->pkt_sn;

			printf("Data_Interface_Tx[%d]::SvcDataTxReq() 单播，发送给目的data_ntx[%d],\n",proc->data_pri_id, proc->da);
			AddSignal(sig);

		}
		else  //不是单播包
		{
			sig = proc->data_interface_tx_req[proc->mib->local_id];
			((T_Data_Interface_Tx_Req_Param*)sig->param)->data = proc->data;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->len = proc->len;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->da = proc->da;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->pri = proc->pri;
			((T_Data_Interface_Tx_Req_Param*)sig->param)->svc_type = proc->svc_type;
			printf("Data_Interface_Tx[%d]::SvcDataTxReq() 非单播，发送给目的data_ntx[%d],\n",proc->data_pri_id, proc->mib->local_id);
			AddSignal(sig);
		}
		SetTimer(&proc->tx_cfm_tmr, 3);//定时时间？？？
		wait_cfm_pkt_cnt++;
		proc->state = WAIT_TX_CFM;
	}
	else
	{
		sig = proc->svc_data_tx_cfm;
		((T_Svc_Data_Tx_Cfm_Param*)sig->param)->cfm_flag = NET_BUSY;
		((T_Svc_Data_Tx_Cfm_Param*)sig->param)->pri = param->pri;
		((T_Svc_Data_Tx_Cfm_Param*)sig->param)->pkt_sn = param->pkt_sn;
		printf("Data_Interface_Tx[%d]::SvcDataTxReq() 回复上层网络忙\n",proc->data_pri_id);
		AddSignal(sig);
	}
	return 0;
}

static int DataInterfaceTxCfm(Signal *sig)
{
	Data_Interface_Tx *proc = (Data_Interface_Tx*)sig->dst;
	Data_Interface_Tx_Cfm_Param *param = (Data_Interface_Tx_Cfm_Param*)sig->param;

	unsigned short svc_type;
	unsigned short pkt_sn;
	unsigned short pri;
	unsigned short da;
	unsigned short len;
	unsigned short *data;//数据
	printf("Data_Interface_Tx[%d]::DataInterfaceTxCfm()收到cfm,\n",proc->data_pri_id);
	if (proc->state == WAIT_TX_CFM)
	{
		receive_cfm_pkt_cnt++;
		if (param->cfm_flag == SEND_SUCCESS)
		{
			//if (wait_cfm_pkt_cnt == receive_cfm_pkt_cnt)   //为等待cfm包序号
			if (param->cfm_pkt_sn == proc->pkt_sn)
			{
				CancelTimer(&proc->tx_cfm_tmr);
				//删除该数据包
				DeleteDataQueElmt(proc);

				sig = proc->svc_data_tx_cfm;
				((T_Svc_Data_Tx_Cfm_Param*)sig->param)->cfm_flag = param->cfm_flag;
				((T_Svc_Data_Tx_Cfm_Param*)sig->param)->pri = proc->pri;
				((T_Svc_Data_Tx_Cfm_Param*)sig->param)->pkt_sn = proc->pkt_sn;
				AddSignal(sig);
				//是否还有未发送数据
				if (proc->data_que_list.size)//还有要发送的数据
				{
					DataQueRd(proc,&data,&len,&da,&pri,&svc_type, &pkt_sn);//读队列

					proc->pri = pri;//保存要发送的这包几个参数，待cfm时使用
					proc->pkt_sn = pkt_sn;
					proc->data = data;
					proc->da = da;
					proc->len = len;
					proc->svc_type = svc_type;
					//是否是单播,根据目的地址获知
					if (MAX_NODE_CNT>da && da>=0)	//单播
					{
						sig = proc->data_interface_tx_req[da];
						((T_Data_Interface_Tx_Req_Param*)sig->param)->data = proc->data;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->len = proc->len;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->da = proc->da;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->pri = proc->pri;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->svc_type = proc->svc_type;
						printf("Data_Interface_Tx[%d]::DataInterfaceTxCfm(),收到cfm，再发送给目的data_ntx[%d],\n",proc->data_pri_id, proc->da);
						AddSignal(sig);

					}
					else  //不是单播包
					{
						sig = proc->data_interface_tx_req[proc->mib->local_id];
						((T_Data_Interface_Tx_Req_Param*)sig->param)->data = proc->data;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->len = proc->len;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->da = proc->da;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->pri = proc->pri;
						((T_Data_Interface_Tx_Req_Param*)sig->param)->svc_type = proc->svc_type;
						AddSignal(sig);
					}
					SetTimer(&proc->tx_cfm_tmr, 3);//定时时间？？？
					receive_cfm_pkt_cnt++;
					//proc->state = WAIT_TX_CFM;
				}
				else  //没有要发送的数据
				{
					proc->state = IDLE;
				}
			}
			else   //不是等待的cfm包
			{

			}
		}
		else if (param->cfm_flag == NO_REACH)
		{

		}
		else if (param->cfm_flag == NET_BUSY)
		{

		}
	}

	return 0;
}



static int TxCfmTmrOut(Timer *tmr)
{
	Data_Interface_Tx *proc = (Data_Interface_Tx*)tmr->dst;
	Signal *sig;
	sig = proc->svc_data_tx_cfm;
	((T_Svc_Data_Tx_Cfm_Param*)sig->param)->cfm_flag = NO_REACH;
	((T_Svc_Data_Tx_Cfm_Param*)sig->param)->pri = proc->pri;
	((T_Svc_Data_Tx_Cfm_Param*)sig->param)->pkt_sn = proc->pkt_sn;
	AddSignal(sig);
	proc->state = IDLE;
	return 0;
}

void DataInterfaceTxInit(Data_Interface_Tx* proc)
{
	proc->state = IDLE;
	proc->da = 0;
	proc->pri = 0;
	proc->pkt_sn = 0;
	proc->data = NULL;
	proc->data_que_list.btm = 0;
	proc->data_que_list.top = 0;
	proc->data_que_list.size = 0;
	memset(proc->data_que_list.data_que_elmt, 0, sizeof(proc->data_que_list.data_que_elmt));
	
}

void DataInterfaceTxSetup(Data_Interface_Tx* proc)
{
	//unsigned short i;
	Timer *tmr;
	proc->svc_data_tx_req.next=0;
	proc->svc_data_tx_req.src=0;
	proc->svc_data_tx_req.dst=proc;
	proc->svc_data_tx_req.func=SvcDataTxReq;
	proc->svc_data_tx_req.pri=SDL_PRI_NORM;
	proc->svc_data_tx_req.param=&proc->svc_data_tx_req_param;
	
	proc->data_interface_tx_cfm.next=0;
	proc->data_interface_tx_cfm.src=0;
	proc->data_interface_tx_cfm.dst=proc;
	proc->data_interface_tx_cfm.func=DataInterfaceTxCfm;
	proc->data_interface_tx_cfm.pri=SDL_PRI_NORM;
	proc->data_interface_tx_cfm.param=&proc->data_interface_tx_cfm_param;
	
	
	tmr=&proc->tx_cfm_tmr;
	tmr->next=0;
	tmr->dst=proc;
	tmr->pri=SDL_PRI_NORM;
	tmr->func=TxCfmTmrOut;
}
