// =====================================================================
//   @ Author: Cucorianu Eusebiu Adelin                                                                                      
//   @ Create Time: 22-10-2022 8:03 PM                                                                                                                                                
//   @ Contact: cucorianu.adelin@protonmail.com                                                                                                                          
//   @ Modified time: 23-10-2022 2:33 PM                                                                                                                                    
//   @ Description: This is the program's platfrom which helps managing
//	   the workflow.
// =====================================================================

#ifndef PLATFORM_H
#define PLATFORM_H

#include <cstdint>

namespace Nocturn
{
	using int8	 = int8_t;
	using uint8	 = uint8_t;
	using uint16 = uint16_t;
	using int16	 = int16_t;
	using uint32 = uint32_t;
	using int32	 = int32_t;
	using uint64 = uint64_t;
	using int64	 = int64_t;

	enum class RStatus : uint8
	{
		RSucces = 0,
		RInvalidNumberOfArguments,
		RInvalidPathName,
		RFail,
		End
	};

	constexpr RStatus RSucces{ RStatus::RSucces };
	constexpr RStatus RInvalidNumberOfArguments{ RStatus::RInvalidNumberOfArguments };
	constexpr RStatus RInvalidPathName{ RStatus::RInvalidPathName };
	constexpr RStatus RFail{ RStatus::RFail };

	#define CPP_VERSION_17 201703L
	#define CPP_VERSION_20 202002L

	#if __cplusplus >= CPP_VERSION_17
	#define NODISCARD [[nodiscard]]
	#else
	#define NODISCARD
	#endif

	#if __cplusplus >= CPP_VERSION_20
	#define LIKELY [[likely]]
	#define UNLIKELY [[unlikely]]
	#else
	#define LIKELY
	#define UNLIKELY
	#endif
} // namespace Nocturn
#endif 