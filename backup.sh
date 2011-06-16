#!/bin/bash
cd www
../drush/drush sql-dump --ordered-dump --result-file=../db.sql
git add www/sites/default/files/
git add db.sql 
git commit -m "backup"

