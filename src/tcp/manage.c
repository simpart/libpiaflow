/**
 * @file tcp/manage.c
 * @brief initialize,close tcp function
 * @author simpart
 */

/*** include ***/
#include "protopia.h"
#include "tetraring.h"
#include "com.h"

/*** global ***/
int g_pfwtcp_cntchn = 0;  //! connection counter chain
int g_pfwtcp_stschn = 0;  //! cflag buffer chain

/*** function ***/
/**
 * create chain
 */
int pfwtcp_init () {

    g_pfwtcp_cntchn = ttrchn_create();
    g_pfwtcp_stschn = ttrchn_create();
    
    return PFW_OK;
}

int pfwtcp_close () {

    ttrchn_free(g_pfwtcp_cntchn);
    ttrchn_free(g_pfwtcp_stschn);
    
    return PFW_OK;
}
/* end of file */
