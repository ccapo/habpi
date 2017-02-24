SELECT
    m.id,
    mt.name AS 'type',
    m.message,
    m.created_at
FROM
    message AS m
JOIN
    message_type AS mt
ON
    m.message_type_id = mt.id
--WHERE
--    mt.id in (5,6,7)
--ORDER BY
--    created_at ASC;