#ifndef LOGGING_H
#define LOGGING_H

#include <format>
#include <iostream>

namespace Nocturn
{
	/**
	 * \brief Function that outputs the formatted input.
	 * \tparam Args type of Args
	 * \param fmt _Fmt_string format
	 * \param args input args
	 */
	template< typename... Args >
	void Log( std::_Fmt_string< Args... > fmt, Args &&...args )
	{
		std::cout << std::format( "[{} : {}]: ", __FILE__, __LINE__ );
		std::cout << std::format( fmt, std::forward< Args >( args )... );
		std::cout << '\n';
	}
} // namespace Nocturn
#endif