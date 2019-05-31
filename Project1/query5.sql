select count(*)
from
(select distinct Items.userID
from Users,Items
where Rating>1000 and Items.UserID=Users.UserID);
