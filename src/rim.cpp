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
 *  Tool to pack RIM archives.
 */

#include <set>

#include "src/common/error.h"
#include "src/common/platform.h"
#include "src/common/cli.h"
#include "src/common/readfile.h"
#include "src/common/writefile.h"
#include "src/common/filepath.h"

#include "src/aurora/rimwriter.h"
#include "src/aurora/util.h"

#include "src/util.h"

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &archive, std::set<Common::UString> &files,
                      Aurora::GameID &game);

int main(int argc, char **argv) {
	initPlatform();

	try {
		std::vector<Common::UString> args;
		Common::Platform::getParameters(argc, argv, args);

		Aurora::GameID game = Aurora::kGameIDUnknown;

		int returnValue = 1;
		Common::UString archive;
		std::set<Common::UString> files;

		if (!parseCommandLine(args, returnValue, archive, files, game))
			return returnValue;

		for (const auto &file : files)
			if (file.equalsIgnoreCase(archive))
				throw Common::Exception("Trying to pack file \"%s\" into itself?!?", file.c_str());

		Common::WriteFile writeFile(archive);

		size_t i = 1;
		Aurora::RIMWriter rimWriter(files.size(), writeFile);
		for (std::set<Common::UString>::const_iterator iter = files.begin(); iter != files.end(); ++iter, ++i) {
			std::printf("Packing %u/%u: %s ... ", (uint)i, (uint)files.size(), iter->c_str());
			std::fflush(stdout);

			Common::UString file = *iter;
			Common::ReadFile fileStream(file);

			const Aurora::FileType type = TypeMan.unaliasFileType(TypeMan.getFileType(file), game);

			rimWriter.add(Common::FilePath::getStem(file), type, fileStream);
			std::printf("Done\n");
		}
	} catch (...) {
		Common::exceptionDispatcherError();
	}

	return 0;
}

bool parseCommandLine(const std::vector<Common::UString> &argv, int &returnValue,
                      Common::UString &archive, std::set<Common::UString> &files,
                      Aurora::GameID &game) {
	using Common::CLI::NoOption;
	using Common::CLI::kContinueParsing;
	using Common::CLI::Parser;
	using Common::CLI::Callback;
	using Common::CLI::ValGetter;
	using Common::CLI::ValAssigner;
	using Common::CLI::makeEndArgs;
	using Common::CLI::makeAssigners;
	using Aurora::GameID;

	NoOption archiveOpt(false, new ValGetter<Common::UString &>(archive, "output archive"));
	NoOption filesOpt(true, new ValGetter<std::set<Common::UString> &>(files, "files[...]"));

	Parser parser(argv[0], "BioWare RIM archive packer",
	              "",
	              returnValue,
	              makeEndArgs(&archiveOpt, &filesOpt));

	parser.addSpace();
	parser.addOption("jade", "Unalias file types according to Jade Empire rules",
	                 kContinueParsing,
	                 makeAssigners(new ValAssigner<GameID>(Aurora::kGameIDJade, game)));

	return parser.process(argv);
}
