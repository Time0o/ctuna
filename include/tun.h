#ifndef _INCLUDE_TUN_H_
#define _INCLUDE_TUN_H_

/* stdcxx includes */
#include <cstdint>
#include <string>
#include <vector>

/* Linux includes */
#include <linux/if.h>


class TUN
{
	private:

		int _fd { -1 };
		int _sock { -1 };

		char _name[IFNAMSIZ];

		std::string _addr;
		std::string _netmask;

	public:

		~TUN();

		std::string name() const
		{ return _name; }

		std::string addr() const
		{ return _addr; }

		std::string netmask() const
		{ return _netmask; }

		void open(std::string const &addr, std::string const &netmask);

		void intercept();

		std::vector<std::uint8_t> read();
};


#endif /* _INCLUDE_TUN_H_ */
