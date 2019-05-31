select 
ItemID
from 
Items
where 
Currently=(select max(Currently) from Items);
