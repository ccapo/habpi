/*
 * Copyright (C) 2010 Eric S. Raymond.
 *
 * This software is distributed under a BSD-style license. See the
 * file "COPYING" in the top-level directory of the distribution for details.
 *
 */

/* This simple program shows the basic functionality of the C++ wrapper class */
#include <iostream>

#include "libgpsmm.h"
#include <stdint.h>
#include <iomanip>

using namespace std;

typedef struct {
  uint8_t type = 0, nstats = 0, status = 0, mode = 0;
  float lat = 0.0f, lon = 0.0f, alt = 0.0f, spd = 0.0f, dir = 0.0f, climb = 0.0f;
} gps_msg_t;

gps_msg_t gps_msg;

/*
 * We should get libgps_dump_state() from the client library, but
 * scons has a bug; we can't get it to add -lgps to the link line,
 * apparently because it doesn't honor parse_flags on a Program()
 * build of a C++ file.
 */
static void libgps_dump_state(struct gps_data_t *collect) {
  //if (collect->set & TIME_SET)
	//  (void)fprintf(stdout, "TIME: %lf\n", collect->fix.time);
  if (collect->set & LATLON_SET) {
    gps_msg.lat = collect->fix.latitude;
    gps_msg.lon = collect->fix.longitude;
	}
  if (collect->set & ALTITUDE_SET) {
    gps_msg.alt = collect->fix.altitude;
  }
  if (collect->set & SPEED_SET) {
    gps_msg.spd = collect->fix.speed;
  }
  if (collect->set & TRACK_SET) {
    gps_msg.dir = collect->fix.track;
  }
  if (collect->set & CLIMB_SET) {
    gps_msg.climb = collect->fix.climb;
  }
  if (collect->set & STATUS_SET) {
    gps_msg.status = collect->status;
  }
  if (collect->set & MODE_SET) {
    gps_msg.mode = collect->fix.mode;
  }
  if (collect->set & SATELLITE_SET) {
    gps_msg.nstats = collect->satellites_used;
  }
}


int main(void) {
  gpsmm gps_rec("localhost", DEFAULT_GPSD_PORT);

  if (gps_rec.stream(WATCH_ENABLE|WATCH_JSON) == NULL) {
    cerr << "No GPSD running.\n";
    return 1;
  }
  
  gps_msg.type = 1;

  for (;;) {
    struct gps_data_t *newdata;

    if (!gps_rec.waiting(5000000))
      continue;

    if ((newdata = gps_rec.read()) == NULL) {
      cerr << "Read error.\n";
      return 1;
    } else {
      libgps_dump_state(newdata);

      cout << "type: " << static_cast<int>(gps_msg.type);
      cout << ", nstats: " << static_cast<int>(gps_msg.nstats);
      cout << ", status: " << static_cast<int>(gps_msg.status);
      cout << ", mode: " << static_cast<int>(gps_msg.mode) << endl;
      cout << "lat, lon: " << std::setprecision(8) << gps_msg.lat << ", " << std::setprecision(8) << gps_msg.lon;
      cout << ", alt: " << std::setprecision(8) << gps_msg.alt;
      cout << ", spd: " << std::setprecision(8) << gps_msg.spd;
      cout << ", dir: " << std::setprecision(8) << gps_msg.dir;
      cout << ", climb: " << std::setprecision(8) << gps_msg.climb << endl;
      cout << endl;
    }
  }

  cout << "Exiting\n";
  return 0;
}

