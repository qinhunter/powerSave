#ifndef FAN_H
#define FAN_H
#include <stdlib.h> 
#include <stdio.h>
#include "../posix/ndpi_api.h" //iphdr
#include <string.h> 
#include <pcap.h>

#define DBG_PRINT if(1 == g_ulDbgPrint) printf

int g_ulDbgPrint = 0;

//路由表元素节点
typedef struct node {
    struct node *pLeftChild;
    struct node *pRightChild;
    int iPort;
}NODE;

//路由表root


NODE *createNode();

void createRouteTree(NODE * g_pRouteTree, int iRoute, int iMask, int iPort);

int getIpFwdPort(NODE * g_pRouteTree, int iIp);

//'创建节点'函数
NODE *createNode() {
    NODE *pNode = malloc(sizeof(NODE));
    pNode->pLeftChild = NULL;
    pNode->pRightChild = NULL;
    pNode->iPort = -1;
    return pNode;
}

//'创建路由表'函数
void createRouteTree(NODE * g_pRouteTree, int iRoute, int iMask, int iPort) {
    int i = 0;
    // 0 -- left, 1 -- right
    int iLeftOrRight = 0;
    
    if (g_pRouteTree == NULL) {
        g_pRouteTree = createNode();
	printf("createRouteTree create the root node \n");
    }
    
    DBG_PRINT("input rounte: %8x, mask: %d, port: %d\n", iRoute, iMask, iPort);

    NODE *pCurrNode = g_pRouteTree;
    for (i = 0; i < iMask; i++) {
        iLeftOrRight = (iRoute >> (31 - i)) & 0x1;
        
        if(0 == iLeftOrRight) {
            if (NULL == pCurrNode->pLeftChild) {
                pCurrNode->pLeftChild = createNode();
            }
            pCurrNode = pCurrNode->pLeftChild;
            DBG_PRINT("0 left\n");
        }
        else {
            if (NULL == pCurrNode->pRightChild) {
                pCurrNode->pRightChild = createNode();
            }
            pCurrNode = pCurrNode->pRightChild;
            DBG_PRINT("1 right\n");
        }
        
    }

    pCurrNode->iPort = iPort;
    DBG_PRINT("%d port\n", iPort);
    return;
}

//ip转发查找路由表函数
int getIpFwdPort(NODE * g_pRouteTree, int iIp) {
	int i = 0, iLeftOrRight = 0, iPort = -1;
	NODE *pCurrNode = g_pRouteTree;

	iPort = (-1 == pCurrNode->iPort)?iPort:pCurrNode->iPort;

	DBG_PRINT("input ip: %8x\n", iIp);

	for (i = 0; i < 32; i++) {
		iLeftOrRight = (iIp >> (31-i)) & 0x1;

		if (0 == iLeftOrRight) {
			if (NULL != pCurrNode->pLeftChild) {
				pCurrNode = pCurrNode->pLeftChild;
				iPort = (-1 == pCurrNode->iPort)?iPort:pCurrNode->iPort;
				DBG_PRINT("0 go left, %d port\n", iPort);
			}
			else {
				break;
			}
		}
		else {// (1 == iLeftOrRight)
			if (NULL != pCurrNode->pRightChild) {
				pCurrNode = pCurrNode->pRightChild;
				iPort = (-1 == pCurrNode->iPort)?iPort:pCurrNode->iPort;
				DBG_PRINT("1 go right, %d port\n", iPort);
			}
			else {
				break;
			}
		}
	}

	return iPort;
}

#endif
