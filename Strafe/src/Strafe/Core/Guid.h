
#pragma once

namespace strafe
{
	struct Guid
	{
		Guid() = default;
		Guid(const uint64_t& id) : m_RawId(id) {}

		uint64_t m_RawId{};

		static const Guid null;

		bool operator==(const Guid& rhs) const { return m_RawId == rhs.m_RawId; }
		bool operator!=(const Guid& rhs) const { return m_RawId != rhs.m_RawId; }
		bool operator<(const Guid& rhs) const { return m_RawId < rhs.m_RawId; }
		Guid& operator=(const Guid& rhs) = default;
		operator uint64_t() const { return m_RawId; }
	};

	class GuidGenerator
	{
		//The number of bits to represent time
		static constexpr inline uint32_t BitLenTime{ 39 };
		//The number of bits to represent sequence
		static constexpr inline uint32_t BitLenSequence{ 8 };
		//The number of bits to represent machine id
		static constexpr inline uint32_t BitLenMachineId{ 63 - BitLenTime - BitLenSequence };
		//Mask
		static constexpr inline uint16_t MaskSequence{ (1 << BitLenSequence) - 1 };

		struct GuidSettings
		{
			std::chrono::year_month_day m_Date;
			std::chrono::utc_clock::time_point m_StartTime;
			uint16_t m_MachineId;
		};

	public:
		GuidGenerator();

		static uint64_t GenerateGuid();
		static uint64_t GenerateGuid(const std::chrono::utc_clock::time_point& timestamp);

		static uint64_t GetTime(const uint64_t& uuid);
		static uint16_t GetSequence(const uint64_t& uuid);
		static uint16_t GetMachineId(const uint64_t& uuid);

	private:
		inline static GuidSettings m_Settings{};

		inline static uint64_t m_StartTime{};
		inline static uint64_t m_ElapsedTime{};
		inline static uint16_t m_Sequence{};
		inline static uint16_t m_MachineID{};

		inline static bool m_Initialized{ false };

		static int64_t ConvertTime(const std::chrono::utc_clock::time_point& time);
		static std::string GetMacAddress();
		static uint64_t ConvertMacAddressToNumber(const std::string& mac);
	};
}
