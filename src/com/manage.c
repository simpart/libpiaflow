/**
 * @file com/manage.c
 * @brief initialize,close all function
 * @author simpart
 */

/*** include ***/
#include "protopia.h"
#include "tetraring.h"
#include "com.h"
#include "tcp.h"

/*** function ***/
/**
 * create chain
 */
int pfw_init () {
    
    pfwtcp_init();
    
    return PFW_OK;
}

int pfw_close () {

    pfwtcp_close();

    return PFW_OK;
}
/* end of file */
