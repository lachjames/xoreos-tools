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
 *  Unit tests for our RIM file archive writer class.
 */

#include "gtest/gtest.h"

#include "src/common/memwritestream.h"

#include "src/aurora/rimwriter.h"
#include "src/aurora/rimfile.h"

// Percy Bysshe Shelley's "Ozymandias"
static const char *kFileData =
		"I met a traveller from an antique land\n"
		"Who said: Two vast and trunkless legs of stone\n"
		"Stand in the desert. Near them, on the sand,\n"
		"Half sunk, a shattered visage lies, whose frown,\n"
		"And wrinkled lip, and sneer of cold command,\n"
		"Tell that its sculptor well those passions read\n"
		"Which yet survive, stamped on these lifeless things,\n"
		"The hand that mocked them and the heart that fed:\n"
		"And on the pedestal these words appear:\n"
		"'My name is Ozymandias, king of kings:\n"
		"Look on my works, ye Mighty, and despair!'\n"
		"Nothing beside remains. Round the decay\n"
		"Of that colossal wreck, boundless and bare\n"
		"The lone and level sands stretch far away.";

// The xoreos logo as 32x32 bitmap
unsigned char kLogoData[] = {
		0x42, 0x4d, 0x8a, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x00,
		0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00,
		0x00, 0x00, 0x01, 0x00, 0x20, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10,
		0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
		0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x42, 0x47,
		0x52, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x28, 0x28, 0x28, 0x71, 0x28,
		0x28, 0x28, 0x6c, 0x28, 0x28, 0x28, 0x6c, 0x28, 0x28, 0x28, 0x5d, 0x28,
		0x28, 0x28, 0x35, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x28, 0x28, 0x28, 0x28, 0x54, 0x28, 0x28, 0x28, 0x6a, 0x28,
		0x28, 0x28, 0x6c, 0x28, 0x28, 0x28, 0x6d, 0x28, 0x28, 0x28, 0x2f, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x27, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xbb, 0x28, 0x28, 0x28, 0x43, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x20, 0x28,
		0x28, 0x28, 0xa1, 0x28, 0x28, 0x28, 0xfe, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0x70, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xcc, 0x28,
		0x28, 0x28, 0x13, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0x9c, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0x6f, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0x3d, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28,
		0x28, 0x28, 0x0d, 0x28, 0x28, 0x28, 0xe4, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0x6f, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x27, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0x3e, 0x28, 0x28, 0x28, 0x08, 0x28, 0x28, 0x28, 0xf0, 0x28,
		0x28, 0x28, 0xfe, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0x6f, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x28,
		0x28, 0x28, 0x76, 0x28, 0x28, 0x28, 0x71, 0x28, 0x28, 0x28, 0x73, 0x28,
		0x28, 0x28, 0x8c, 0x28, 0x28, 0x28, 0xd0, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xfd, 0x28,
		0x28, 0x28, 0xd2, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xe9, 0x28, 0x28, 0x28, 0x9b, 0x28,
		0x28, 0x28, 0x77, 0x28, 0x28, 0x28, 0x71, 0x28, 0x28, 0x28, 0x72, 0x28,
		0x28, 0x28, 0x31, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x29, 0x29, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x28, 0x28, 0x28, 0xcf, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xef, 0x28, 0x28, 0x28, 0x45, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2a,
		0x2a, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0xa3, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xd1, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2a, 0x2a, 0x2a, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0xca, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xf5, 0x28, 0x28, 0x28, 0x01, 0x28, 0x28, 0x28, 0x00, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x28, 0x28, 0x28, 0x1a, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0x57, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc1, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xfc, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x72, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xb7, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4f, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0x97, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0x9a, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x7e, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xc2, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdc, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28,
		0x28, 0x28, 0x3c, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0x7e, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x28, 0x28, 0x28, 0x03, 0x28, 0x28, 0x28, 0xef, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0x24, 0x28, 0x28, 0x28, 0x00, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x28,
		0x28, 0x28, 0xe1, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xfb, 0x28, 0x28, 0x28, 0x29, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x29,
		0x29, 0x29, 0x74, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0x9a, 0x28, 0x28, 0x28, 0x18, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x28,
		0x28, 0x28, 0xbe, 0x28, 0x28, 0x28, 0xb7, 0x28, 0x28, 0x28, 0xb9, 0x28,
		0x28, 0x28, 0xd5, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xe7, 0x28,
		0x28, 0x28, 0xa1, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xe6, 0x28,
		0x28, 0x28, 0xbd, 0x28, 0x28, 0x28, 0xb6, 0x28, 0x28, 0x28, 0xb8, 0x28,
		0x28, 0x28, 0x4f, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xf6, 0x28, 0x28, 0x28, 0x14, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0xc3, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0x6f, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x27, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xdc, 0x28, 0x28, 0x28, 0x0e, 0x28,
		0x28, 0x28, 0x01, 0x28, 0x28, 0x28, 0x00, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0xa3, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0x6f, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0x79, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x29, 0x29, 0x00, 0x00,
		0x00, 0x00, 0x48, 0x28, 0x28, 0x28, 0xed, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0x6f, 0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xca, 0x28, 0x28, 0x28, 0x6f, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
		0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x51, 0x28, 0x28, 0x28, 0xb6, 0x28,
		0x28, 0x28, 0xfd, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28,
		0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0xff, 0x28, 0x28, 0x28, 0x73, 0x28,
		0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x06, 0x28, 0x28, 0x28, 0x29, 0x28, 0x28, 0x28, 0x27, 0x28,
		0x28, 0x28, 0x25, 0x28, 0x28, 0x28, 0x15, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x26, 0x26, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x0c, 0x28, 0x28, 0x28, 0x22, 0x28, 0x28, 0x28, 0x27, 0x28,
		0x28, 0x28, 0x27, 0x28, 0x28, 0x28, 0x11, 0x28, 0x28, 0x28, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

GTEST_TEST(RIMWriter, WriteEmpty) {
	Common::MemoryWriteStreamDynamic writeStream;
	Aurora::RIMWriter rimWriter(0, writeStream);

	const Aurora::RIMFile rim(new Common::MemoryReadStream(writeStream.getData(), writeStream.size(), true));

	EXPECT_EQ(rim.getID(), MKTAG('R', 'I', 'M', ' '));
	EXPECT_EQ(rim.getResources().size(), 0);
}

GTEST_TEST(RIMWriter, WriteFile) {
	Common::MemoryReadStream dataStream(kFileData, true);
	const size_t kFileDataSize = dataStream.size();

	Common::MemoryWriteStreamDynamic writeStream;
	Aurora::RIMWriter rimWriter(1, writeStream);
	rimWriter.add("ozymandias", Aurora::kFileTypeTXT, dataStream);

	const Aurora::RIMFile rim(new Common::MemoryReadStream(writeStream.getData(), writeStream.size(), true));

	EXPECT_EQ(rim.getID(), MKTAG('R', 'I', 'M', ' '));
	EXPECT_EQ(rim.getResources().size(), 1);

	EXPECT_EQ(rim.findResource("ozymandias", Aurora::kFileTypeTXT), 0);
	EXPECT_EQ(rim.findResource("ozymandias", Aurora::kFileTypeBMP), 0xFFFFFFFF);

	Common::SeekableReadStream *readStream = rim.getResource(0);
	ASSERT_EQ(readStream->size(), kFileDataSize);

	std::unique_ptr<byte[]> fileData = std::make_unique<byte[]>(readStream->size());
	readStream->read(fileData.get(), readStream->size());

	for (size_t i = 0; i < kFileDataSize; ++i) {
		EXPECT_EQ(fileData[i], kFileData[i]);
	}

	delete readStream;
}

GTEST_TEST(RIMWriter, WriteMultipleFiles) {
	Common::MemoryReadStream dataStream1(kFileData, true);
	const size_t kFileDataSize = dataStream1.size();

	const size_t kLogoDataSize = sizeof(kLogoData);
	Common::MemoryReadStream dataStream2(kLogoData, kLogoDataSize);

	Common::MemoryWriteStreamDynamic writeStream;
	Aurora::RIMWriter rimWriter(3, writeStream);
	rimWriter.add("ozymandias_1", Aurora::kFileTypeTXT, dataStream1);
	dataStream1.seek(0);
	rimWriter.add("ozymandias_2", Aurora::kFileTypeTXT, dataStream1);
	rimWriter.add("logo", Aurora::kFileTypeBMP, dataStream2);

	const Aurora::RIMFile rim(new Common::MemoryReadStream(writeStream.getData(), writeStream.size(), true));

	EXPECT_EQ(rim.getID(), MKTAG('R', 'I', 'M', ' '));
	EXPECT_EQ(rim.getResources().size(), 3);

	EXPECT_EQ(rim.findResource("ozymandias_1", Aurora::kFileTypeTXT), 0);
	EXPECT_EQ(rim.findResource("ozymandias_1", Aurora::kFileTypeBMP), 0xFFFFFFFF);
	EXPECT_EQ(rim.findResource("ozymandias_2", Aurora::kFileTypeTXT), 1);
	EXPECT_EQ(rim.findResource("ozymandias_2", Aurora::kFileTypeBMP), 0xFFFFFFFF);
	EXPECT_EQ(rim.findResource("logo", Aurora::kFileTypeBMP), 2);
	EXPECT_EQ(rim.findResource("logo", Aurora::kFileTypeTXT), 0xFFFFFFFF);

	Common::SeekableReadStream *readStream1 = rim.getResource(0);
	ASSERT_EQ(readStream1->size(), kFileDataSize);
	Common::SeekableReadStream *readStream2 = rim.getResource(1);
	ASSERT_EQ(readStream2->size(), kFileDataSize);
	Common::SeekableReadStream *readStream3 = rim.getResource(2);
	ASSERT_EQ(readStream3->size(), kLogoDataSize);

	std::unique_ptr<byte[]> fileData1 = std::make_unique<byte[]>(readStream1->size());
	readStream1->read(fileData1.get(), readStream1->size());
	std::unique_ptr<byte[]> fileData2 = std::make_unique<byte[]>(readStream2->size());
	readStream2->read(fileData2.get(), readStream2->size());
	std::unique_ptr<byte[]> fileData3 = std::make_unique<byte[]>(readStream3->size());
	readStream3->read(fileData3.get(), readStream3->size());

	for (size_t i = 0; i < kFileDataSize; ++i) {
		EXPECT_EQ(fileData1[i], kFileData[i]);
	}
	for (size_t i = 0; i < kFileDataSize; ++i) {
		EXPECT_EQ(fileData2[i], kFileData[i]);
	}
	for (size_t i = 0; i < kLogoDataSize; ++i) {
		EXPECT_EQ(fileData3[i], kLogoData[i]);
	}

	delete readStream1;
	delete readStream2;
	delete readStream3;
}
