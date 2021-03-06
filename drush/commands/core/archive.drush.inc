<?php
// $Id$

/**
 * @file
 *   An early implementation of Site Archive dump/restore. See
 *   http://groups.drupal.org/site-archive-format.
 */

function archive_drush_command() {
  $items['archive-dump'] = array(
    'description' => 'Backup your code, files, and database into a single file.',
    'arguments' => array(
      'targets' => 'Optional. Site specifications, delimited by commas. Typically, list subdirectory name(s) under /sites.',
    ),
    'options' => array(
      'description' => 'Describe the archive contents.',
      'tags' => 'Add tags to the archive manifest. Delimit multiple by commas.',
      'destination' => 'Specify where the archive should be stored. Include filename but omit the .gz suffix.',
      'generator' => 'The generator name to store in the MANIFEST file. The default is "Drush archive-dump".',
      'generatorversion' => 'The generator version number to store in the MANIFEST file. The default is ' . DRUSH_VERSION . '.',
      'pipe' => 'Only print the destination of the archive. Useful for scripts that don\'t pass --destination.',
    ),
    'examples' => array(
      'drush archive-dump default,example.com,foo.com' => 'Write an archive containing 3 sites in it.',
      'drush archive-dump @sites' => 'Save archive containing all sites in a multi-site.',
      'drush archive-dump default --destination=/backups/mysite.tar' => 'Save archive to custom location.',
    ),
    'core' => array('6+'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_SITE,
  );
  return $items;
}

/**
 * Command callback. Generate site archive file.
 */
function drush_archive_dump($sites_subdirs = '@self') {
  $aliases = drush_sitealias_resolve_sitespecs(explode(',', $sites_subdirs));
  foreach ($aliases as $key => $alias) {
    $full[$key] = $alias += sitealias_get_databases_from_record($alias);
  }

  // User did not pass a specific value for --destination. Make one.
  if (!$destination = drush_get_option('destination')) {
    $date = gmdate('Ymd_his');
    $first = current($full);
    $prefix = count($sites_subdirs) > 1 ? 'multiple_sites' : $first['default']['default']['database'];
    $file = "$prefix.$date.tar";
    //
    drush_include_engine('version_control', 'backup');
    $backup = new drush_pm_version_control_backup();
    // TODO: this standard drush pattern leads to a slightly obtuse directory structure.
    $backup_dir = $backup->prepare_backup_dir('archive-dump');
    if (empty($backup_dir)) {
      $backup_dir = "/tmp";
    }
    $destination = "$backup_dir/$file";
  }
  else {
    $destination = realpath($destination);
  }

  $docroot_path = realpath(drush_get_context('DRUSH_DRUPAL_ROOT'));
  $docroot = basename($docroot_path);
  $workdir = dirname($docroot_path);
  // Archive Drupal core, excluding sites dir.
  drush_shell_cd_and_exec($workdir, "tar --exclude '{$docroot}/sites' --dereference -cf %s %s", $destination, $docroot);
  // Add sites/all to the same archive.
  drush_shell_cd_and_exec($workdir, "tar --dereference -rf %s %s", $destination, "{$docroot}/sites/all");

  // Dump the database(s) for each site and add to the archive.
  foreach ($full as $key => $alias) {
    foreach ($alias['databases'] as $dbkey => $target) {
      $tmp = drush_tempdir();
      // Use a subdirectory name matching the docroot name.
      drush_mkdir("{$tmp}/{$docroot}");
      $result_file = "{$tmp}/{$target['default']['database']}.sql";
      drush_set_option('result-file', $result_file);
      $table_selection = drush_sql_get_table_selection();
      list($dump_exec, $dump_file) = drush_sql_build_dump_command($table_selection, $target['default']);
      drush_shell_exec($dump_exec);
      drush_shell_cd_and_exec($tmp, 'tar --dereference -rf %s %s', $destination, basename($result_file));
    }
  }

  // Build a manifest file AND add sites/$subdir to archive as we go.
  $platform = array(
    'datestamp' => time(),
    'formatversion' => '.1',
    'generator' => drush_get_option('generator', 'Drush archive-dump'),
    'generatorversion' => drush_get_option('generatorversion', DRUSH_VERSION),
    'description' => drush_get_option('description', ''),
    'tags' => explode(',', drush_get_option('tags', '')),
  );
  $contents = drush_export_info($platform);

  $i=0;
  foreach ($full as $key => $alias) {
    $vget = drush_invoke_sitealias_args($alias, 'vget', array('site_name'), array());
    $name = $vget['output'];
    $pos1 = strpos($name, '"');
    $name = substr($name, $pos1);
    $status = drush_invoke_sitealias_args($alias, 'core-status', array(), array());

    // Add the site specific directory to archive.
    if (!empty($status['object']['%paths']['%site'])) {
      drush_shell_cd_and_exec($workdir, "tar --dereference -rf %s %s", $destination,  "{$docroot}/sites/" . basename($status['object']['%paths']['%site']));
    }

    $site = array(
      'name' => str_replace(array('"', "\n"), '', $name),
      'docroot' => DRUPAL_ROOT,
      'sitedir' => @$status['object']['%paths']['%site'],
      'files' => array(
        'public' => @$status['object']['%paths']['%files'],
        'private' => @$status['object']['%paths']['%private'],
      ),
    );
    // Add info for each DB connection (usually only 1);
    foreach ($alias['databases'] as $dbkey => $target) {
      $site['database'][$dbkey]['file'] = './' . $target['default']['database'] . '.sql';
      $site['database'][$dbkey]['driver'] = $target['default']['driver'];
    }
    $info['sites'][$i] = $site;
    $contents .= "\n" . drush_export_info($info, TRUE);
    unset($info);
    $i++;
  }
  file_put_contents("{$tmp}/MANIFEST.info", $contents);

  // Add manifest to archive.
  drush_shell_cd_and_exec($tmp, 'tar --dereference -rf %s %s', $destination, "MANIFEST.info");

  // Compress the archive
  drush_shell_exec("gzip -f %s", $destination);

  drush_log(dt('Archive saved to !dest.gz', array('!dest' => $destination)), 'ok');
  drush_print_pipe($destination);
  return $destination;
}
