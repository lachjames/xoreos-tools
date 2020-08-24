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
 *  XML parsing helpers, using libxml2.
 */

#ifndef XML_XMLPARSER_H
#define XML_XMLPARSER_H

#include <list>
#include <map>
#include <memory>

#include <boost/noncopyable.hpp>

#include "src/common/ustring.h"

struct _xmlNode;

namespace Common {
	class ReadStream;
}

namespace XML {

class XMLNode;

/** Class to parse a ReadStream into a simple XML tree. */
class XMLParser : boost::noncopyable {
public:
	/** Parse an XML file out of a stream.
	 *
	 *  @param stream The stream to read the XML from.
	 *  @param makeLower Should all tags be converted to lowercase, to ease case-insensitive comparison?
	 *  @param fileName The file name to tell libxml2. Only used for error reporting.
	 */
	XMLParser(Common::ReadStream &stream, bool makeLower = false,
	          const Common::UString &fileName = "stream.xml");
	~XMLParser();

	/** Return the XML root node. */
	const XMLNode &getRoot() const;

private:
	std::unique_ptr<XMLNode> _rootNode;
};

class XMLNode : boost::noncopyable {
public:
	~XMLNode();

	typedef std::map<Common::UString, Common::UString> Properties;
	typedef std::list<std::unique_ptr<const XMLNode>> Children;

	const Common::UString &getName() const;
	const Common::UString &getContent() const;

	/** Return the parent node, or 0 if this is the root node. */
	const XMLNode *getParent() const;

	/** Return a list of children. */
	const Children &getChildren() const;

	/** Find a child node by name. */
	const XMLNode *findChild(const Common::UString &name) const;

	/** Return all the properties on this node. */
	const Properties &getProperties() const;
	/** Return a certain property on this node. */
	Common::UString getProperty(const Common::UString &name, const Common::UString &def = "") const;


private:
	typedef std::map<Common::UString, const XMLNode *, Common::UString::iless> ChildMap;

	Common::UString _name;
	Common::UString _content;

	XMLNode *_parent;

	Children _children;
	ChildMap _childMap;

	Properties _properties;


	XMLNode(_xmlNode &node, bool makeLower = false, XMLNode *parent = 0);

	void load(_xmlNode &node, bool makeLower);


	friend class XMLParser;
};

} // End of namespace XML

#endif // XML_XMLPARSER_H
