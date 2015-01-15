/* Copyright 2013-2015 Matt Tytel
 *
 * cursynth is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cursynth is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cursynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cursynth.h"
#include <iostream>
#include <stdlib.h>
#include <getopt.h>

int main(int argc, char **argv) {
  unsigned buffer_size = mopo::DEFAULT_BUFFER_SIZE;
  unsigned sample_rate = mopo::DEFAULT_SAMPLE_RATE;

  int getopt_response = 0;
  int digit_optind = 0;

  while (getopt_response != -1) {
    static const struct option long_options[] = {
      {"sample-rate", required_argument, 0, 's'},
      {"buffer-size", required_argument, 0, 'b'},
      {"version", no_argument, 0, 'V'},
      {0, 0, 0, 0}
    };

    int option_index = 0;
    getopt_response = getopt_long(argc, argv, "s:b:V",
                                  long_options, &option_index);

    switch (getopt_response) {
      case 's':
        sample_rate = atoi(optarg);
        break;
      case 'b':
        buffer_size = atoi(optarg);
        break;
      case 'V':
        std::cout << "Cursynth " << VERSION << std::endl;
        exit(EXIT_SUCCESS);
        break;
      case -1:
        break;
      default:
        std::cout << std::endl << "Usage:" << std::endl
                  << "cursynth [--buffer-size OR -b preferred-buffer-size]"
                  << std::endl
                  << "         [--sample-rate OR -s preferred-sample-rate]"
                  << std::endl
                  << "         [--version OR -V]"
                  << std::endl;
        exit(EXIT_FAILURE);
        break;
    }
  }

  mopo::Cursynth cursynth;
  cursynth.start(sample_rate, buffer_size);

  return 0;
}
