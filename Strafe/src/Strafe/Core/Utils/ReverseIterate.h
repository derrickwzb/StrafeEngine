#pragma once

//generic reverse iterator for multiple types of ranges

template <typename RangeType>
struct ReverseIterationAdapter
{
	constexpr explicit ReverseIterationAdapter(RangeType& InRange )
		: Range(InRange)
	{
	}

	ReverseIterationAdapter(ReverseIterationAdapter&&) = delete;
	ReverseIterationAdapter& operator=(ReverseIterationAdapter&&) = delete;
	ReverseIterationAdapter& operator=(const ReverseIterationAdapter&) = delete;
	ReverseIterationAdapter(const ReverseIterationAdapter&) = delete;
	~ReverseIterationAdapter() = default;

	__forceinline constexpr auto begin() const 
	{
		using std::rbegin;
		return rbegin(Range);
	}

	__forceinline constexpr auto end() const
	{
		using std::rend;
		return rend(Range);
	}

private:
	RangeType& Range;
};

template <typename ElementType, unsigned int N>
struct ReverseIterationAdapter<ElementType(&)[N]>
{
	constexpr explicit ReverseIterationAdapter(ElementType* InArray )
		: Array(InArray)
	{
	}

	ReverseIterationAdapter(ReverseIterationAdapter&&) = delete;
	ReverseIterationAdapter& operator=(ReverseIterationAdapter&&) = delete;
	ReverseIterationAdapter& operator=(const ReverseIterationAdapter&) = delete;
	ReverseIterationAdapter(const ReverseIterationAdapter&) = delete;
	~ReverseIterationAdapter() = default;

	inline constexpr ReversePointerIterator<ElementType> begin() const 
	{
		return ReversePointerIterator<ElementType>(Array + N);
	}

	inline constexpr ReversePointerIterator<ElementType> end() const 
	{
		return ReversePointerIterator<ElementType>(Array);
	}

private:
	ElementType* Array;
};


//{ 1, 2, 3, 4, 5 }; -> Output: [ "5", "4", "3", "2", "1" ]

template <typename RangeType>
inline constexpr ReverseIterationAdapter<RangeType> ReverseIterate(RangeType&& Range )
{
	return ReverseIterationAdapter<RangeType>(Range);
}