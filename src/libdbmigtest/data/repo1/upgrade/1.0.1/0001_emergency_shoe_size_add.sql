-- Stakeholders are demanding UK shoe sizes right away.. do this tactically.
-- Adding this as an emergency patch release!

alter table human
add uk_shoe_size int null;

--//@UNDO

alter table human drop uk_shoe_size;

