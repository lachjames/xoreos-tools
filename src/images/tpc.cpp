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
 *  TPC (BioWare's own texture format) loading.
 */

#include <cassert>
#include <cstring>

#include "src/common/util.h"
#include "src/common/maths.h"
#include "src/common/error.h"
#include "src/common/memreadstream.h"

#include "src/images/tpc.h"
#include "src/images/txi.h"
#include "src/images/util.h"

static const byte kEncodingGray         = 0x01;
static const byte kEncodingRGB          = 0x02;
static const byte kEncodingRGBA         = 0x04;
static const byte kEncodingSwizzledBGRA = 0x0C;

namespace Images {

TPC::TPC(Common::SeekableReadStream &tpc) : _txiDataSize(0) {
	load(tpc);
}

TPC::~TPC() {
}

void TPC::load(Common::SeekableReadStream &tpc) {
	try {

		byte encoding;

		readHeader (tpc, encoding);
		readData   (tpc, encoding);

		fixupCubeMap();

	} catch (Common::Exception &e) {
		e.add("Failed reading TPC file");
		throw;
	}

	// In xoreos-tools, we always want decompressed images
	decompress();
}

Common::SeekableReadStream *TPC::getTXI() const {
	if (!_txiData || (_txiDataSize == 0))
		return 0;

	return new Common::MemoryReadStream(_txiData.get(), _txiDataSize);
}

void TPC::readHeader(Common::SeekableReadStream &tpc, byte &encoding) {
	// Number of bytes for the pixel data in one full image
	uint32_t dataSize = tpc.readUint32LE();
	bool isUncompressed = dataSize == 0;

	tpc.skip(4); // Some float

	// Image dimensions
	uint32_t width  = tpc.readUint16LE();
	uint32_t height = tpc.readUint16LE();

	if ((width >= 0x8000) || (height >= 0x8000))
		throw Common::Exception("Unsupported image dimensions (%ux%u)", width, height);

	// How's the pixel data encoded?
	encoding = tpc.readByte();

	// Number of mip maps in the image
	size_t mipMapCount = tpc.readByte();

	// If uncompressed, calculate dataSize
	if (isUncompressed) {
		switch (encoding) {
			case kEncodingGray:
				dataSize = width * height;
				break;
			case kEncodingRGB:
				dataSize = width * height * 3;
				break;
			case kEncodingRGBA:
			case kEncodingSwizzledBGRA:
				dataSize = width * height * 4;
		}
	} else {
		checkCubeMap(width, height);
	}

	uint32_t minDataSize = getMinDataSize(isUncompressed, encoding);
	_format = getPixelFormat(isUncompressed, encoding);

	// Calculate the complete data size for images with mipmaps
	size_t completeDataSize = dataSize;
	uint32_t w = width, h = height;
	for (size_t i = 1; i < mipMapCount; ++i) {
		w >>= 1;
		h >>= 1;

		w = MAX<uint32_t>(w, 1);
		h = MAX<uint32_t>(h, 1);

		completeDataSize += getDataSize(_format, w, h);
	}

	completeDataSize *= _layerCount;

	tpc.skip(114); // Reserved
	tpc.skip(completeDataSize);
	readTXIData(tpc);

	_isAnimated = checkAnimated(width, height, dataSize);

	if (_isAnimated) {
		w = width;
		h = height;
		mipMapCount = 0;

		while (w > 0 && h > 0) {
			w /= 2;
			h /= 2;
			mipMapCount++;
		}
	}

	if (!isUncompressed) {
		if (encoding == kEncodingRGB) {
			// S3TC DXT1

			if (dataSize != ((width * height) / 2) && !_isAnimated)
				throw Common::Exception("Invalid data size for a texture of %ux%u pixels and format %u",
				                        width, height, encoding);

		} else if (encoding == kEncodingRGBA) {
			// S3TC DXT5

			if (dataSize != (width * height) && !_isAnimated)
				throw Common::Exception("Invalid data size for a texture of %ux%u pixels and format %u",
				                        width, height, encoding);

		} else
			throw Common::Exception("Unknown TPC encoding: %d (%d)", encoding, dataSize);
	}

	tpc.seek(128);

	if (!hasValidDimensions(_format, width, height))
		throw Common::Exception("Invalid dimensions (%dx%d) for format %d", width, height, _format);

	const size_t fullImageDataSize = getDataSize(_format, width, height);

	size_t fullDataSize = tpc.size() - 128;
	if (fullDataSize < (_layerCount * fullImageDataSize))
		throw Common::Exception("Image wouldn't fit into data");

	_mipMaps.reserve(mipMapCount);

	size_t layerCount;
	uint combinedSize = 0;
	for (layerCount = 0; layerCount < _layerCount; layerCount++) {
		uint32_t layerWidth  = width;
		uint32_t layerHeight = height;

		uint32_t layerSize;
		if (_isAnimated)
			layerSize = getDataSize(_format, layerWidth, layerHeight);
		else
			layerSize = dataSize;

		for (size_t i = 0; i < mipMapCount; i++) {
			std::unique_ptr<MipMap> mipMap = std::make_unique<MipMap>();

			mipMap->width  = MAX<uint32_t>(layerWidth,  1);
			mipMap->height = MAX<uint32_t>(layerHeight, 1);

			mipMap->size = MAX<uint32_t>(layerSize, minDataSize);

			const size_t mipMapDataSize = getDataSize(_format, mipMap->width, mipMap->height);

			// Wouldn't fit
			if ((fullDataSize < mipMap->size) || (mipMap->size < mipMapDataSize))
				break;

			fullDataSize -= mipMap->size;
			combinedSize += mipMap->size;

			_mipMaps.emplace_back(mipMap.release());

			layerWidth  >>= 1;
			layerHeight >>= 1;
			layerSize = getDataSize(_format, layerWidth, layerHeight);

			if ((layerWidth < 1) && (layerHeight < 1))
				break;
		}
	}

	if ((layerCount != _layerCount) || ((_mipMaps.size() % _layerCount) != 0))
		throw Common::Exception("Failed to correctly read all texture layers (%u, %u, %u, %u)",
		                        (uint) _layerCount, (uint) mipMapCount,
		                        (uint) layerCount, (uint) _mipMaps.size());
}

bool TPC::checkCubeMap(uint32_t &width, uint32_t &height) {
	/* Check if this texture is a cube map by looking if height equals to six
	 * times width. This means that there are 6 sides of width * (height / 6)
	 * images in this texture, making it a cube map.
	 *
	 * The individual sides are then stores on after another, together with
	 * their mip maps.
	 *
	 * I.e.
	 * - Side 0, mip map 0
	 * - Side 0, mip map 1
	 * - ...
	 * - Side 1, mip map 0
	 * - Side 1, mip map 1
	 * - ...
	 *
	 * The ordering of the sides should be the usual Direct3D cube map order,
	 * which is the same as the OpenGL cube map order.
	 *
	 * Yes, that's a really hacky way to encode a cube map. But this is how
	 * the original game does it. It works and doesn't clash with other, normal
	 * textures because TPC textures always have power-of-two side lengths,
	 * and therefore (height / width) == 6 isn't true for non-cubemaps.
	 */

	if ((height == 0) || (width == 0) || ((height / width) != 6))
		return false;

	height /= 6;


	_layerCount = 6;
	_isCubeMap  = true;

	return true;
}

bool TPC::checkAnimated(uint32_t &width, uint32_t &height, uint32_t &dataSize) {
	std::unique_ptr<Common::SeekableReadStream> txiStream(getTXI());

	// If there is no TXI data, it cannot be animated
	if (!txiStream)
		return false;

	TXI txi(*txiStream);

	if (txi.getFeatures().procedureType != "cycle" ||
	    txi.getFeatures().numX == 0 ||
	    txi.getFeatures().numY == 0 ||
	    txi.getFeatures().fps == 0.0f) {

		return false;
	}

	_layerCount = txi.getFeatures().numX * txi.getFeatures().numY;

	width  /= txi.getFeatures().numX;
	height /= txi.getFeatures().numY;

	dataSize /= _layerCount;

	return true;
}

void TPC::deSwizzle(byte *dst, const byte *src, uint32_t width, uint32_t height) {
	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			const uint32_t offset = deSwizzleOffset(x, y, width, height) * 4;

			*dst++ = src[offset + 0];
			*dst++ = src[offset + 1];
			*dst++ = src[offset + 2];
			*dst++ = src[offset + 3];
		}
	}
}

