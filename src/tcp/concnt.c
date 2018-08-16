/**
 * @file concnt.c
 * @brief connection counter
 * @author simpart
 */

/*** include ***/
#include <stdlib.h>
#include "tetraring.h"
#include "protopia.h"
#include "com.h"
#include "tcp.h"

/*** global ***/
extern int g_pfwtcp_cntchn;
extern int g_pfwtcp_stschn;

/*** function ***/
/**
 * (public) count tcp connection
 *
 * @param[in] (piaip_hdrv4) struct of ip header
 * @return (int) connection count
 * @return (PFW_NG) failed count
 */
int pfwtcp_cntconn (piaip_v4hdr_t * iphdr) {
    int ret     = 0;
    int ret_idx = 0;
    piatcp_hdr_t    *tcphdr = NULL;
    pfwtcp_flgsts_t *flgsts = NULL;
    
    if (NULL == iphdr) {
        return PFW_NG;
    }
    
    tcphdr = (piatcp_hdr_t *) piaip_seekpld(iphdr);
    
    /* find chain index */
    flgsts = (pfwtcp_flgsts_t *) pfwtcp_getsts(iphdr, tcphdr, &ret_idx);
    if (NULL == flgsts) {
        if (PFW_NG == ret_idx) {
            printf("contents is null\n");
            return PFW_NG;
        } else if (PFWTCP_RET_NOTFIND == ret_idx) {
            /* not find chain index, add chain */
            flgsts = (pfwtcp_flgsts_t *) pfwtcp_addsts(iphdr, tcphdr);
            return PFW_OK;
        } else {
            printf("error\n");
            return PFW_NG;
        }
    }
    
    /* update status */
    ret = pfwtcp_updsts(flgsts, tcphdr);
    switch (ret) {
        case PFWTCP_RET_CONREL: /* connection release */
            
            /* decrement counter */
            ret = pfwtcp_count(flgsts, PFWTCP_CNTTP_DEC);
            if (PFW_NG == ret) {
                return PFW_NG;
            }
            
        case PFWTCP_RET_RESET:  /* reset on the way */
            
            /* remove from chain */
            ttrchn_remove(g_pfwtcp_stschn, ret_idx);
            break;
            
        case PFWTCP_RET_CONEST: /* connection established */
            
            /* increment counter */
            ret = pfwtcp_count(flgsts, PFWTCP_CNTTP_INC);
            if (PFW_NG == ret) {
                return PFW_NG;
            }
            break;
            
        case PFWTCP_RET_NOCHG:  /* no change status */
            /* nothing to do */
            break;
    }
    
    
    return PFW_OK;
}


/**
 * (public) get connection count
 */
int pfwtcp_getcount (uint8_t *srv_addr) {
    ttrchn_t     *chain   = NULL;
    pfwtcp_cnt_t *cnt_elm = NULL;
    int ret_cnt = 0;
    
    if (NULL == srv_addr) {
        return PFW_NG;
    }
     
    /* get counter chain */
    chain = ttrchn_gethead(g_pfwtcp_cntchn);
    if (NULL == chain) {
        return PFW_NG;
    }
    while (NULL != chain) {
        cnt_elm = (pfwtcp_cnt_t *) chain->conts;
        if (NULL == cnt_elm) {
            return PFW_NG;
        }
        
        if (0 != memcmp(&(cnt_elm->srv_addr[0]), srv_addr, PIAIP_IPSIZ)) {
            /* matched server address */
            ret_cnt += cnt_elm->count;
        }

        chain = (ttrchn_t *) chain->next;
    }
    
    return ret_cnt;
}

/**
 * (public) get counter list
 */
ttrchn_t * pfwtcp_getlist () {
    return (ttrchn_t*) ttrchn_gethead(g_pfwtcp_cntchn);
}

/**
 * (private) update connection counter
 */
