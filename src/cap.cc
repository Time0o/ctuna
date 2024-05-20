/* stdcxx lib includes */
#include <stdexcept>

/* Linux includes */
#include <sys/capability.h>

/* local includes */
#include "cap.h"


void check_cap(cap_value_t cap)
{
	auto caps { ::cap_get_proc() };

	cap_flag_value_t flag { static_cast<cap_flag_value_t>(0) };
	::cap_get_flag(caps, cap, CAP_EFFECTIVE, &flag);

	if (!flag)
	  throw std::runtime_error { "missing capability" };
}
