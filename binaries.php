<?php

$packages = explode("\n\n", file_get_contents($_SERVER["argv"][1]));
$selectedList = explode("\n", file_get_contents($_SERVER["argv"][2]));

$selected = array();
foreach($selectedList as $package) {
	$package = trim($package);
	if($package == "") {
		continue;
	}
	$selected[$package] = true;
}

foreach($packages as $package) {
	$headers = array();
	foreach(explode("\n", $package) as $line) {
		$pos = strpos($line, ":");
		if($pos === false) {
			continue;
		}
		$headers[trim(substr($line, 0, $pos))] = trim(substr($line, $pos + 1));
	}
	
	if(!isset($headers["Package"]) || !isset($headers["Binary"])) {
		continue;
	}
	
	if(!isset($selected[$headers["Package"]])) {
		continue;
	}
	
	foreach(explode(",", $headers["Binary"]) as $binary) {
		echo trim($binary) . "\n";
	}
}
