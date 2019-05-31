DROP VIEW IF EXISTS w2;
DROP VIEW IF EXISTS q2;
CREATE VIEW w2 AS SELECT distinct store FROM hw2.temporaldata WHERE fuelprice > 4;
CREATE VIEW q2 AS SELECT distinct store FROM hw2.temporaldata WHERE unemploymentrate > 10;
SELECT q2.store FROM q2 EXCEPT SELECT w2.store FROM w2;

