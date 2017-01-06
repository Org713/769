#include "data_rly_tx.h"
enum{IDLE, WAIT_TX_CFM};

//放入到队列中，参数ifLocalNetPkt=1代表本地网络包，=0代表中继网络包
static int WtRlyQue(Data_Rly_Tx* proc, Net_Pkt *net_pkt, unsigned short pri)
{
	unsigned short test;
	unsigned short btm,top,size;
	top = proc->rly_que_list.top;
	btm = proc->rly_que_list.btm;
	size = proc->rly_que_list.size; 
	if(size >= RLY_QUE_MAX_NUM)  //20
	{
		printf("rly_que_list() 队列满\n");
		return 0;
	}
	else
	{
		proc->rly_que_list.que_elmt[top].pri = pri;
		proc->rly_que_list.que_elmt[top].TTL = RLY_QUE_MAX_TTL;
		memcpy(&(proc->rly_que_list.que_elmt[top].net_pkt), net_pkt, sizeof(Net_Pkt));//2017.1.5

		proc->rly_que_list.top++;
		proc->rly_que_list.top %= RLY_QUE_MAX_NUM;
		proc->rly_que_list.size++;
		return 1;
	}	
}

//读队列
int RdRlyQue(Data_Rly_Tx* proc,unsigned short **net_pkt,unsigned short *da, unsigned short *len, unsigned short *pri)
{	
	unsigned short btm;
	if (proc->rly_que_list.size != 0)
	{
		btm = proc->rly_que_list.btm;
		*net_pkt		= (unsigned short*)&proc->rly_que_list.que_elmt[btm].net_pkt;
		*da			= proc->rly_que_list.que_elmt[btm].net_pkt.da;
		//*len		= proc->rly_que_list.que_elmt[btm].net_pkt.len;
		*len		= sizeof(proc->rly_que_list.que_elmt[btm].net_pkt);//包的长度，不是包中数据的长度
		*pri		= proc->rly_que_list.que_elmt[btm].net_pkt.pri;
		return 1;
	}
	return 0;
}

//删除队列中的网络包
void DeleteRlyQue(Data_Rly_Tx* proc)
{
	unsigned short btm=proc->rly_que_list.btm;
	if(proc->rly_que_list.size)
	{
		printf("DeleteRlyQue() 删除队列中网络包\n");

		proc->rly_que_list.que_elmt[btm].pri = 0;
		proc->rly_que_list.que_elmt[btm].TTL = 0;

		proc->rly_que_list.que_elmt[btm].net_pkt.svc_type = 0;
		proc->rly_que_list.que_elmt[btm].net_pkt.pkt_sn = 0;
		proc->rly_que_list.que_elmt[btm].net_pkt.da = 0;
		proc->rly_que_list.que_elmt[btm].net_pkt.sa = 0;
		proc->rly_que_list.que_elmt[btm].net_pkt.pri = 0;
		proc->rly_que_list.que_elmt[btm].net_pkt.len = 0;
		memset(proc->rly_que_list.que_elmt[btm].net_pkt.net_payload, 0, NET_PAYLOAD_SIZE_TEST);

		proc->rly_que_list.btm ++;
		proc->rly_que_list.btm %= RLY_QUE_MAX_NUM;
		proc->rly_que_list.size --;
	}

}


static int DataRlyTxCfm(Signal* sig)
{
	Data_Rly_Tx *proc = (Data_Rly_Tx*)sig->dst;
	Data_Rly_Tx_Cfm_Param *param = (Data_Rly_Tx_Cfm_Param*)sig->param;
	
	unsigned short	isSucc_flag;
	unsigned short 	da;
	unsigned short  pri;
	unsigned short	*temp_net_pkt;//取网络包时暂时存放
	unsigned short	len;
	unsigned short	ttl;
	unsigned short  btm;

	if (proc->state == WAIT_TX_CFM)
	{
		if (param->cfm_flag == SUCC)
		{
			DeleteRlyQue(proc);//删除该包
			while (proc->rly_que_list.size != 0)	//还有未发送包
			{
				btm = proc->rly_que_list.btm;
				ttl = proc->rly_que_list.que_elmt[btm].TTL;
				if (ttl == 0) //网络包ttl超期
				{
					DeleteRlyQue(proc);
				}
				else  //没有超期
				{
					isSucc_flag = RdRlyQue(proc, &temp_net_pkt, &da, &len, &pri);
					if (isSucc_flag)
					{
						printf("取中继队列\n");
					}
					sig = proc->data_rly_tx_req[pri][da];
					((T_Data_Rly_Tx_Req_Param*)sig->param)->net_pkt = temp_net_pkt;
					((T_Data_Rly_Tx_Req_Param*)sig->param)->da = da;
					((T_Data_Rly_Tx_Req_Param*)sig->param)->len = len;
					((T_Data_Rly_Tx_Req_Param*)sig->param)->pri = pri;
					AddSignal(sig);

					//是否小于最低存储空间
					if (proc->rly_que_list.size < MIN_STORE_SPACE)//小于最低存储空间
					{
						if (!proc->jam_flag)//给过拥塞解除指示
						{
							return 0;
						}
						else  //没有给过拥塞解除指示
						{
							sig = proc->data_rly_tx_jam_ind;
							((T_Data_Rly_Tx_Jam_Ind_Param*)sig->param)->jam_flag = 0;
							AddSignal(sig);
							proc->jam_flag = 0;//拥塞解除
							return 0;
						}
					}
					else
					{
						return 0;//不小于最低存储空间
					}
				}
			}
			proc->state = IDLE;//没有网络包发送
			return 0;
		}
		else    //上一包 发送失败了
		{
			//中继网络包发送失败，在网络层不再重新发送
		}
	}
	return 0;
}

