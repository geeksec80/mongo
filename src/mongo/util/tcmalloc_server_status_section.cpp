/*    Copyright 2013 10gen Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects
 *    for all of the code used other than as permitted herein. If you modify
 *    file(s) with this exception, you may extend this exception to your
 *    version of the file(s), but you are not obligated to do so. If you do not
 *    wish to do so, delete this exception statement from your version. If you
 *    delete this exception statement from all source files in the program,
 *    then also delete it in the license file.
 */

#include "mongo/platform/basic.h"

#include <third_party/gperftools-2.2/src/gperftools/malloc_extension.h>

#include "mongo/db/commands/server_status.h"

namespace mongo {
namespace {
    class TCMallocServerStatusSection : public ServerStatusSection {
    public:

        TCMallocServerStatusSection() : ServerStatusSection("tcmalloc") {}
        virtual bool includeByDefault() const { return false; }
        
        virtual BSONObj generateSection(OperationContext* txn,
                                        const BSONElement& configElement) const {

            BSONObjBuilder builder;

            // For a list of properties see the "Generic Tcmalloc Status" section of
            // http://google-perftools.googlecode.com/svn/trunk/doc/tcmalloc.html and
            // http://code.google.com/p/gperftools/source/browse/src/gperftools/malloc_extension.h
            {
                BSONObjBuilder sub(builder.subobjStart("generic"));
                appendNumericPropertyIfAvailable(sub,    "current_allocated_bytes",
                                                 "generic.current_allocated_bytes");
                appendNumericPropertyIfAvailable(sub,    "heap_size",
                                                 "generic.heap_size");
            }
            {
                BSONObjBuilder sub(builder.subobjStart("tcmalloc"));
                appendNumericPropertyIfAvailable(sub,     "pageheap_free_bytes",
                                                 "tcmalloc.pageheap_free_bytes");
                appendNumericPropertyIfAvailable(sub,     "pageheap_unmapped_bytes",
                                                 "tcmalloc.pageheap_unmapped_bytes");
                appendNumericPropertyIfAvailable(sub,     "max_total_thread_cache_bytes",
                                                 "tcmalloc.max_total_thread_cache_bytes");
                appendNumericPropertyIfAvailable(sub,     "current_total_thread_cache_bytes",
                                                 "tcmalloc.current_total_thread_cache_bytes");
                // Not including tcmalloc.slack_bytes since it is deprecated.

                // These are not available in our version but are available with use-system-tcmalloc
                appendNumericPropertyIfAvailable(sub,     "central_cache_free_bytes",
                                                 "tcmalloc.central_cache_free_bytes");
                appendNumericPropertyIfAvailable(sub,     "transfer_cache_free_bytes",
                                                 "tcmalloc.transfer_cache_free_bytes");
                appendNumericPropertyIfAvailable(sub,     "thread_cache_free_bytes",
                                                 "tcmalloc.thread_cache_free_bytes");
            }

            char buffer[4096];
            MallocExtension::instance()->GetStats(buffer, sizeof(buffer));
            builder.append("formattedString", buffer);

            return builder.obj();
        }

    private:
        static void appendNumericPropertyIfAvailable(BSONObjBuilder& builder,
                                                     StringData bsonName,
                                                     const char* property) {
            size_t value;
            if (MallocExtension::instance()->GetNumericProperty(property, &value))
                builder.appendNumber(bsonName, value);
        }
    } tcmallocServerStatusSection;
}
}

