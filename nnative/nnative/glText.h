#pragma once

#include <array>
#include <stdint.h>

namespace glText
{
	namespace glFont
	{
		static const char* _A =
			"..XX.."
			"..XX.."
			".X..X."
			".X..X."
			"X....X"
			"X....X"
			"XXXXXX"
			"X....X"
			"X....X";

		static const char* _B =
			"XXXX.."
			"X...X."
			"X...X."
			"XXXX.."
			"X...X."
			"X....X"
			"X....X"
			"X....X"
			"XXXXX.";

		static const char* letters[] = { _A, _B };

		template <int H, int W>
		struct FontItem 
		{
			static constexpr int height() { return H; }
			static constexpr int width() { return W; }

			std::array<uint32_t, H* W> data;
		};

		using FontItemInst = FontItem<21, 15>;


		void GenerateLetter(FontItemInst& dst, const char* src, int rawH, int rawW)
		{
			std::vector<unsigned> rawIntensity(dst.data.size(), 0);

			for (int srcY = 0; srcY < rawH; ++srcY)
			{
				for (int srcX = 0; srcX < rawW; ++srcX)
				{
					int val = src[(rawH - srcY - 1) * rawW + srcX] == 'X' ? 255 : 0;
					int dstXc = srcX * 2 + 2;
					int dstYc = srcY * 2 + 2;

					rawIntensity[(dstYc + 0) * dst.width() + (dstXc + 0)] += val;

					rawIntensity[(dstYc + 1) * dst.width() + (dstXc + 0)] += val / 3;
					rawIntensity[(dstYc - 1) * dst.width() + (dstXc + 0)] += val / 3;
					rawIntensity[(dstYc + 0) * dst.width() + (dstXc + 1)] += val / 3;
					rawIntensity[(dstYc + 0) * dst.width() + (dstXc - 1)] += val / 3;

					rawIntensity[(dstYc + 1) * dst.width() + (dstXc + 1)] += val / 9;
					rawIntensity[(dstYc + 1) * dst.width() + (dstXc - 1)] += val / 9;
					rawIntensity[(dstYc - 1) * dst.width() + (dstXc + 1)] += val / 9;
					rawIntensity[(dstYc - 1) * dst.width() + (dstXc - 1)] += val / 9;

					rawIntensity[(dstYc + 2) * dst.width() + (dstXc + 0)] += val / 15;
					rawIntensity[(dstYc - 2) * dst.width() + (dstXc + 0)] += val / 15;
					rawIntensity[(dstYc + 0) * dst.width() + (dstXc + 2)] += val / 15;
					rawIntensity[(dstYc + 0) * dst.width() + (dstXc - 2)] += val / 15;

					rawIntensity[(dstYc + 2) * dst.width() + (dstXc + 1)] += val / 33;
					rawIntensity[(dstYc + 2) * dst.width() + (dstXc - 1)] += val / 33;
					rawIntensity[(dstYc - 2) * dst.width() + (dstXc + 1)] += val / 33;
					rawIntensity[(dstYc - 2) * dst.width() + (dstXc - 1)] += val / 33;
					rawIntensity[(dstYc + 1) * dst.width() + (dstXc + 2)] += val / 33;
					rawIntensity[(dstYc - 1) * dst.width() + (dstXc + 2)] += val / 33;
					rawIntensity[(dstYc + 1) * dst.width() + (dstXc - 2)] += val / 33;
					rawIntensity[(dstYc - 1) * dst.width() + (dstXc - 2)] += val / 33;


					//rawIntensity[(2 * srcY + 0) * dst.width() + (2 * srcX + 2)] += val / 8;
					//rawIntensity[(2 * srcY + 2) * dst.width() + (2 * srcX + 0)] += val / 8;
					//rawIntensity[(2 * srcY + 4) * dst.width() + (2 * srcX + 2)] += val / 8;
					//rawIntensity[(2 * srcY + 2) * dst.width() + (2 * srcX + 4)] += val / 8;

					//rawIntensity[(2 * srcY + 0) * dst.width() + (2 * srcX + 0)] += val / 32;
					//rawIntensity[(2 * srcY + 4) * dst.width() + (2 * srcX + 0)] += val / 32;
					//rawIntensity[(2 * srcY + 0) * dst.width() + (2 * srcX + 4)] += val / 32;
					//rawIntensity[(2 * srcY + 4) * dst.width() + (2 * srcX + 4)] += val / 32;
				}
			}

			for (int idx = 0; idx < dst.data.size(); ++idx)
			{
				unsigned ri = rawIntensity[idx];
				if (rawIntensity[idx] >= 255)
				{
					dst.data[idx] = 0xffffffff;
				}
				else if (ri == 0)
				{
					dst.data[idx] = 0x0;
				}
				else
				{
					dst.data[idx] = (ri << 24) | (ri << 16) | (ri << 8) | (ri);
				}
			}
		}

		static std::vector<FontItemInst> font;
		static bool initialized = false;

		void GenerateFonts()
		{
			font.resize(sizeof(letters) / sizeof(letters[0]));
			for (int i = 0; i < font.size(); ++i)
			{
				GenerateLetter(font[i], letters[i], 9, 6);
			}
		}

		FontItemInst GetItem(char ltr)
		{
			if (!initialized)
			{
				GenerateFonts();
				initialized = true;
			}

			return font[ltr-'A'];
		}

	}


}