int pfwtcp_count (pfwtcp_flgsts_t *sts, int type) {
    ttrchn_t     *chain   = NULL;
    pfwtcp_cnt_t *cnt_elm = NULL;
    
    /* get counter chain */
    chain = ttrchn_gethead(g_pfwtcp_cntchn);
    if (NULL == chain) {
        return PFW_NG;
    }
    /* find target counter */
    while (NULL != chain) {
        cnt_elm = (pfwtcp_cnt_t *) chain->conts;
        if (NULL == cnt_elm) {
            return PFW_NG;
        }
        
        if ( (0 != memcmp(&(sts->srv_addr[0]), &(cnt_elm->srv_addr[0]), PIAIP_IPSIZ)) &&
             (0 != memcmp(&(sts->cli_addr[0]), &(cnt_elm->cli_addr[0]), PIAIP_IPSIZ)) ) {
            /* matched count chain */
            break;
        }
        
        chain   = (ttrchn_t *) chain->next;
        cnt_elm = NULL;
    }
    if (NULL == cnt_elm) {
        /* not add chain yet */
        if (PFWTCP_CNTTP_DEC == type) {
            /* invalid type */
            return PFW_NG;
        }
        
        cnt_elm = (pfwtcp_cnt_t *) malloc(sizeof(pfwtcp_cnt_t));
        if (NULL == cnt_elm) {
            return PFW_NG;
        }
        memset(cnt_elm, 0x00, sizeof(pfwtcp_cnt_t));
        
        /* set contents */
        memcpy(&(cnt_elm->srv_addr[0]), &(sts->srv_addr[0]), PIAIP_IPSIZ);
        memcpy(&(cnt_elm->cli_addr[0]), &(sts->cli_addr[0]), PIAIP_IPSIZ);
    }
    
    
    /* update counter */
    if (PFWTCP_CNTTP_INC == type) {
        cnt_elm->count++;
    } else if (PFWTCP_CNTTP_DEC == type) {
        cnt_elm->count--;
    } else {
        return PFW_NG;
    }
    
    return PFW_OK;
}

/**
 * (private) find status from chain
 */
uint8_t * pfwtcp_getsts (piaip_v4hdr_t * iphdr, piatcp_hdr_t *tcphdr, int *ret) {
    ttrchn_t        *chain  = NULL;
    pfwtcp_flgsts_t *flgsts = NULL;
    int sport = 0;
    int dport = 0;
    
    /* get chain */
    chain = ttrchn_gethead(g_pfwtcp_stschn);
    if (NULL == chain) {
        *ret = PFW_NG;
        return NULL;
    }
    chain = (ttrchn_t *) chain->next;  // head is empty
    
    sport  = piatcp_getport(tcphdr, PIATCP_PORT_SRC);
    dport  = piatcp_getport(tcphdr, PIATCP_PORT_DST);
    
    /* find matched chain */
    while (NULL != chain) {
        flgsts = (pfwtcp_flgsts_t *) chain->conts;
        if (NULL == flgsts) {
            printf("contents is null\n");
            *ret = PFW_NG;
            return NULL;
        }
        
        /* check server address */
        if ( (0 != memcmp(&(flgsts->srv_addr[0]), iphdr->sip, PIAIP_IPSIZ)) &&
             (0 != memcmp(&(flgsts->cli_addr[0]), iphdr->sip, PIAIP_IPSIZ)) ) {
            goto PFWTCP_NXTCHAIN;
        }
        /* check client address */
        if ( (0 != memcmp(&(flgsts->srv_addr[0]), iphdr->dip, PIAIP_IPSIZ)) &&
             (0 != memcmp(&(flgsts->cli_addr[0]), iphdr->dip, PIAIP_IPSIZ)) ) {
            goto PFWTCP_NXTCHAIN;
        }
        /* check source port */
        if ( (flgsts->srv_port != sport) &&
             (flgsts->cli_port != sport) ) {
            goto PFWTCP_NXTCHAIN;
        }
        /* check dest port */
        if ( (flgsts->srv_port != dport) && 
             (flgsts->cli_port != dport) ) {
            goto PFWTCP_NXTCHAIN;
        }
        
        *ret = (chain->idx)-1;
        return (uint8_t *) flgsts;
        
        PFWTCP_NXTCHAIN: chain = (ttrchn_t *) chain->next;
    }
    
    *ret = PFWTCP_RET_NOTFIND;
    return NULL;
}

