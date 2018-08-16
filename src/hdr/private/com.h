/**
 * @file com.h
 * @brief defined common
 * @autor simpart
 */
#ifndef __PFWCOM_H__
#define __PFWCOM_H__

/*** define ***/
/**
 * @brief value of function return
 */
#define PFW_OK  0  //! function is successful
#define PFW_NG -1  //! function is failed
/**
 * @brief error value
 */
#define PFW_ERR_NMCH  -10 //! not match
/**
 * @brief common flag
 */
#define PFW_TRUE  1  //! value of true
#define PFW_FALSE 0  //! value of false


/*** prototype ***/
int pfw_init  (void);
int pfw_close (void);



#endif
/* end of file */
