#pragma once

namespace mousecapture
{
namespace utils
{

uint64_t Crc64(const gsl::span<const uint8_t>& span, uint64_t crc = 0);

} // namespace utils
} // namespace mousecapture
