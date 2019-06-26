#pragma once

#include <stdint.h>

class fp16
{
	int16_t value;

	fp16(int16_t v, bool ignore) noexcept
		: value(v)
	{
	}

public: 
	fp16() noexcept : value(0) {}

	fp16(const float& f) noexcept : value(static_cast<int16_t>(f* 256.0)) {}

	fp16(const double& f) noexcept : value(static_cast<int16_t>(f* 256.0)) {}

	fp16(const int& i) noexcept : value(static_cast<int16_t>(i * 256)) {}


	fp16 operator+(const fp16& rhs) const noexcept
	{
		return fp16{ value + rhs.value, true };
	}

	fp16 operator-(const fp16& rhs) const noexcept
	{
		return fp16{ value - rhs.value, true };
	}

	fp16 operator*(const fp16& rhs) const noexcept
	{
		return fp16{ (static_cast<int32_t>(value) * rhs.value) / 256, true };
	}

	fp16 operator/(const fp16& rhs) const noexcept
	{
		return fp16{ (static_cast<int32_t>(value) * 256 / rhs.value), true };
	}


	fp16& operator+=(const fp16& rhs) noexcept
	{
		value += rhs.value;
		return *this;
	}

	fp16& operator-=(const fp16& rhs) noexcept
	{
		value -= rhs.value;
		return *this;
	}

	fp16& operator*=(const fp16& rhs) noexcept
	{
		value = (static_cast<int32_t>(value) * rhs.value) / 256;
		return *this;
	}

	fp16& operator/=(const fp16& rhs) noexcept
	{
		value = (static_cast<int32_t>(value) * 256 / rhs.value);
		return *this;
	}

	bool operator<(const fp16& rhs) const noexcept
	{
		return value < rhs.value;
	}

	bool operator>(const fp16& rhs) const noexcept
	{
		return value > rhs.value;
	}

	bool operator<=(const fp16& rhs) const noexcept
	{
		return value <= rhs.value;
	}

	bool operator>=(const fp16& rhs) const noexcept
	{
		return value >= rhs.value;
	}

	bool operator==(const fp16& rhs) const noexcept
	{
		return value == rhs.value;
	}

	operator float() const noexcept
	{
		return value / 256.0f;
	}
};

