Drop view if exists q6 cascade;
create view q6 as SELECT sales.store, sales.weekdate, weeklysales, temperature, fuelprice, cpi, unemploymentrate
from hw2.sales join hw2.temporaldata on sales.store=temporaldata.store and sales.weekdate=temporaldata.weekdate;

create view correlation as select corr(weeklysales,temperature) as temperature, corr(weeklysales,fuelprice) as fuelprice, corr(weeklysales, cpi) as cpi, corr(weeklysales,unemploymentrate) as unemploymentrate from q6;

select 'Temperature' as attribute, case when temperature > 0 then '+' else '-' end as corr_sign, temperature as correlation from correlation union all
select 'FuelPrice' as attribute, case when fuelprice > 0 then '+' else '-' end as corr_sign, fuelprice as correlation from correlation union all
select 'CPI' as attribute, case when cpi > 0 then '+' else '-' end as corr_sign, cpi as correlation from correlation union all
select 'UnemploymentRate' as attribute, case when unemploymentrate > 0 then '+' else '-' end, unemploymentrate as correlation from correlation;

