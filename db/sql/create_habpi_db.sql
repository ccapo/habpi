--
-- File generated with SQLiteStudio v3.1.1 on Wed Mar 22 02:07:12 2017
--
-- Text encoding used: UTF-8
--
PRAGMA foreign_keys = off;
BEGIN TRANSACTION;

-- Table: message
DROP TABLE IF EXISTS message;
CREATE TABLE message (id INTEGER PRIMARY KEY AUTOINCREMENT, message_type_id INTEGER REFERENCES message_type (id), sensor_type_id INTEGER REFERENCES sensor_type (id), message STRING NOT NULL, created_at DATETIME);

-- Table: message_type
DROP TABLE IF EXISTS message_type;
CREATE TABLE message_type (id INTEGER PRIMARY KEY AUTOINCREMENT, name STRING NOT NULL, description STRING);
INSERT INTO message_type (id, name, description) VALUES (1, 'SYS', 'System Status');
INSERT INTO message_type (id, name, description) VALUES (2, 'LAT', 'Latitude in Degrees');
INSERT INTO message_type (id, name, description) VALUES (3, 'LON', 'Longitude in Degress');
INSERT INTO message_type (id, name, description) VALUES (4, 'ALT', 'Altitude in m');
INSERT INTO message_type (id, name, description) VALUES (5, 'TEMP', 'Temperature in C');
INSERT INTO message_type (id, name, description) VALUES (6, 'PRES', 'Atmospheric Pressure in kPa');
INSERT INTO message_type (id, name, description) VALUES (7, 'AX', 'Acceleration X Component in m/s^2');
INSERT INTO message_type (id, name, description) VALUES (8, 'AY', 'Acceleration Y Component in m/s^2');
INSERT INTO message_type (id, name, description) VALUES (9, 'AZ', 'Acceleration Z Component in m/s^2');
INSERT INTO message_type (id, name, description) VALUES (10, 'A_PITCH', 'Acceleration Pitch Angle in rad [-pi/2:pi/2]');
INSERT INTO message_type (id, name, description) VALUES (11, 'A_ROLL', 'Acceleration Roll Angle in rad [-pi:pi]');
INSERT INTO message_type (id, name, description) VALUES (12, 'A_MAG', 'Acceleration Magnitude in m/s^2');
INSERT INTO message_type (id, name, description) VALUES (13, 'A_INC', 'Acceleration Inclination in rad [0:2*pi]');
INSERT INTO message_type (id, name, description) VALUES (14, 'A_ORIENT', 'Acceleration Orientation');
INSERT INTO message_type (id, name, description) VALUES (15, 'GYROX', 'Gyroscope X Component');
INSERT INTO message_type (id, name, description) VALUES (16, 'GYROY', 'Gyroscope Y Component');
INSERT INTO message_type (id, name, description) VALUES (17, 'GYROZ', 'Gyroscope Z Component');
INSERT INTO message_type (id, name, description) VALUES (18, 'GYRO_PITCH', 'Gyroscope Pitch Angle in rad [-pi/2:pi/2]');
INSERT INTO message_type (id, name, description) VALUES (19, 'GYRO_ROLL', 'Gyroscope Roll Angle in rad [-pi:pi]');
INSERT INTO message_type (id, name, description) VALUES (20, 'GYRO_YAW', 'Gyroscope Yaw Angle in rad [0:2*pi]');
INSERT INTO message_type (id, name, description) VALUES (21, 'GYRO_RATE', 'Gyroscope Rate of Change in rad/s');
INSERT INTO message_type (id, name, description) VALUES (22, 'MAG_HEADING', 'Magnetic Heading Angle in rad [0:2*pi]');
INSERT INTO message_type (id, name, description) VALUES (23, 'MAG_BEARING', 'Magnetic Bearing Name');
INSERT INTO message_type (id, name, description) VALUES (24, 'QW', 'Quarternion W Component');
INSERT INTO message_type (id, name, description) VALUES (25, 'QX', 'Quarternion W Component');
INSERT INTO message_type (id, name, description) VALUES (26, 'QY', 'Quarternion W Component');
INSERT INTO message_type (id, name, description) VALUES (27, 'QZ', 'Quarternion W Component');
INSERT INTO message_type (id, name, description) VALUES (28, 'EULER_HEADING', 'Euler Heading Angle in rad [0:2*pi]');
INSERT INTO message_type (id, name, description) VALUES (29, 'EULER_ROLL', 'Euler Roll Angle in rad [-pi:pi]');
INSERT INTO message_type (id, name, description) VALUES (30, 'EULER_PITCH', 'Euler Pitch Angle in rad [-pi/2:pi/2]');
INSERT INTO message_type (id, name, description) VALUES (31, 'BAT', 'Battery Status');
INSERT INTO message_type (id, name, description) VALUES (32, 'CAM', 'Camera Status');

-- Table: sensor_type
DROP TABLE IF EXISTS sensor_type;
CREATE TABLE sensor_type (id INTEGER PRIMARY KEY, name STRING NOT NULL, description STRING);
INSERT INTO sensor_type (id, name, description) VALUES (1, 'GPS', 'GPS sensor');
INSERT INTO sensor_type (id, name, description) VALUES (2, 'BMP180', 'BMP180 Pressure and Temperature sensor');
INSERT INTO sensor_type (id, name, description) VALUES (3, 'MPL3115A2', 'MPL3115A2 Pressure and Temperature sensor');
INSERT INTO sensor_type (id, name, description) VALUES (4, 'BNO55', 'Absolute Orientation (Accelerometer, Gyroscope & Compass) and Temperature sensor');
INSERT INTO sensor_type (id, name, description) VALUES (5, 'Battery', 'Battery sensor');
INSERT INTO sensor_type (id, name, description) VALUES (6, 'Camera', 'Raspberry Pi Camera');

-- Trigger: timestamp
DROP TRIGGER IF EXISTS timestamp;
CREATE TRIGGER timestamp AFTER INSERT ON message FOR EACH ROW BEGIN UPDATE message SET created_at = CAST (strftime('%s', 'now') || '.' || substr(strftime('%f', 'now'), 4, 3) AS REAL) WHERE id = NEW.rowid; END;

COMMIT TRANSACTION;
PRAGMA foreign_keys = on;
