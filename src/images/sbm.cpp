/* xoreos-tools - Tools to help with xoreos development
 *
 * xoreos-tools is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos-tools is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos-tools. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  Decoding SBM (font bitmap data).
 */

#include <cstring>

#include "src/common/util.h"
#include "src/common/readstream.h"
#include "src/common/error.h"

#include "src/images/sbm.h"
#include "src/images/util.h"

namespace Images {

SBM::SBM(Common::SeekableReadStream &sbm, bool deswizzle) {
	_format = kPixelFormatB8G8R8A8;

	load(sbm, deswizzle);
}

SBM::~SBM() {
}

void SBM::load(Common::SeekableReadStream &sbm, bool deswizzle) {
	try {

		readData(sbm, deswizzle);

	} catch (Common::Exception &e) {
		e.add("Failed reading SBM file");
		throw;
	}

	// Flip SBM image data to make their origin the lower left corner too
	::Images::flipVertically(_mipMaps[0]->data.get(), _mipMaps[0]->width, _mipMaps[0]->height, 4);
}

void SBM::readData(Common::SeekableReadStream &sbm, bool deswizzle) {
	if ((sbm.size() % 1024) != 0)
		throw Common::Exception("Invalid SBM (%u)", (uint)sbm.size());

	const size_t rowCount = (sbm.size() / 1024);

	_mipMaps.emplace_back(std::make_unique<MipMap>());

	_mipMaps[0]->width  = 4 * 32;
	_mipMaps[0]->height = NEXTPOWER2((uint32_t) rowCount * 32);
	_mipMaps[0]->size   = _mipMaps[0]->width * _mipMaps[0]->height * 4;

	_mipMaps[0]->data = std::make_unique<byte[]>(_mipMaps[0]->size);

	// SBM data consists of character sized 32 * 32 pixels, with 2 bits per pixel.
	// 4 characters each are on top of each other, occupying the same x/y
	// coordinates but different planes.
	// We'll unpack them and draw them next to each other instead.

	static const int masks [4] = { 0x03, 0x0C, 0x30, 0xC0 };
	static const int shifts[4] = {    0,    2,    4,    6 };

	byte *data = _mipMaps[0]->data.get();
	byte buffer[1024];
	for (size_t c = 0; c < rowCount; c++) {

		if (sbm.read(buffer, 1024) != 1024)
			throw Common::Exception(Common::kReadError);

		for (int y = 0; y < 32; y++) {
			for (int plane = 0; plane < 4; plane++) {
				for (int x = 0; x < 32; x++) {
					const uint32_t offset = deswizzle ? deSwizzleOffset(x, y, 32, rowCount) : (y * 32 + x);

					const byte a = ((buffer[offset] & masks[plane]) >> shifts[plane]) * 0x55;

					*data++ = 0xFF; // B
					*data++ = 0xFF; // G
					*data++ = 0xFF; // R
					*data++ = a;    // A
				}
			}
		}
	}

	byte *dataEnd = _mipMaps[0]->data.get() + _mipMaps[0]->size;
	std::memset(data, 0, dataEnd - data);
}

} // End of namespace Images
