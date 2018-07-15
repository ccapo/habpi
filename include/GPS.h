/**
 * Wrapper for libgpsmm
 *
 * Written By: Chris Capobianco
 * Date: 2018-06-05
 */
#pragma once

#include <iostream>
#include <stdint.h>
#include <iomanip>
#include "libgpsmm.h"

class GPS {
public:
	// GPS Constructor
	GPS();

	// GPS Constructor
	~GPS();

	// Begin GPS
	bool begin();

	// Update GPS
	void update();

	// Store GPS Data
	void storeData(struct gps_data_t *collect);

	// Static Constants
	static const int GpsTimeout = 20000; //2000000

private:
	gpsmm *gps_rec;
	struct gps_data_t *data;
};