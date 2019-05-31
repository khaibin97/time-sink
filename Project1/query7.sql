select count(*) 
from 
(select distinct Category 
from Category, Bids
where Bids.ItemID=Category.ItemID and Amount>100
);