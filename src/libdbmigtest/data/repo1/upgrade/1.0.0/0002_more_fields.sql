-- Add some more basic fields to human

alter table human add dob timestamp with time zone null;

alter table human
add height_in_cm int null;

alter table human
add weight_in_kg int null;

--//@UNDO

alter table human drop weight_in_kg;
alter table human drop height_in_cm;
alter table human drop dob;
