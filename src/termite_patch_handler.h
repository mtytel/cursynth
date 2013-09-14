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

#pragma once
#ifndef TERMITE_PATCH_HANDLER_H
#define TERMITE_PATCH_HANDLER_H

namespace laf {

  class TermitePatchHandler {
    public:
      TermitePatchHandler();

      void queryPatches(const std::string& patch);
      void loadNext();
      void loadPrev();
      void loadFromFile(const std::string& file_name);
      void saveToFile();

    private:
      int patch_index_;
      std::vector<std::string> patches_;
      std::map<int, Control*> midi_learn_;
  };
} // namespace laf

#endif // TERMITE_PATCH_HANDLER
