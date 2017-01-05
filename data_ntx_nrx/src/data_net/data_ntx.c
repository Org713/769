#include "data_ntx.h"

enum{IDLE,WAIT_TX_CFM,WAIT_RT_CFM,WAIT_RT_FIND_CFM};
enum{NO_REACH,SEND_SUCCESS,NET_BUSY,WAIT_SEND,SENT};

//组网络包，并把网络包写到指定的地址中  
static void WtNetPkt(Data_Ntx* proc, Net_Pkt* net_pkt, Data_Interface_Tx_Req_Param* param)
{
	//unsigned short test;
	//memset(des_data,0,(param->len + NET_FRM_HEAD_LEN));  //yun:网络包空间 清空
	//test = sizeof(net_pkt);
	memset(net_pkt, 0, sizeof(Net_Pkt));  // 网络包净荷

	//填网络帧头,除了序号（序号在发送的时候才会填写）
	//((Net_Frm*)des_data)->pri_sa_da_mul_sn = (proc->mib->local_id) << 8;  //yun:源地址
	//((Net_Frm*)des_data)->pri_sa_da_mul_sn |= (da & 0x1f)<<3;  //yun:目的地址
	//((Net_Frm*)des_data)->pri_sa_da_mul_sn |= pri<<13;  //yun:优先级
	//((Net_Frm*)des_data)->sn_len |= (len &0x3ff);  //yun:长度 ，没序号

	net_pkt->svc_type = param->svc_type;
	//net_pkt->pkt_sn = proc->pkt_sn;//本地分配包序号
	net_pkt->pkt_sn = param->pkt_sn;//上层data_interface_tx发来
	net_pkt->da = param->da;
	net_pkt->sa = proc->mib->local_id;
	net_pkt->pri = param->pri;
	net_pkt->len = param->len;

	memcpy(net_pkt->net_payload, param->data, param	->len);  //没有网络包头
	//proc->pkt_sn++;
	return;
}

//放入到队列中，参数ifLocalNetPkt=1代表本地网络包，=0代表中继网络包
static int WtNetQue(Data_Ntx* proc, Net_Pkt *net_pkt, unsigned short ifLocalNetPkt)
{
	unsigned short test;
	unsigned short btm,top,size;
	top = proc->net_que_list.top;
	btm = proc->net_que_list.btm;
	size = proc->net_que_list.size; 
	if(size >= NET_QUE_MAX_NUM)  //20
	{
		printf("net_que_list() 队列满\n");
		return 0;
	}
	else
	{
		if (ifLocalNetPkt)  //本地网络包
		{
			proc->net_que_list.que_elmt[top].pkt_type = LOCAL_NET_PKT;
			proc->net_que_list.que_elmt[top].TTL = NET_PKT_MAX_TTL;
		}
		else
		{
			proc->net_que_list.que_elmt[top].pkt_type = RLY_NET_PKT;
			proc->net_que_list.que_elmt[top].TTL = NET_PKT_MAX_TTL;
		}
		test = sizeof(Net_Pkt);
		memcpy(&(proc->net_que_list.que_elmt[top].net_pkt), net_pkt, sizeof(Net_Pkt));//这里还没确定网络包及包头长度

		proc->net_que_list.top++;
		proc->net_que_list.top %= NET_QUE_MAX_NUM;
		proc->net_que_list.size++;
		return 1;
	}	
}

//读队列
int RdNetQue(Data_Ntx* proc,unsigned short **net_pkt,unsigned short *da, unsigned short *len, unsigned short *pri, unsigned short *pkt_type)
{	
	unsigned short btm;
	if (proc->net_que_list.size != 0)
	{
		btm = proc->net_que_list.btm;
		*net_pkt		= (unsigned short*)&proc->net_que_list.que_elmt[btm].net_pkt;
		*da			= proc->net_que_list.que_elmt[btm].net_pkt.da;
		*len		= proc->net_que_list.que_elmt[btm].net_pkt.len;
		*pri		= proc->net_que_list.que_elmt[btm].net_pkt.pri;
		*pkt_type	= proc->net_que_list.que_elmt[btm].pkt_type;
		return 1;
	}
	return 0;
}