void TPC::readData(Common::SeekableReadStream &tpc, byte encoding) {
	for (MipMaps::iterator mipMap = _mipMaps.begin(); mipMap != _mipMaps.end(); ++mipMap) {

		// If the texture width is a power of two, the texture memory layout is "swizzled"
		const bool widthPOT = ((*mipMap)->width & ((*mipMap)->width - 1)) == 0;
		const bool swizzled = (encoding == kEncodingSwizzledBGRA) && widthPOT;

		(*mipMap)->data = std::make_unique<byte[]>((*mipMap)->size);

		if (swizzled) {
			std::vector<byte> tmp((*mipMap)->size);

			if (tpc.read(&tmp[0], (*mipMap)->size) != (*mipMap)->size)
				throw Common::Exception(Common::kReadError);

			deSwizzle((*mipMap)->data.get(), &tmp[0], (*mipMap)->width, (*mipMap)->height);

		} else {
			if (tpc.read((*mipMap)->data.get(), (*mipMap)->size) != (*mipMap)->size)
				throw Common::Exception(Common::kReadError);

			// Unpacking 8bpp grayscale data into RGB
			if (encoding == kEncodingGray) {
				std::unique_ptr<byte[]> dataGray((*mipMap)->data.release());

				(*mipMap)->size = (*mipMap)->width * (*mipMap)->height * 3;
				(*mipMap)->data = std::make_unique<byte[]>((*mipMap)->size);

				for (int i = 0; i < ((*mipMap)->width * (*mipMap)->height); i++)
					std::memset((*mipMap)->data.get() + i * 3, dataGray[i], 3);
			}
		}

	}
}

