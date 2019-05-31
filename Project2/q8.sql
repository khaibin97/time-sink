Drop view if exists norm cascade;

create view norm as 
select dept, (WeeklySales/size) as normsales 
from hw2.Sales,hw2.Stores 
where Stores.store=Sales.store;

create view ordered_q8 as 
select ROW_NUMBER() over (order by null) as row_num, * 
from (select dept,sum(normsales) from norm group by dept order by sum desc) as foo;


select dept,sum as normsales from ordered_q8 where row_num<=10;

