/**
 * @file concnt.h
 * @brief defined connection counter
 * @author simpart
 */
#include "tetraring.h"
#include "protopia.h"

#ifndef __PFWCNC_H__
#define __PFWCNC_H__

/*** define ***/
/* return value */
#define PFWTCP_RET_NOTFIND  -10
#define PFWTCP_RET_CONREL   0x010    //! connection release
#define PFWTCP_RET_CONEST   0x011    //! connection establish
#define PFWTCP_RET_NOCHG    0x012    //! no change status
#define PFWTCP_RET_RESET    0x013    //! no change reset

/* connection category */
#define PFWTCP_CONSTS_SYN     0x0100 //! syn request
#define PFWTCP_CONSTS_SYNACK  0x0101 //! synack reply
#define PFWTCP_CONSTS_ETB     0x0102 //! established connection
#define PFWTCP_CONSTS_FINACK  0x0103 //! disconect

/* count type */
#define PFWTCP_CNTTP_INC  0x0200     //! increment
#define PFWTCP_CNTTP_DEC  0x0201     //! decrement

/*** struct ***/
typedef struct pfwtcp_flgsts_t {
    uint8_t  srv_addr[PIAIP_IPSIZ];  //! server address
    uint8_t  cli_addr[PIAIP_IPSIZ];  //! client address
    uint16_t srv_port;               //! server port
    uint16_t cli_port;               //! client port
    int      status;                 //! cflag status
} pfwtcp_flgsts_t;

typedef struct pfwtcp_cnt {
    uint8_t  srv_addr[PIAIP_IPSIZ];
    uint8_t  cli_addr[PIAIP_IPSIZ];
    uint16_t count;
} pfwtcp_cnt_t;


/*** protopype ***/
/* manage.c */
int pfwtcp_init  (void);
int pfwtcp_close (void);

/* concnt.c */
int        pfwtcp_cntconn  (piaip_v4hdr_t *);
int        pfwtcp_getcount (uint8_t *);
ttrchn_t * pfwtcp_getlist  (void);
int        pfwtcp_count    (pfwtcp_flgsts_t *, int);
uint8_t *  pfwtcp_getsts   (piaip_v4hdr_t *  , piatcp_hdr_t *, int *);
uint8_t *  pfwtcp_addsts   (piaip_v4hdr_t *  , piatcp_hdr_t *);
int        pfwtcp_updsts   (pfwtcp_flgsts_t *, piatcp_hdr_t *);
#endif
/* end of file */
