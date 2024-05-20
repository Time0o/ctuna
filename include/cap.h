#ifndef _INCLUDE_CAP_H_
#define _INCLUDE_CAP_H_

/* Linux includes */
#include <sys/capability.h>

namespace CTuna {

	void check_cap(cap_value_t cap);
}


#endif /* _INCLUDE_CAP_H_ */