/**
 * (private) add status to chain
 */
uint8_t * pfwtcp_addsts (piaip_v4hdr_t * iphdr, piatcp_hdr_t *tcphdr) {
    int ret = 0;
    pfwtcp_flgsts_t *addsts = NULL;
    
    /* check cflag */
    if (PIA_TRUE != piatcp_issyn(tcphdr)) {
        /* this is not init status, not add chain */
        return NULL;
    }
    
    /* init chain element */
    addsts = (pfwtcp_flgsts_t *) malloc(sizeof(pfwtcp_flgsts_t));
    if (NULL == addsts) {
        return NULL;
    }
    memset(addsts, 0x00, sizeof(pfwtcp_flgsts_t));
    
    /* set element contents */
    memcpy(addsts->srv_addr, iphdr->dip, PIAIP_IPSIZ);
    memcpy(addsts->cli_addr, iphdr->sip, PIAIP_IPSIZ);
    addsts->srv_port = piatcp_getport(tcphdr, PIATCP_PORT_DST);
    addsts->cli_port = piatcp_getport(tcphdr, PIATCP_PORT_SRC);
    addsts->status   = PFWTCP_CONSTS_SYN;
    
    /* add connection chain */
    ret = ttrchn_add(g_pfwtcp_stschn, addsts);
    if (TTR_NG == ret) {
        return NULL;
    }
    
    return (uint8_t *) addsts;
}

/**
 * (private) update flag status
 *
 * @param[in] (pfwtcp_flgsts_t) flag status struct
 * @param[in] (piatcp_hdr_t) tcp header struct
 * @return (int) PFWTCP_RET_CONREL : release connection
 * @return (int) PFWTCP_RET_RESET  : reset connection (no established)
 * @return (int) PFWTCP_RET_CONEST : connection established
 * @return (int) PFWTCP_RET_NOCHG  : no change status
 */
int pfwtcp_updsts (pfwtcp_flgsts_t *sts, piatcp_hdr_t *tcphdr) {
    
    /* check status and controll flag */
    if (PIA_TRUE == piatcp_isrst(tcphdr)) {

        /* reset flag */
        if (PFWTCP_CONSTS_ETB == sts->status) {
            /* release connection */
            return PFWTCP_RET_CONREL;
        }
        return PFWTCP_RET_RESET;

    } else if ( (PFWTCP_CONSTS_SYN == sts->status) &&
                (PIA_TRUE == piatcp_issynack(tcphdr)) ) {
        
        /* connection reply status (syn -> synack) */
        sts->status = PFWTCP_CONSTS_SYNACK;
        
    } else if ( (PFWTCP_CONSTS_SYNACK == sts->status) &&
                (PIA_TRUE == piatcp_isack(tcphdr)) ) {
        
        /* established (syn-ack -> ack) */
            sts->status = PFWTCP_CONSTS_ETB;
            return PFWTCP_RET_CONEST; 
        
    } else if ( (PFWTCP_CONSTS_ETB == sts->status) &&
                (PIA_TRUE == piatcp_isfinack(tcphdr)) ) {
        
        /* release request (ack -> finack) */
        sts->status = PFWTCP_CONSTS_FINACK;
        
    } else if ( (PFWTCP_CONSTS_FINACK == sts->status) &&
                (PIA_TRUE == piatcp_isack(tcphdr)) ) {
        
        /* connection release (finack -> ack) */
        return PFWTCP_RET_CONREL;
        
    }

    return PFWTCP_RET_NOCHG;
}
/* end of file */
