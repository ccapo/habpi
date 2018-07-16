#include "HABPi.h"

// GPS Constructor
GPS::GPS() {}

// GPS Destructor
GPS::~GPS() {}

// Begin GPS
bool GPS::begin() {
	gps_rec = new gpsmm("localhost", DEFAULT_GPSD_PORT);

  if (gps_rec->stream(WATCH_ENABLE|WATCH_JSON) == NULL) {
    Module::logger.error("GPSD is not running");
    return false;
  } else {
    Module::logger.info("GPSD is running");
    return true;
  }
}

// Update GPS
void GPS::update() {
	if (gps_rec == NULL) {
		Module::logger.error("GPS: gps_rec is NULL");
    return;
	}

	if (!gps_rec->waiting(GPS::GpsTimeout)) {
		Module::logger.error("GPS Timeout Error");
    return;
	}

  if ((data = gps_rec->read()) == NULL) {
    Module::logger.error("GPS Read Error");
    return;
  } else {
    storeData(data);

    std::cout << "nstats: " << static_cast<int>(Module::sensorMsg.gps_nsats);
    std::cout << ", status: " << static_cast<int>(Module::sensorMsg.gps_status);
    std::cout << ", mode: " << static_cast<int>(Module::sensorMsg.gps_mode) << std::endl;
    std::cout << "lat, lon: " << std::setprecision(8) << Module::sensorMsg.gps_lat << ", " << std::setprecision(8) << Module::sensorMsg.gps_lon;
    std::cout << ", alt: " << std::setprecision(8) << Module::sensorMsg.gps_alt;
    std::cout << ", dir: " << std::setprecision(8) << Module::sensorMsg.gps_dir;
    std::cout << ", gspd: " << std::setprecision(8) << Module::sensorMsg.gps_gspd;
    std::cout << ", vspd: " << std::setprecision(8) << Module::sensorMsg.gps_vspd << std::endl;
  }
}

/**
 * We should get libgps_dump_state() from the client library, but
 * scons has a bug; we can't get it to add -lgps to the link line,
 * apparently because it doesn't honor parse_flags on a Program()
 * build of a C++ file.
 */
void GPS::storeData(struct gps_data_t *collect) {
  // if (collect->set & TIME_SET) {
  //   std::cout << "TIME: " << collect->fix.time << std::endl;
  // }
  if (collect->set & LATLON_SET) {
    Module::sensorMsg.gps_lat = collect->fix.latitude;
    Module::sensorMsg.gps_lon = collect->fix.longitude;
	}
  if (collect->set & ALTITUDE_SET) {
    Module::sensorMsg.gps_alt = collect->fix.altitude;
  }
  if (collect->set & SPEED_SET) {
    Module::sensorMsg.gps_gspd = collect->fix.speed;
  }
  if (collect->set & TRACK_SET) {
    Module::sensorMsg.gps_dir = collect->fix.track;
  }
  if (collect->set & CLIMB_SET) {
    Module::sensorMsg.gps_vspd = collect->fix.climb;
  }
  if (collect->set & STATUS_SET) {
    Module::sensorMsg.gps_status = collect->status;
  }
  if (collect->set & MODE_SET) {
    Module::sensorMsg.gps_mode = collect->fix.mode;
  }
  if (collect->set & SATELLITE_SET) {
    Module::sensorMsg.gps_nsats = collect->satellites_used;
  }
}