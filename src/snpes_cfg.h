#ifndef __SNPES_CFG_H__
#define __SNPES_CFG_H__

#if !defined(SNPES_MICRO)
#define SNPES_MICRO
#define PKT_SIZE   64
#define META_SIZE  6
#define BUF_CNT    8
#define BUF_SIZE   PKT_SIZE*BUF_CNT
#define S_IN_CNT   7
#define S_OUT_CNT  BUF_CNT-S_IN_CNT
#define CLT_CNT    8
#endif /* SNPES_MICRO */

#define MAX_TIMEOUT_CNT 2
#define TIMEOUT_THLD 5000

#endif /* __SNPES_CFG_H__ */
