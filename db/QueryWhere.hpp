/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU  General  Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _QUERY_WHERE_HPP_
#define _QUERY_WHERE_HPP_

#include <string>
#include <stack>

#include <boost/cstdint.hpp>

#include "db/Connection.hpp"
#include "db/Query.hpp"

namespace Database {
class QueryWhere : public virtual Query {
private:
    std::stack<std::string *> conditionsStack;
    std::string conditions;

public:
    template<typename T> void addEqualCondition(const std::string &column, const T &value) {
        addEqualCondition<T>("", column, value);
    };

    template<typename T> void addEqualCondition(const std::string &table, const std::string &column, const T &value) {
        conditionsStack.push(new std::string(escapeAndChainKeys(table, column) + " = " + quote<T>(value)));
    };

    template<typename T> void addNotEqualCondition(const std::string &column, const T &value) {
        addNotEqualCondition<T>("", column, value);
    };

    template<typename T> void addNotEqualCondition(const std::string &table, const std::string &column, const T &value) {
        conditionsStack.push(new std::string(escapeAndChainKeys(table, column) + " != " + quote<T>(value)));
    };

    void andConditions();
    void orConditions();
protected:
    QueryWhere();
    QueryWhere(const PConnection connection);

    virtual ~QueryWhere();

    std::string buildQuerySegment();
private:
    void mergeConditions(const std::string &operation);
    QueryWhere(const QueryWhere &org);
};
}

#endif // _QUERY_WHERE_HPP_
