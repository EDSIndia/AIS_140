/******************** (C) COPYRIGHT 2022 EDS INDIA *****************************
* File Name     : TCP.h					       	               *
* Author        : SANTANU ROY                                                  *
* Date          : 16/12/2022                                                   *
* Description   : This File Defines variables & function used in TCP SOCKET    * 
* Revision	: Rev0 						      	       *
*******************************************************************************/



#define OPEN_SOCKET      0
#define WAIT_OPEN        1
#define TCP_CONNECTED    2
#define ACK_DATA         3
#define CLOSE_SOCKET     4
#define DEACT_CONTEXT    5


void TCP_Connection(char SRVR);                                                      // TCP Connection managment 