static int DataRlyRxInd(Signal *sig)
{
	Data_Rly_Tx *proc = (Data_Rly_Tx*)sig->dst;
	Data_Rly_Rx_Ind_Param *param = (Data_Rly_Rx_Ind_Param*)sig->param;

	unsigned short	isSucc_flag;
	unsigned short 	da;
	unsigned short  pri;
	unsigned short	*temp_net_pkt;//取网络包时暂时存放
	unsigned short	len;//包的长度

	if (proc->state == IDLE)
	{
		//放到中继队列中，并给该包设置ttl值；
		isSucc_flag = WtRlyQue(proc, (Net_Pkt*)param->net_pkt, param->pri);
		if (isSucc_flag)
		{
			printf("放到中继队列中\n");
		}
		//读取队列
		isSucc_flag = RdRlyQue(proc, &temp_net_pkt, &da, &len, &pri);
		if (isSucc_flag)
		{
			printf("取中继队列\n");
		}
		if (MAX_NODE_CNT < da) //广播or组播
		{
			da = proc->mib->local_id;	//2017.1.6
			sig = proc->data_rly_tx_req[pri][da];
		}
		else  //单播
		{
			sig = proc->data_rly_tx_req[pri][da];
		}
		((T_Data_Rly_Tx_Req_Param*)sig->param)->net_pkt = temp_net_pkt;
		((T_Data_Rly_Tx_Req_Param*)sig->param)->da = da;
		((T_Data_Rly_Tx_Req_Param*)sig->param)->len = len;
		((T_Data_Rly_Tx_Req_Param*)sig->param)->pri = pri;
		AddSignal(sig);
		proc->state = WAIT_TX_CFM;
	}
	else
	{
		//放到中继队列中，并给该包设置ttl值；
		isSucc_flag = WtRlyQue(proc, (Net_Pkt*)param->net_pkt, param->pri);
		if (isSucc_flag)
		{
			printf("放到中继队列中\n");
		}
		//size是否大于最大存储空间个数
		if (proc->rly_que_list.size > MAX_STORE_SPACE) //size > 最大存储空间个数
		{
			if (proc->jam_flag)  //给过拥塞指示
			{
				return 0;
			}
			else  //没有给过拥塞指示
			{
				sig = proc->data_rly_tx_jam_ind;
				((T_Data_Rly_Tx_Jam_Ind_Param*)sig->param)->jam_flag = 1;
				AddSignal(sig);
				proc->jam_flag = 1;
				return 0;
			}
		}
		else  
		{
			return 0;
		}
	}
	return 0;
}


void DataRlyTxInit(Data_Rly_Tx* proc)
{
	proc->state = IDLE;
	proc->jam_flag = 0xff;
}
void DataRlyTxSetup(Data_Rly_Tx* proc)
{
	unsigned short i,j;
	Signal* sig;
	for (i = 0; i < DATA_PRI_NUM; i++)
	{
		for (j = 0; j < MAX_NODE_CNT; j++)
		{
			sig = &proc->data_rly_tx_cfm[i][j];
			sig->next=0;
			sig->src=0;
			sig->dst=proc;
			sig->func=DataRlyTxCfm;
			sig->pri=SDL_PRI_NORM;
			sig->param = &proc->data_rly_tx_cfm_param[i][j];
		}
	}

	sig = &proc->data_rly_rx_ind;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataRlyRxInd;
	sig->pri=SDL_PRI_NORM;
	sig->param = &proc->data_rly_rx_ind_param;
}
	