//删除队列中的网络包
void DeleteNetPkt(Data_Ntx* proc)
{
	unsigned short btm=proc->net_que_list.btm;
	if(proc->net_que_list.size)
	{
		printf("DeleteNetPkt() 删除队列中网络包\n");

		proc->net_que_list.que_elmt[btm].pkt_type = 0;
		proc->net_que_list.que_elmt[btm].TTL = 0;

		proc->net_que_list.que_elmt[btm].net_pkt.svc_type = 0;
		proc->net_que_list.que_elmt[btm].net_pkt.pkt_sn = 0;
		proc->net_que_list.que_elmt[btm].net_pkt.da = 0;
		proc->net_que_list.que_elmt[btm].net_pkt.sa = 0;
		proc->net_que_list.que_elmt[btm].net_pkt.pri = 0;
		proc->net_que_list.que_elmt[btm].net_pkt.len = 0;
		memset(proc->net_que_list.que_elmt[btm].net_pkt.net_payload, 0, sizeof(proc->net_que_list.que_elmt[btm].net_pkt.net_payload));
		
		proc->net_que_list.btm ++;
		proc->net_que_list.btm %= NET_QUE_MAX_NUM;
		proc->net_que_list.size --;
	}

}

static int DataInterfaceTxReq(Signal *sig)
{
	Data_Ntx *proc = (Data_Ntx*)sig->dst;
	Data_Interface_Tx_Req_Param *param = (Data_Interface_Tx_Req_Param*)sig->param;

	//unsigned short svc_type;
	unsigned short isSucc_flag;
	unsigned short pri;
	unsigned short da;
	unsigned short len;
	Net_Pkt net_pkt;//组网络包
	unsigned short *temp_net_pkt;//取网络包时暂时存放
	//unsigned short pkt_sn;//上层的包 序号 2016.12.30
	unsigned short pkt_type;
	printf("Data_Ntx[%d]::DataInterfaceTxReq()收到req,\n",proc->module_id);

	//svc_type = param->svc_type;
	//pri = param->pri;
	//da = param->da;
	//len = param->len;
	//memcpy(proc->rcv_data, param->data, param->len);//保存要发送的数据 2016.12.28

	//sig = proc->data_interface_tx_cfm;
	//((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_flag = SEND_SUCCESS;
	//((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_pkt_sn = param->pkt_sn;
	//AddSignal(sig);

	////测试，发和收连接
	//sig = proc->data_ntx_to_data_nrx_test;
	//((T_Data_Ntx_To_Data_Nrx_Test_Param*)sig->param)->data = proc->rcv_data;//2016.12.28
	//((T_Data_Ntx_To_Data_Nrx_Test_Param*)sig->param)->len = param->len;
	//((T_Data_Ntx_To_Data_Nrx_Test_Param*)sig->param)->da = param->da;
	//((T_Data_Ntx_To_Data_Nrx_Test_Param*)sig->param)->pri = param->pri;
	//((T_Data_Ntx_To_Data_Nrx_Test_Param*)sig->param)->svc_type = param->svc_type;
	//((T_Data_Ntx_To_Data_Nrx_Test_Param*)sig->param)->sa = proc->mib->local_id;
	//AddSignal(sig);
	//printf("Data_Ntx[%d]::DataInterfaceTxReq()，发-->收测试\n",proc->module_id);
	

	if (proc->state != IDLE)
	{
		//组网络包，
		WtNetPkt(proc, &net_pkt, param);

		//放到待发送队列中，标记为本地网络包，并设置ttl值
		isSucc_flag = WtNetQue(proc, &net_pkt, 1);

		if (isSucc_flag)
		{
			printf("已经放进队列中\n");
		}
	}
	else if (proc->state == IDLE)
	{
		//组网络包
		WtNetPkt(proc, &net_pkt, param);
		//放到待发送队列中，标记为本地网络包，并设置ttl值
		isSucc_flag = WtNetQue(proc, &net_pkt, 1);
		if (isSucc_flag)
		{
			printf("已经放进队列中\n");
		}
		//读取 队列中的网络包   
		isSucc_flag = RdNetQue(proc, &temp_net_pkt, &da, &len, &pri, &pkt_type);
		if(isSucc_flag)
		{
			printf("取网络包\n");
		}
		//本地保存这几个变量，等着后面使用：中继、等待cfm
		proc->net_pkt = temp_net_pkt;
		proc->len = len;
		proc->pri = pri;
		proc->da = da;
		proc->pkt_type = pkt_type;

		if  ((MAX_NODE_CNT > da)&&(da >= 0))	//单播
		{
			memset(proc->fail_ra, 0xff, sizeof(proc->fail_ra));
			proc->retry_cnt = 0;

			sig = proc->data_ntx_rt_req; //路由查询
			((T_Data_Ntx_Rt_Req_Param*)sig->param)->da = da;
			memcpy(((T_Data_Ntx_Rt_Req_Param*)sig->param)->fail_ra, proc->fail_ra, sizeof(proc->fail_ra));
			((T_Data_Ntx_Rt_Req_Param*)sig->param)->local_data_pri_id = proc->data_pri_id;// 为了路由cfm找到本data_ntx[?][da]

			AddSignal(sig);
			
			proc->state = WAIT_RT_CFM;

		}
		else    //非单播
		{
			proc->ra = 0xff;
			sig = proc->dlc_txpump_tx_req;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->net_pkt	= (unsigned short*)proc->net_pkt;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->len		= proc->len;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->ra			= proc->ra;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->pri		= proc->pri;
			AddSignal(sig);
			proc->wait_cfm_nsn = ((Net_Pkt*)proc->net_pkt)->pkt_sn;//等待cfm的网络包序号
			SetTimer(&proc->tx_req_timer,3);//定时器时间？？？？
			proc->state = WAIT_TX_CFM;
		}
	}
	return 0;
}

