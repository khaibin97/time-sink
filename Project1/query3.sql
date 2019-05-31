select count(*)
from
(
    select ItemID, count(*) as cat
    from Category
    group by ItemID
    having cat=4
);