void TPC::readTXIData(Common::SeekableReadStream &tpc) {
	// TXI data for the rest of the TPC
	_txiDataSize = tpc.size() - tpc.pos();

	if (_txiDataSize == 0)
		return;

	_txiData = std::make_unique<byte[]>(_txiDataSize);

	if (tpc.read(_txiData.get(), _txiDataSize) != _txiDataSize)
		throw Common::Exception(Common::kReadError);
}

uint32_t TPC::getMinDataSize(bool uncompressed, byte encoding) {
	if (uncompressed) {
		switch (encoding) {
			case kEncodingGray:
				return 1;
			case kEncodingRGB:
				return 3;
			case kEncodingRGBA:
			case kEncodingSwizzledBGRA:
				return 4;
		}
	} else {
		switch (encoding) {
			case kEncodingRGB:
				return 8;
			case kEncodingRGBA:
				return 16;
		}
	}

	throw Common::Exception("Unknown TPC encoding: %d", encoding);
}

PixelFormat TPC::getPixelFormat(bool uncompressed, byte encoding) {
	if (uncompressed) {
		switch (encoding) {
			case kEncodingGray:
			case kEncodingRGB:
				return kPixelFormatR8G8B8;
			case kEncodingRGBA:
				return kPixelFormatR8G8B8A8;
			case kEncodingSwizzledBGRA:
				return kPixelFormatB8G8R8A8;
		}
	} else {
		switch (encoding) {
			case kEncodingRGB:
				return kPixelFormatDXT1;
			case kEncodingRGBA:
				return kPixelFormatDXT5;
		}
	}

	throw Common::Exception("Unknown TPC encoding: %d", encoding);
}

void TPC::fixupCubeMap() {
	/* Do various fixups to the cube maps. This includes rotating and swapping a
	 * few sides around. This is done by the original games as well.
	 */

	if (!isCubeMap())
		return;

	for (size_t j = 0; j < getMipMapCount(); j++) {
		assert(getLayerCount() > 0);

		const size_t index0 = 0 * getMipMapCount() + j;
		assert(index0 < _mipMaps.size());

		const  int32_t width  = _mipMaps[index0]->width;
		const  int32_t height = _mipMaps[index0]->height;
		const uint32_t size   = _mipMaps[index0]->size;

		for (size_t i = 1; i < getLayerCount(); i++) {
			const size_t index = i * getMipMapCount() + j;
			assert(index < _mipMaps.size());

			if ((width  != _mipMaps[index]->width ) ||
			    (height != _mipMaps[index]->height) ||
			    (size   != _mipMaps[index]->size  ))
				throw Common::Exception("Cube map layer dimensions mismatch");
		}
	}

	// Since we need to rotate the individual cube sides, we need to decompress them all
	decompress();

	// Rotate the cube sides so that they're all oriented correctly
	for (size_t i = 0; i < getLayerCount(); i++) {
		for (size_t j = 0; j < getMipMapCount(); j++) {
			const size_t index = i * getMipMapCount() + j;
			assert(index < _mipMaps.size());

			MipMap &mipMap = *_mipMaps[index];

			static const int rotation[6] = { 3, 1, 0, 2, 2, 0 };

			rotate90(mipMap.data.get(), mipMap.width, mipMap.height, getBPP(_format), rotation[i]);
		}
	}

	// Swap the first two sides of the cube maps
	for (size_t j = 0; j < getMipMapCount(); j++) {
		const size_t index0 = 0 * getMipMapCount() + j;
		const size_t index1 = 1 * getMipMapCount() + j;
		assert((index0 < _mipMaps.size()) && (index1 < _mipMaps.size()));

		MipMap &mipMap0 = *_mipMaps[index0];
		MipMap &mipMap1 = *_mipMaps[index1];

		mipMap0.data.swap(mipMap1.data);
	}
}

} // End of namespace Images
