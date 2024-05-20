#ifndef _INCLUDE_CAP_H_
#define _INCLUDE_CAP_H_

/* Linux includes */
#include <sys/capability.h>

/* local includes */
#include "exception.h"

namespace CTuna {
	struct Cap_missing;
}


struct CTuna::Cap_missing : public Exception
{
	using Exception::Exception;
};


namespace CTuna {

	void check_cap(cap_value_t cap);
}


#endif /* _INCLUDE_CAP_H_ */
