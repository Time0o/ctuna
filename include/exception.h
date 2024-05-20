#ifndef _INCLUDE_EXCEPTION_H_
#define _INCLUDE_EXCEPTION_H_


namespace CTuna {
	class Exception;
}


class CTuna::Exception
{
	private:

		char const *_what;

	public:

		Exception()
		:
			_what { "" }
		{ }

		explicit Exception(char const *what)
		:
			_what { what }
		{ }

		char const *what() const
		{ return _what; }
};


#endif /* _INCLUDE_EXCEPTION_H_ */
