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
 *  Handling BioWare's GFF'd talk tables.
 */

#ifndef AURORA_TALKTABLE_GFF_H
#define AURORA_TALKTABLE_GFF_H

#include <map>
#include <memory>

#include "src/common/types.h"
#include "src/common/ustring.h"

#include "src/aurora/types.h"
#include "src/aurora/talktable.h"

namespace Common {
	class SeekableReadStream;
	class SeekableSubReadStreamEndian;
}

namespace Aurora {

/** Loading BioWare's GFF'd talk tables.
 *
 *  See class TalkTable for a general overview how talk tables work.
 *
 *  Unlike TalkTable_TLK, a GFF talk table stores the string data
 *  within a V4.0 GFF. It does not store any language ID (the language
 *  is implicit in the talk tables file name), nor any other data
 *  besides the raw strings. getSoundResRef() always returns an empty
 *  string.
 *
 *  There are three versions of GFF'd talk tables known and supported
 *  - V0.2, used by Sonic Chronicles and Dragon Age: Origins (PC)
 *  - V0.4, used by Dragon Age: Origins (Xbox 360)
 *  - V0.5, used by Dragon Age II
 */
class TalkTable_GFF : public TalkTable {
public:
	/** Take over this stream and read a GFF'd TLK out of it. */
	TalkTable_GFF(Common::SeekableReadStream *tlk, Common::Encoding encoding);
	~TalkTable_GFF();

	std::list<uint32_t> getStrRefs() const;
	bool getString(uint32_t strRef, Common::UString &string, Common::UString &soundResRef) const;

	bool getEntry(uint32_t strRef, Common::UString &string, Common::UString &soundResRef,
	              uint32_t &volumeVariance, uint32_t &pitchVariance, float &soundLength,
	              uint32_t &soundID) const;

	void setEntry(uint32_t strRef, const Common::UString &string, const Common::UString &soundResRef,
	              uint32_t volumeVariance, uint32_t pitchVariance, float soundLength,
	              uint32_t soundID);


private:
	struct Entry {
		Common::UString text;

		const GFF4Struct *strct { nullptr };

		Entry() = default;
		Entry(const GFF4Struct *s) : strct(s) { }
		Entry(const Common::UString &t) : text(t) { }
	};

	typedef std::map<uint32_t, std::unique_ptr<Entry>> Entries;


	std::unique_ptr<GFF4File> _gff;

	Entries _entries;

	void load(Common::SeekableReadStream *tlk);
	void load02(const GFF4Struct &top);
	void load05(const GFF4Struct &top);

	Common::UString readString(const Entry &entry) const;
	Common::UString readString02(const Entry &entry) const;
	Common::UString readString05(const Entry &entry, bool bigEndian) const;

	Common::UString readString05(Common::SeekableSubReadStreamEndian &huffTree,
	                             Common::SeekableSubReadStreamEndian &bitStream,
	                             const Entry &entry) const;
};

} // End of namespace Aurora

#endif // AURORA_TALKTABLE_GFF_H
