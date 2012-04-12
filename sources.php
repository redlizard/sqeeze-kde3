<?php

$sources = file_get_contents($_SERVER["argv"][1]);
$packages = explode("\n\n", $sources);

foreach($packages as $package) {
	$headers = array();
	foreach(explode("\n", $package) as $line) {
		$pos = strpos($line, ":");
		if($pos === false) {
			continue;
		}
		$headers[trim(substr($line, 0, $pos))] = trim(substr($line, $pos + 1));
	}
	
	if(strpos($package, "kdelibs5") !== false) {
		continue;
	}
	
	if(!isset($headers["Section"]) || !isset($headers["Package"])) {
		continue;
	}
	
	if($headers["Section"] == "kde") {
		echo $headers["Package"] . "\n";
	} else if(strpos($headers["Package"], "kde") !== false) {
		echo '#' . $headers["Package"] . "\n";
	}
}
