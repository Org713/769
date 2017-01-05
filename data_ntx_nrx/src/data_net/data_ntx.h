#ifndef _DATA_NTX_H
#define _DATA_NTX_H

#include "..\sdl\sdl.h"
#include "..\common.h"
#include "..\mib\mib.h"

#define DATA_LENGTH 100 //数据长度
//2017.1.2
#define NET_QUE_MAX_NUM 20//网络队列长度
#define NET_PKT_MAX_TTL 100//网络包生存时间
//#define NET_PKT_RESEND_FROM_FM_TMR  100//网络包从铁电中读出来的时间
//#define RT_FAIL_TMR 1000//路由失败时间定时，用于清除铁电中的数据
#define LOCAL_NET_PKT 0
#define RLY_NET_PKT 1

typedef struct _data_interface_tx_req_param
{
	unsigned short *data;
	unsigned short len;
	unsigned short da;
	unsigned short pri;
	unsigned short svc_type;
	unsigned short pkt_sn;//2016.12.30 包序号 cfm使用
}Data_Interface_Tx_Req_Param;

typedef struct _t_data_interface_tx_cfm_param
{
	unsigned short cfm_flag;//0 1 2
	unsigned short cfm_pkt_sn;//2016.12.30  回复cfm包序号
}T_Data_Interface_Tx_Cfm_Param;

typedef struct _t_data_ntx_to_data_nrx_test_param
{
	unsigned short *data;
	unsigned short len;
	unsigned short da;
	unsigned short pri;
	unsigned short svc_type;
	unsigned short sa;  //测试
}T_Data_Ntx_To_Data_Nrx_Test_Param;

typedef struct _net_que_elmt
{
	unsigned short pkt_type;//本地包、中继包 标志
	unsigned short TTL;
	Net_Pkt net_pkt;
}Net_Que_Elmt;

typedef struct _net_que_list
{
	unsigned short top;
	unsigned short btm;
	unsigned short size;
	//unsigned short curt_snd_flag;//正在发送标记
	Net_Que_Elmt que_elmt[NET_QUE_MAX_NUM];

}Net_Que_List;

typedef struct _t_data_ntx_rt_req_param
{
	unsigned short da;
	unsigned short fail_ra[3];
	unsigned short local_data_pri_id;// 为了路由cfm找到本data_ntx[?][da]   2017.1.4
}T_Data_Ntx_Rt_Req_Param;

typedef struct _data_ntx_rt_cfm_param
{
	unsigned short cfm_flag;
	unsigned short ra;
}Data_Ntx_Rt_Cfm_Param;

typedef struct _t_dlc_txpump_tx_req_param
{
	unsigned short *net_pkt;
	unsigned short len;
	unsigned short ra;
	unsigned short pri;
	unsigned short data_pri_id;//后面这两个参数是为了下层cfm时能找到对应的data_ntx[][]   2017.1.4
	unsigned short module_id;
}T_Dlc_Txpump_Tx_Req_Param;

typedef struct _t_data_ntx_rt_find_req_param
{
	unsigned short da;
}T_Data_Ntx_Rt_Find_Req_Param;

typedef struct _data_rly_tx_req_param
{
	unsigned short *net_pkt;
	unsigned short da;
	unsigned short len;
	unsigned short pri;
}Data_Rly_Tx_Req_Param;

typedef struct _t_data_rly_tx_cfm_param
{
	unsigned short cfm_flag;
}T_Data_Rly_Tx_Cfm_Param;

typedef struct _dlc_txpump_tx_cfm_param
{
	unsigned short cfm_flag;
	unsigned short nsn;
}Dlc_Txpump_Tx_Cfm_Param;

typedef struct _data_ntx_rt_find_cfm_param
{
	unsigned short ra;
}Data_Ntx_Rt_Find_Cfm_Param;

typedef struct _data_ntx
{
	void *entity;
	Mib *mib;
	u8 state;
	unsigned short	data_pri_id;//数据优先级ID  0.1.2.3
	unsigned short	module_id;//本模块的id号 0-31  2016.12.28

	unsigned short	rcv_data[DATA_LENGTH];//测试

	Net_Que_List	net_que_list;//网络数据包队列 2017.1.2

	unsigned short	pkt_sn;//发送的网络包序号
	unsigned short	wait_cfm_nsn;//等待cfm的网络包序号

	unsigned short	*net_pkt;//网络包地址  请求路由时暂时保存
	unsigned short	ra;	//下一跳的收地址   请求路由时暂时保存
	unsigned short	pri;  //请求路由时暂时保存
	unsigned short	len;   //请求路由时暂时保存
	unsigned short	pkt_type;//本地网络包or中继网络包
	unsigned short	da;//路由探索使用

	unsigned short	fail_ra[3];
	unsigned short	retry_cnt;

	Timer	tx_req_timer;//等待发送cfm定时器
	Timer	rt_find_timer;//路由探索等待定时器

	/***************sig received*****************/

	Signal	data_interface_tx_req;//2017.1.3
	Data_Interface_Tx_Req_Param data_interface_tx_req_param;

	Signal	data_rly_tx_req;//2017.1.2
	Data_Rly_Tx_Req_Param data_rly_tx_req_param;

	Signal	data_ntx_rt_cfm;//2017.1.3
	Data_Ntx_Rt_Cfm_Param data_ntx_rt_cfm_param;

	Signal	data_ntx_rt_find_cfm;
	Data_Ntx_Rt_Find_Cfm_Param data_ntx_rt_find_cfm_param;

	Signal	dlc_txpump_tx_cfm;
	Dlc_Txpump_Tx_Cfm_Param dlc_txpump_tx_cfm_param;

	Signal	data_tf_ind;

	/***************sig to send*****************/
	Signal*	data_interface_tx_cfm;

	Signal*	data_ntx_rt_req;
	Signal* dlc_txpump_tx_req;
	Signal* data_ntx_rt_find_req;
	Signal*	data_rly_tx_cfm;
	
	//发 和 收 测试
	Signal*	data_ntx_to_data_nrx_test;
}Data_Ntx;

extern void DataNtxSetup(Data_Ntx* proc);
extern void DataNtxInit(Data_Ntx* proc);


#endif