static int DataRlyTxReq(Signal *sig)
{
	Data_Ntx *proc = (Data_Ntx*)sig->dst;
	Data_Rly_Tx_Req_Param *param = (Data_Rly_Tx_Req_Param*)sig->param;
	
	if (proc->state != IDLE)
	{
		if (WtNetQue(proc, (Net_Pkt*)param->net_pkt, 0))
		{
			printf("已经放进队列中\n");
		}
	}
	else if (proc->state == IDLE)
	{
		//与上面的DataInterfaceTxReq()雷同，待测试后添加
	}
	
	return 0;
}
//路由查询的cfm
static int DataNtxRtCfm(Signal *sig)
{
	Data_Ntx *proc = (Data_Ntx*)sig->dst;
	Data_Ntx_Rt_Cfm_Param *param = (Data_Ntx_Rt_Cfm_Param*)sig->param;
	
	if (proc->state = WAIT_RT_CFM)
	{
		if (param->cfm_flag == SUCC)
		{
			proc->ra = param->ra;//路由查询到的 收节点
			sig = proc->dlc_txpump_tx_req;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->net_pkt		= (unsigned short*)proc->net_pkt;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->len			= proc->len;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->ra				= proc->ra;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->pri			= proc->pri;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->data_pri_id	= proc->data_pri_id;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->module_id		= proc->module_id;

			AddSignal(sig);
			SetTimer(&proc->tx_req_timer,3);//定时器时间？？？？
			proc->wait_cfm_nsn = ((Net_Pkt*)proc->net_pkt)->pkt_sn;//等待cfm的网络包序号
			proc->state = WAIT_TX_CFM;
		}
		else
		{
			sig = proc->data_ntx_rt_find_req; //路由查询失败，进行路由探索
			((T_Data_Ntx_Rt_Find_Req_Param*)sig->param)->da = proc->da;
			AddSignal(sig);
			SetTimer(&proc->rt_find_timer,3);//定时器时间？？？？
			proc->state = WAIT_RT_FIND_CFM;
		}
	}	
	return 0;
}
//路由探索的cfm
static int DataNtxRtFindCfm(Signal *sig)
{
	Data_Ntx *proc = (Data_Ntx*)sig->dst;
	Data_Ntx_Rt_Find_Cfm_Param *param = (Data_Ntx_Rt_Find_Cfm_Param*)sig->param;
	if (proc->state == WAIT_RT_FIND_CFM)
	{
		if (param->ra != 0xff)  //找到下一跳
		{
			proc->ra = param->ra;//保存ra
			sig = proc->dlc_txpump_tx_req;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->net_pkt	= (unsigned short*)proc->net_pkt;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->len		= proc->len;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->ra			= proc->ra;
			((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->pri		= proc->pri;
			AddSignal(sig);
			proc->wait_cfm_nsn = ((Net_Pkt*)proc->net_pkt)->pkt_sn;//等待cfm的网络包序号
			SetTimer(&proc->tx_req_timer,3);//定时器时间？？？？
			proc->state = WAIT_TX_CFM;
		}
		else  //未找到下一跳
		{
			sig = proc->data_interface_tx_cfm;
			((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_flag = FAIL;
			AddSignal(sig);
			proc->state = IDLE;
		}
	}
	return 0;
}

static int DlcTxpumpTxCfm(Signal *sig)
{
	Data_Ntx *proc = (Data_Ntx*)sig->dst;
	Dlc_Txpump_Tx_Cfm_Param *param = (Dlc_Txpump_Tx_Cfm_Param*)sig->param;
	
	unsigned short pri;
	unsigned short da;
	unsigned short len;
	unsigned short pkt_type;
	unsigned short *temp_net_pkt;//取网络包时暂时存放
	unsigned short btm;
	unsigned short ttl;
	if (proc->state == WAIT_TX_CFM)   //2017.1.5修改
	{
		if (param->cfm_flag == SUCC)
		{
			if (proc->pkt_type == LOCAL_NET_PKT)//本地网络包
			{
				if (param->nsn == proc->wait_cfm_nsn)//等待cfm的网络包序号
				{
					sig = proc->data_interface_tx_cfm;
					((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_flag = SUCC;
					((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_pkt_sn = proc->wait_cfm_nsn;//回复给上层的包序号，上层cfm使用？
					AddSignal(sig);
				}
				else
				{
					return 0;
				}
			}
			else if (proc->pkt_type == RLY_NET_PKT)//中继网络包
			{
				sig = proc->data_rly_tx_cfm;
				((T_Data_Rly_Tx_Cfm_Param*)sig->param)->cfm_flag = SUCC;
				AddSignal(sig);
			}
			//清空该包,即删除该包
			DeleteNetPkt(proc);
			//判断是否还有未发送的包
			while (proc->net_que_list.size != 0)//队列中还有网络包
			{
				RdNetQue(proc, &temp_net_pkt, &da, &len, &pri, &pkt_type);
				btm = proc->net_que_list.btm;
				ttl = proc->net_que_list.que_elmt[btm].TTL;
				//网络包ttl是否超期
				if (ttl == 0)//超期
				{
					//删除该包
					DeleteNetPkt(proc);
				}
				else  //未超期
				{
					//与前面的单播包判断一样。
					proc->net_pkt = temp_net_pkt;//本地保存，等着后面使用
					proc->len = len;
					proc->pri = pri;
					proc->da = da;
					proc->pkt_type = pkt_type;
					if ((MAX_NODE_CNT > da)&&(da >= 0))	//单播
					{
						memset(proc->fail_ra, 0xff, sizeof(proc->fail_ra));
						proc->retry_cnt = 0;

						sig = proc->data_ntx_rt_req; //路由查询
						((T_Data_Ntx_Rt_Req_Param*)sig->param)->da = da;
						memcpy(((T_Data_Ntx_Rt_Req_Param*)sig->param)->fail_ra, proc->fail_ra, sizeof(proc->fail_ra));
						AddSignal(sig);
			
						proc->state = WAIT_RT_CFM;
					}
					else    //非单播
					{
						proc->ra = 0xff;
						sig = proc->dlc_txpump_tx_req;
						((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->net_pkt	= proc->net_pkt;
						((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->len		= proc->len;
						((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->ra			= proc->ra;
						((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->pri		= proc->pri;
						AddSignal(sig);
						proc->wait_cfm_nsn = ((Net_Pkt*)proc->net_pkt)->pkt_sn;//等待cfm的网络包序号
						SetTimer(&proc->tx_req_timer,3);//定时器时间？？？？
						proc->state = WAIT_TX_CFM;
					}
					return 0;
				}
			}
			proc->state = IDLE;
			return 0;
		}
		else  //失败
		{
			proc->retry_cnt++;
			if (proc->retry_cnt > 3)  //发送失败次数超过3次
			{
				if (proc->pkt_type == LOCAL_NET_PKT)//本地网络包
				{
					sig = proc->data_interface_tx_cfm;
					((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_flag = FAIL;
					((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_pkt_sn = proc->wait_cfm_nsn;//回复给上层的包序号，上层cfm使用？
					AddSignal(sig);
				}
				else if (proc->pkt_type == RLY_NET_PKT)//中继网络包
				{
					sig = proc->data_rly_tx_cfm;
					((T_Data_Rly_Tx_Cfm_Param*)sig->param)->cfm_flag = FAIL;
					AddSignal(sig);
				}
				//清空该包,即删除该包
				DeleteNetPkt(proc);
				//判断是否还有未发送的包
				while (proc->net_que_list.size != 0)//队列中还有网络包
				{
					RdNetQue(proc, &temp_net_pkt, &da, &len, &pri, &pkt_type);
					btm = proc->net_que_list.btm;
					ttl = proc->net_que_list.que_elmt[btm].TTL;
					if (ttl == 0)//超期
					{
						//删除该包
						DeleteNetPkt(proc);
					}
					else  //未超期
					{
						//与前面的单播包判断一样。
						proc->net_pkt = temp_net_pkt;//本地保存，等着后面使用
						proc->len = len;
						proc->pri = pri;
						proc->da = da;
						proc->pkt_type = pkt_type;
						if ((MAX_NODE_CNT > da)&&(da >= 0))	//单播
						{
							memset(proc->fail_ra, 0xff, sizeof(proc->fail_ra));
							proc->retry_cnt = 0;

							sig = proc->data_ntx_rt_req; //路由查询
							((T_Data_Ntx_Rt_Req_Param*)sig->param)->da = da;
							memcpy(((T_Data_Ntx_Rt_Req_Param*)sig->param)->fail_ra, proc->fail_ra, sizeof(proc->fail_ra));
							AddSignal(sig);
			
							proc->state = WAIT_RT_CFM;
						}
						else    //非单播
						{
							proc->ra = 0xff;
							sig = proc->dlc_txpump_tx_req;
							((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->net_pkt	= proc->net_pkt;
							((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->len		= proc->len;
							((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->ra			= proc->ra;
							((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->pri		= proc->pri;
							AddSignal(sig);
							proc->wait_cfm_nsn = ((Net_Pkt*)proc->net_pkt)->pkt_sn;//等待cfm的网络包序号
							SetTimer(&proc->tx_req_timer,3);//定时器时间？？？？
							proc->state = WAIT_TX_CFM;
						}
						return 0;
					}
				}
				proc->state = IDLE;
				return 0;
			}
			else  //发送失败次数未超过3次,重新查找路由,并把失败的收节点反馈给路由模块，用于查找其他路径
			{
				proc->fail_ra[proc->retry_cnt-1] = proc->ra;
				sig = proc->data_ntx_rt_req;
				((T_Data_Ntx_Rt_Req_Param*)sig->param)->da = proc->da;
				memcpy(((T_Data_Ntx_Rt_Req_Param*)sig->param)->fail_ra, proc->fail_ra, sizeof(proc->fail_ra));
				AddSignal(sig);
				proc->state = WAIT_RT_CFM;
			}
		}
	}
	return 0;
}

static int DataTfInd(Signal *sig)
{
	Data_Ntx *proc = (Data_Ntx*)sig->dst;
	//Data_Interface_Tx_Req_Param *param = (Data_Interface_Tx_Req_Param*)sig->param;
	unsigned short btm,top,size;
	unsigned short ttl;
	btm = proc->net_que_list.btm;
	size = proc->net_que_list.size;
	top = proc->net_que_list.top;
	//队列中的每一个网络包的TTL都要做处理
	while (size != 0)
	{
		ttl = proc->net_que_list.que_elmt[btm].TTL;
		if (ttl != 0)
		{
			proc->net_que_list.que_elmt[btm].TTL--;
		}
		btm++;
		btm %= NET_QUE_MAX_NUM;
		size--;
	}
	return 0;
}

static int TxReqTimerOut(Timer *tmr)
{
	Data_Ntx *proc = (Data_Ntx*)tmr->dst;

	return 0;
}

static int RtFindTimerOut(Timer *tmr)
{
	Data_Ntx *proc = (Data_Ntx*)tmr->dst;
	Signal *sig;
	//查找路由探索结果，获取下一跳ra
	//??这里有些问题,看SDL图？？？？？
	if (proc->ra != 0xff)  //找到
	{
		//保存ra？？
		sig = proc->dlc_txpump_tx_req;
		((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->net_pkt	= (unsigned short*)proc->net_pkt;
		((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->len		= proc->len;
		((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->ra			= proc->ra;
		((T_Dlc_Txpump_Tx_Req_Param*)sig->param)->pri		= proc->pri;
		AddSignal(sig);
		SetTimer(&proc->tx_req_timer,3);//定时器时间？？？？
		proc->state = WAIT_TX_CFM;
	}
	else
	{
		sig = proc->data_interface_tx_cfm;
		((T_Data_Interface_Tx_Cfm_Param*)sig->param)->cfm_flag = FAIL;
		AddSignal(sig);
		proc->state = IDLE;
	}

	return 0;
}

void DataNtxInit(Data_Ntx* proc)
{
	proc->state = IDLE;
	proc->pkt_sn = 1;
	proc->net_pkt = NULL;
	proc->ra = 0;
	proc->retry_cnt = 0;
	proc->pri = 0;
	proc->len = 0;
	proc->da = 0;
	memset(proc->fail_ra, 0, sizeof(proc->fail_ra));
}

void DataNtxSetup(Data_Ntx* proc)
{
	Signal *sig;
	Timer *tmr;

	sig = &proc->data_interface_tx_req;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataInterfaceTxReq;
	sig->pri=SDL_PRI_NORM;
	sig->param = &proc->data_interface_tx_req_param;

	sig = &proc->data_rly_tx_req;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataRlyTxReq;
	sig->pri=SDL_PRI_NORM;
	sig->param = &proc->data_rly_tx_req_param;

	sig = &proc->data_ntx_rt_cfm;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataNtxRtCfm;
	sig->pri=SDL_PRI_NORM;
	sig->param = &proc->data_ntx_rt_cfm_param;

	sig = &proc->data_ntx_rt_find_cfm;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataNtxRtFindCfm;
	sig->pri=SDL_PRI_NORM;
	sig->param = &proc->data_ntx_rt_find_cfm_param;

	sig = &proc->dlc_txpump_tx_cfm;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DlcTxpumpTxCfm;
	sig->pri=SDL_PRI_NORM;
	sig->param = &proc->dlc_txpump_tx_cfm_param;

	sig = &proc->data_tf_ind;
	sig->next=0;
	sig->src=0;
	sig->dst=proc;
	sig->func=DataTfInd;
	sig->pri=SDL_PRI_NORM;
	sig->param = 0;

	tmr = &proc->tx_req_timer;
	tmr->next=0;
	tmr->dst=proc;
	tmr->pri=SDL_PRI_NORM;
	tmr->func=TxReqTimerOut;

	tmr = &proc->rt_find_timer;
	tmr->next=0;
	tmr->dst=proc;
	tmr->pri=SDL_PRI_NORM;
	tmr->func=RtFindTimerOut;
}