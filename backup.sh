#!/bin/bash
cd www
../drush/drush sql-dump --ordered-dump --result-file=../db.sql
