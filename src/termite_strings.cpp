/* Copyright 2013 Little IO
 *
 * termite is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * termite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with termite.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "termite_strings.h"

namespace laf {

  const char* TermiteStrings::filter_strings_[] = {
    "low pass",
    "high pass",
    "band pass"
  };

  const char* TermiteStrings::legato_strings_[] = {
    "off",
    "on"
  };

  const char* TermiteStrings::portamento_strings_[] = {
    "off",
    "auto",
    "on"
  };

  const char* TermiteStrings::wave_strings_[] = {
    "sin",
    "triangle",
    "square",
    "down saw",
    "up saw",
    "three step",
    "four step",
    "eight step",
    "three pyramid",
    "five pyramid",
    "nine pyramid",
    "white noise",
  };
} // namespace laf
