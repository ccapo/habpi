.mode csv message;
.output db/temp.dat
SELECT m.created_at,m.message FROM message AS m JOIN message_type AS mt ON m.message_type_id = mt.id WHERE mt.name = "TEMP";
.output db/baro.dat
SELECT m.created_at,m.message FROM message AS m JOIN message_type AS mt ON m.message_type_id = mt.id WHERE mt.name = "BARO";
.output db/baro_alt.dat
SELECT m.created_at,m.message FROM message AS m JOIN message_type AS mt ON m.message_type_id = mt.id WHERE mt.name = "BARO_ALT";
.quit
