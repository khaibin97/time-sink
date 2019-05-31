select count(*)
from
(
select distinct Items.UserID
from Items, Bids
where Items.UserID=Bids.UserID
);
