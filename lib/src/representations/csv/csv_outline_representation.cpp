/*
 csv_outline_representation.cpp     MindForger thinking notebook

 Copyright (C) 2016-2021 Martin Dvorak <martin.dvorak@mindforger.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "csv_outline_representation.h"

using namespace std;
using namespace m8r::filesystem;

namespace m8r {

CsvOutlineRepresentation::CsvOutlineRepresentation()
{
}

CsvOutlineRepresentation::~CsvOutlineRepresentation()
{
}

/**
 * @brief Serialize O to CSV in "Recent view" style
 *
 * O is serialized as N descriptor, only shared fields are serialized to avoid sparse
 * lines
 */
void CsvOutlineRepresentation::to(const vector<Outline*>& os, const File& sourceFile)
{
    MF_DEBUG("Exporting MIND to CSV " << sourceFile.getName() << endl);

    if(sourceFile.getName().size()) {
        if(os.size()) {
            std::ofstream out{};
            try {
                out.open(sourceFile.getName());

                toHeader(out);
                for(Outline* o:os) {
                    to(o, out);
                }
            } catch (const std::ofstream::failure& e) {
                cerr << "Error: unable to open/write file " << sourceFile.getName() << " " << e.what();
            }
            out.flush();
            out.close();

            MF_DEBUG("FINISHED export of MIND to CSV " << sourceFile.getName() << endl);
        }
    } else {
        cerr << "Error: target file name is empty";
    }
}

void CsvOutlineRepresentation::toHeader(std::ofstream& out)
{
    // O/N CSV line
    // id,     type, title, offset, depth, reads, writes, created, modified, read, description
    // string, o/n,  int,   int,    int,   int,   int,    long,    long,     long, string

    // TODO tags|tags
    out << "id,type,title,offset,depth,reads,writes,created,modified,read,description\n";
}

void CsvOutlineRepresentation::to(const Outline* o, ofstream& out)
{
    MF_DEBUG("\n  " << o->getName());

    string s{};

    // O
    out << o->getKey() << ",";
    out << "o,";
    s.clear(); quoteValue(o->getName(), s);
    out << s << ",";
    // O's offset and depth == 0
    out << "0,";
    out << "0,";
    out << o->getReads() << ",";
    out << o->getRevision() << ",";
    out << o->getCreated() << ",";
    out << o->getModified() << ",";
    out << o->getRead() << ",";
    s.clear(); quoteValue(o->getDescriptionAsString(" "), s);
    out << s;
    out << "\n";

    // Ns
    const vector<Note*>& ns = o->getNotes();
    int offset = 1;
    for(Note* n:ns) {
        MF_DEBUG("    " << n->getName());
        out << n->getKey() << ",";
        out << "n,";
        s.clear(); quoteValue(n->getName(), s);
        out << s << ",";
        // N's offset: <1,inf>
        out << offset++ << ",";
        // N's depth: <1,inf>
        out << (n->getDepth()+1) << ",";
        out << n->getReads() << ",";
        out << n->getRevision() << ",";
        out << n->getCreated() << ",";
        out << n->getModified() << ",";
        out << n->getRead() << ",";
        MF_DEBUG(" B ");
        s.clear(); quoteValue(n->getDescriptionAsString(" "), s);
        MF_DEBUG(" F ");
        out << s;
        out << "\n";
        MF_DEBUG(" ... DONE" << endl);
        out.flush();
    }
}

void CsvOutlineRepresentation::quoteValue(const std::string& is, std::string& os)
{
    if(is.size()) {
        os.append(" ");
        os.append(is);

        replaceAll("\"", "\"\"", os);

        os[0] = '\"';
        os.append("\"");
    }
}

} // m8r namespace
