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
 *  DDS texture (DirectDraw Surface or BioWare's own format) loading).
 */

#include <memory>

#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/readstream.h"

#include "src/images/dds.h"
#include "src/images/util.h"

static const uint32 kDDSID  = MKTAG('D', 'D', 'S', ' ');
static const uint32 kDXT1ID = MKTAG('D', 'X', 'T', '1');
static const uint32 kDXT3ID = MKTAG('D', 'X', 'T', '3');
static const uint32 kDXT5ID = MKTAG('D', 'X', 'T', '5');

static const uint32 kHeaderFlagsHasMipMaps = 0x00020000;

static const uint32 kPixelFlagsHasAlpha  = 0x00000001;
static const uint32 kPixelFlagsHasFourCC = 0x00000004;
static const uint32 kPixelFlagsIsIndexed = 0x00000020;
static const uint32 kPixelFlagsIsRGB     = 0x00000040;

namespace Images {

DDS::DDS(Common::SeekableReadStream &dds) {
	load(dds);
}

DDS::~DDS() {
}

bool DDS::detect(Common::SeekableReadStream &dds) {
	uint32 pos = dds.pos();

	uint32 fourCC = 0;
	try {
		fourCC = dds.readUint32BE();
	} catch (...) {
	}

	dds.seek(pos);

	return fourCC == kDDSID;
}

void DDS::load(Common::SeekableReadStream &dds) {
	try {

		DataType dataType;

		readHeader(dds, dataType);
		readData  (dds, dataType);

	} catch (Common::Exception &e) {
		e.add("Failed reading DDS file");
		throw;
	}

	// In xoreos-tools, we always want decompressed images
	decompress();
}

void DDS::readHeader(Common::SeekableReadStream &dds, DataType &dataType) {
	if (dds.readUint32BE() == kDDSID)
		// We found the FourCC of a standard DDS
		readStandardHeader(dds, dataType);
	else
		// FourCC not found, should be a BioWare DDS then
		readBioWareHeader(dds, dataType);
}

void DDS::readStandardHeader(Common::SeekableReadStream &dds, DataType &dataType) {
	// All DDS header should be 124 bytes (+ 4 for the FourCC) big
	if (dds.readUint32LE() != 124)
		throw Common::Exception("Header size invalid");

	// DDS features
	uint32 flags = dds.readUint32LE();

	// Image dimensions
	uint32 height = dds.readUint32LE();
	uint32 width  = dds.readUint32LE();

	if ((width >= 0x8000) || (height >= 0x8000))
		throw Common::Exception("Unsupported image dimensions (%ux%u)", width, height);

	dds.skip(4 + 4); // Pitch + Depth
	//uint32 pitchOrLineSize = dds.readUint32LE();
	//uint32 depth           = dds.readUint32LE();
	uint32 mipMapCount     = dds.readUint32LE();

	// DDS doesn't provide any mip maps, only one full-size image
	if ((flags & kHeaderFlagsHasMipMaps) == 0)
		mipMapCount = 1;

	dds.skip(44); // Reserved

	// Read the pixel data format
	DDSPixelFormat format;
	format.size     = dds.readUint32LE();
	format.flags    = dds.readUint32LE();
	format.fourCC   = dds.readUint32BE();
	format.bitCount = dds.readUint32LE();
	format.rBitMask = dds.readUint32LE();
	format.gBitMask = dds.readUint32LE();
	format.bBitMask = dds.readUint32LE();
	format.aBitMask = dds.readUint32LE();

	// Detect which specific format it describes
	detectFormat(format, dataType);

	if (!hasValidDimensions(_format, width, height))
		throw Common::Exception("Invalid dimensions (%dx%d) for format %d", width, height, _format);

	dds.skip(16 + 4); // DDCAPS2 + Reserved

	_mipMaps.reserve(mipMapCount);
	for (uint32 i = 0; i < mipMapCount; i++) {
		std::unique_ptr<MipMap> mipMap = std::make_unique<MipMap>();

		mipMap->width  = MAX<uint32>(width , 1);
		mipMap->height = MAX<uint32>(height, 1);

		setSize(*mipMap);

		width  >>= 1;
		height >>= 1;

		_mipMaps.push_back(mipMap.release());
	}

}

#define IsPower2(x) ((x) && (((x) & ((x) - 1)) == 0))
void DDS::readBioWareHeader(Common::SeekableReadStream &dds, DataType &dataType) {
	dataType = kDataTypeDirect;

	dds.seek(0);

	// Image dimensions
	uint32 width  = dds.readUint32LE();
	uint32 height = dds.readUint32LE();

	if ((width >= 0x8000) || (height >= 0x8000))
		throw Common::Exception("Unsupported image dimensions (%ux%u)", width, height);

	// Check that the width and height are really powers of 2
	if (!IsPower2(width) || !IsPower2(height))
		throw Common::Exception("Width and height must be powers of 2");

	// Check which compression
	uint32 bpp = dds.readUint32LE();
	if      (bpp == 3) {
		_format = kPixelFormatDXT1;
	} else if (bpp == 4) {
		_format = kPixelFormatDXT5;
	} else
		throw Common::Exception("Unsupported bytes per pixel value (%d)", bpp);

	// Sanity check for the image data size
	uint32 dataSize = dds.readUint32LE();
	if (((bpp == 3) && (dataSize != ((width * height) / 2))) ||
		  ((bpp == 4) && (dataSize != ((width * height)    ))))
		throw Common::Exception("Invalid data size (%dx%dx%d %d)", width, height, bpp, dataSize);

	if (!hasValidDimensions(_format, width, height))
		throw Common::Exception("Invalid dimensions (%dx%d) for format %d", width, height, _format);

	dds.skip(4); // Some float

	// Number of bytes left for the image data
	size_t fullDataSize = dds.size() - dds.pos();

	// Detect how many mip maps are in the DDS
	do {
		std::unique_ptr<MipMap> mipMap = std::make_unique<MipMap>();

		mipMap->width  = MAX<uint32>(width,  1);
		mipMap->height = MAX<uint32>(height, 1);

		setSize(*mipMap);

		// Wouldn't fit
		if (fullDataSize < mipMap->size)
			break;

		fullDataSize -= mipMap->size;

		_mipMaps.push_back(mipMap.release());

		width  >>= 1;
		height >>= 1;

	} while ((width >= 1) && (height >= 1));
}

void DDS::setSize(MipMap &mipMap) {
	// Depending on the pixel format, set the image data size in bytes

	mipMap.size = getDataSize(_format, mipMap.width, mipMap.height);
}

void DDS::readData(Common::SeekableReadStream &dds, DataType dataType) {
	for (MipMaps::iterator mipMap = _mipMaps.begin(); mipMap != _mipMaps.end(); ++mipMap) {
		(*mipMap)->data = std::make_unique<byte[]>((*mipMap)->size);

		if (dataType == kDataType4444) {

			byte *data = (*mipMap)->data.get();
			for (uint32 i = 0; i < (uint32)((*mipMap)->width * (*mipMap)->height); i++, data += 4) {
				const uint16 pixel = dds.readUint16LE();

				data[0] = ( pixel & 0x0000000F       ) << 4;
				data[1] = ((pixel & 0x000000F0) >>  4) << 4;
				data[2] = ((pixel & 0x00000F00) >>  8) << 4;
				data[3] = ((pixel & 0x0000F000) >> 12) << 4;
			}

		} else if (dataType == kDataTypeDirect)
			if (dds.read((*mipMap)->data.get(), (*mipMap)->size) != (*mipMap)->size)
				throw Common::Exception(Common::kReadError);

	}
}

void DDS::detectFormat(const DDSPixelFormat &format, DataType &dataType) {
	// Big, ugly big pixel format description => format mapping

	dataType = kDataTypeDirect;

	if        ((format.flags & kPixelFlagsHasFourCC) && (format.fourCC == kDXT1ID)) {
		_format = kPixelFormatDXT1;
	} else if ((format.flags & kPixelFlagsHasFourCC) && (format.fourCC == kDXT3ID)) {
		_format = kPixelFormatDXT3;
	} else if ((format.flags & kPixelFlagsHasFourCC) && (format.fourCC == kDXT5ID)) {
		_format = kPixelFormatDXT5;
	} else if ((format.flags & kPixelFlagsIsRGB) && (format.flags & kPixelFlagsHasAlpha) &&
	           (format.bitCount == 32) &&
	           (format.rBitMask == 0x00FF0000) && (format.gBitMask == 0x0000FF00) &&
	           (format.bBitMask == 0x000000FF) && (format.aBitMask == 0xFF000000)) {
		_format = kPixelFormatB8G8R8A8;
	} else if ((format.flags & kPixelFlagsIsRGB) && !(format.flags & kPixelFlagsHasAlpha) &&
	           (format.bitCount == 24) &&
	           (format.rBitMask == 0x00FF0000) && (format.gBitMask == 0x0000FF00) &&
	           (format.bBitMask == 0x000000FF)) {
		_format = kPixelFormatB8G8R8;

	} else if ((format.flags & kPixelFlagsIsRGB) && (format.flags & kPixelFlagsHasAlpha) &&
	           (format.bitCount == 16) &&
	           (format.rBitMask == 0x00007C00) && (format.gBitMask == 0x000003E0) &&
	           (format.bBitMask == 0x0000001F) && (format.aBitMask == 0x00008000)) {
		_format = kPixelFormatA1R5G5B5;

		warning("Found untested DDS RGB5A1 data");

	} else if ((format.flags & kPixelFlagsIsRGB) && !(format.flags & kPixelFlagsHasAlpha) &&
	           (format.bitCount == 16) &&
	           (format.rBitMask == 0x0000F800) && (format.gBitMask == 0x000007E0) &&
	           (format.bBitMask == 0x0000001F)) {
		_format = kPixelFormatR5G6B5;

		warning("Found untested DDS RGB5 data");

	} else if ((format.flags & kPixelFlagsIsRGB) && (format.flags & kPixelFlagsHasAlpha) &&
	           (format.bitCount == 16) &&
	           (format.rBitMask == 0x00000F00) && (format.gBitMask == 0x000000F0) &&
	           (format.bBitMask == 0x0000000F) && (format.aBitMask == 0x0000F000)) {
		_format = kPixelFormatB8G8R8A8;

		dataType = kDataType4444;

	} else if (format.flags & kPixelFlagsIsIndexed)
		// Hopefully, we'll never need to support that :P
		throw Common::Exception("Unsupported feature: Palette");
	else
		// We'll see if there's more formats in the data files :P
		throw Common::Exception("Unknown pixel format (%X, %X, %d, %X, %X, %X, %X)",
				format.flags, format.fourCC, format.bitCount,
				format.rBitMask, format.gBitMask, format.bBitMask, format.aBitMask);
}

} // End of namespace Images
