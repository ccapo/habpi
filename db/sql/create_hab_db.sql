--
-- File generated with SQLiteStudio v3.1.1 on Mon Feb 20 16:43:39 2017
--
-- Text encoding used: UTF-8
--
PRAGMA foreign_keys = off;
BEGIN TRANSACTION;

-- Table: message
DROP TABLE IF EXISTS message;

CREATE TABLE message (
    id              INTEGER  PRIMARY KEY AUTOINCREMENT,
    message_type_id INTEGER  REFERENCES message_type (id),
    message         STRING   NOT NULL,
    created_at      DATETIME
);


-- Table: message_type
DROP TABLE IF EXISTS message_type;

CREATE TABLE message_type (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        STRING  NOT NULL,
    description STRING
);

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             1,
                             'SYS',
                             'System Status'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             2,
                             'GPS_LAT',
                             'GPS Latitude in Degrees'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             3,
                             'GPS_LON',
                             'GPS Longitude in Degress'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             4,
                             'GPS_ALT',
                             'GPS Altitude in km'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             5,
                             'TEMP',
                             'Temperature in C'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             6,
                             'BARO',
                             'Atmospheric Pressure in hPa'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             7,
                             'BARO_ALT',
                             'Barometric Altitude in km'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             8,
                             'MAGX',
                             'Magnetic Field X Component in microTesla'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             9,
                             'MAGY',
                             'Magnetic Field Y Component in microTesla'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             10,
                             'MAGZ',
                             'Magnetic Field Z Component in microTesla'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             11,
                             'MAG_PITCH',
                             'Magnetic Pitch Angle in Degrees [-90:90]'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             12,
                             'MAG_ROLL',
                             'Magnetic Roll Angle in Degrees [-180:180]'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             13,
                             'MAG_HEADING',
                             'Magnetic Heading Angle in Degrees [0:360]'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             14,
                             'BAT',
                             'Battery Status'
                         );

INSERT INTO message_type (
                             id,
                             name,
                             description
                         )
                         VALUES (
                             15,
                             'CAM',
                             'Camera Status'
                         );


-- Trigger: timestamp
DROP TRIGGER IF EXISTS timestamp;
CREATE TRIGGER timestamp
         AFTER INSERT
            ON message
      FOR EACH ROW
BEGIN
    UPDATE message
       SET created_at = strftime('%Y-%m-%d %H:%M:%f', 'now') 
     WHERE id = NEW.rowid;
END;


COMMIT TRANSACTION;
PRAGMA foreign_keys = on;
