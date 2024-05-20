#ifndef _INCLUDE_TUN_H_
#define _INCLUDE_TUN_H_

/* libc includes */
#include <stdint.h>

/* Linux includes */
#include <linux/if.h>

/* local includes */
#include "exception.h"

namespace CTuna {
	struct TUN_error;
	class TUN;
}


struct CTuna::TUN_error : public Exception
{
	using Exception::Exception;
};


class CTuna::TUN
{
	private:

		int _fd { -1 };
		int _sock { -1 };

		char const *_name;
		char const *_addr;
		char const *_netmask;

	public:

		explicit TUN(char const *name);

		~TUN();

		char const *name() const
		{ return _name; }

		char const *addr() const
		{ return _addr; }

		char const *netmask() const
		{ return _netmask; }

		void open(char const *addr, char const *netmask);

		void intercept();

		size_t read(uint8_t *buf, size_t buf_size);
};


#endif /* _INCLUDE_TUN_H_ */
