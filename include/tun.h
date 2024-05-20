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


/**
 * TUN interface
 */
class CTuna::TUN
{
	private:

		int _fd { -1 };
		int _sock { -1 };

		char const *_name;
		char const *_addr;

	public:

		/**
		 * Constructor
		 *
		 * \param name  interface name
		 * \param name  interface IPv4 address
		 * \param name  interface netmask
		 *
		 * \throw CTuna::TUN_error  if some required operation fails
		 *
		 * This creates the interface, assigns it an IPv4 address and puts it
		 * into the 'up' state.
		 */
		explicit TUN(char const *name,
		             char const *addr);

		/**
		 * Destructor
		 *
		 * This will undo all setup operations performed by the constructor.
		 */
		~TUN();

		char const *name() const
		{ return _name; }

		char const *addr() const
		{ return _addr; }

		/**
		 * Intercept all outgoing network traffic
		 *
		 * \param name  interface name
		 * \param name  interface IPv4 address
		 * \param name  interface netmask
		 *
		 * \throw CTuna::TUN_error  if some required operation fails
		 *
		 * This sets the default route for all outgoing packets to this
		 * interface.
		 */
		void intercept();

		/**
		 * Read packet from interface
		 *
		 * \param buf       preallocated buffer to hold packet data
		 * \param buf_size  size of `buf` in bytes
		 *
		 * \return  size of packet in bytes
		 *
		 * \throw CTuna::TUN_error  if a read operation fails
		 */
		size_t read(uint8_t *buf, size_t buf_size);
};


#endif /* _INCLUDE_TUN_H_ */
