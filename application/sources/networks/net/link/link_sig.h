#ifndef __LINK_SIG_H__
#define __LINK_SIG_H__

#include "ak.h"

/*****************************************************************************/
/*  LINK_PHY task define
 */
/*****************************************************************************/
/* private define */
/* define timer */
/* define signal */
enum {
	/* public */
	LINK_PHY_INIT = AK_USER_DEFINE_SIG,
	LINK_PHY_FRAME_SEND_REQ,

	/* private */
	LINK_PHY_FRAME_SEND,
	LINK_PHY_FRAME_SEND_TO,
};

/*****************************************************************************/
/*  LINK_MAC task define
 */
/*****************************************************************************/
/* private define */
/* define timer */
/* define signal */
enum {
	LINK_MAC_INIT = AK_USER_DEFINE_SIG,
};

#endif //__LINK_SIG_H__
