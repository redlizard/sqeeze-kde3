<?php

$mirror = "http://archive.debian.org/debian/";
$packageName = $_SERVER["argv"][1];

$sources = parseControl(file_get_contents("Sources"));
$directory = null;
$files = array();
$version = null;
foreach($sources as $source) {
	if($source["package"] != $packageName) {
		continue;
	}
	
	$version = $source["version"];
	$directory = $source["directory"];
	foreach(explode("\n", $source["files"]) as $file) {
		$index = 0;
		foreach(explode(" ", $file) as $part) {
			if(trim($part) == "") {
				continue;
			}
			if($index++ == 2) {
				$files[] = $part;
			}
		}
	}
}
if($directory === null) {
	die("Package $packageName not found\n");
}

$cwd = getcwd();
$tempdir = tempnam(".", "temp-");
unlink($tempdir);
mkdir($tempdir);
chdir($tempdir);

$dsc = null;
foreach($files as $file) {
	echo "Downloading $file...\n";
	file_put_contents($file, file_get_contents($mirror . $directory . "/" . $file));
	
	$pos = strrpos($file, ".");
	if($pos === false) {
		continue;
	}
	if(strtolower(substr($file, $pos + 1)) == "dsc") {
		$dsc = $file;
	}
}
if($dsc === null) {
	chdir($cwd);
	`rm -rf $tempdir`;
	die("No description file found\n");
}

`dpkg-source -x $dsc`;

$pos = strpos($version, "-");
if($pos !== false) {
	$version = substr($version, 0, $pos);
}
$dirVersion = $version;
$pos = strpos($dirVersion, ":");
if($pos !== false) {
	$dirVersion = substr($dirVersion, $pos + 1);
}

$dirname = "$packageName-$dirVersion";
$targetDirectory = "packages/kde3-" . $dirname;

if(!is_dir($dirname)) {
	chdir($cwd);
	`rm -rf $tempdir`;
	die("Package directory not found\n");
}

chdir($cwd);

rename("$tempdir/$dirname", $targetDirectory);
`rm -rf $tempdir`;



$targetName = "kde3-$packageName";
$targetVersion = "$version-squeeze1";
$date = date("r");

$entry = <<<LOG
$targetName ($targetVersion) squeeze; urgency=medium

  * Ported to squeeze

 -- Ruud Koolen <ruudkoolen@ruudkoolen.nl>  $date


LOG;

file_put_contents("$targetDirectory/debian/changelog", $entry . file_get_contents("$targetDirectory/debian/changelog"));

$control = parseControl(file_get_contents("$targetDirectory/debian/control"));
$newControl = array();

$source = array_shift($control);
$source["source"] = "kde3-" . $source["source"];
$source["maintainer"] = "Ruud Koolen <ruudkoolen@ruudkoolen.nl>";
foreach(array("build-depends", "build-depends-indep") as $header) {
	if(isset($source[$header])) {
		$source[$header] = replacePackageNames($source[$header]);
	}
}
$newControl[] = $source;

$binaries = array();
foreach($control as $package) {
	$binaries[$package["package"]] = "kde3-" . $package["package"];
	
	$package["package"] = "kde3-" . $package["package"];
	
	foreach(array("conflicts", "breaks", "replaces") as $header) {
		if(isset($package[$header])) {
			$package[$header] = removeOld($package[$header]);
			if(trim($package[$header]) == "") {
				unset($package[$header]);
			}
		}
	}
	
	foreach(array("depends", "recommends", "suggests", "enhances", "pre-depends", "conflicts", "breaks", "replaces") as $header) {
		if(isset($package[$header])) {
			$package[$header] = replacePackageNames($package[$header]);
		}
	}
	
	$newControl[] = $package;
}

file_put_contents("$targetDirectory/debian/control", renderControl($newControl));

$dir = opendir("$targetDirectory/debian");
while(($file = readdir($dir)) !== false) {
	if($file == "." || $file == "..") {
		continue;
	}
	$pos = strpos($file, ".");
	if($pos === false) {
		continue;
	}
	$prefix = substr($file, 0, $pos);
	if(isset($binaries[$prefix])) {
		$newname = $targetDirectory . "/debian/" . $binaries[$prefix] . substr($file, $pos);
		rename("$targetDirectory/debian/$file", $newname);
		
		$extension = substr($file, strpos($file, ".") + 1);
		if($extension == "install" || $extension == "manpages") {
			$install = array();
			foreach(explode("\n", file_get_contents($newname)) as $line) {
				$install[] = str_replace("usr/", "opt/kde3/", $line);
			}
			file_put_contents($newname, implode("\n", $install));
		} else if($extension == "menu") {
			file_put_contents($newname, str_replace('"/usr/', '"/opt/kde3/', file_get_contents($newname)));
		} else if($extension == "links") {
			$links = array();
			$found = false;
			foreach(explode("\n", file_get_contents($newname)) as $line) {
				if((trim($line) != "") && (substr($line, 0, 5) != "/usr/")) {
					$found = true;
				}
				$links[] = str_replace("usr/", "opt/kde3/", $line);
			}
			file_put_contents($newname, implode("\n", $links));
			if($found) {
				echo "Notice: $newname\n";
			}
		} else if(in_array($extension, array("docs", "lintian", "mime", "pam", "presubj", "README.Debian")) || (substr($extension, 0, 8) == "doc-base")) {
			// Safe, skip
		} else {
			echo "Notice: $newname\n";
		}
	}
}
closedir($dir);

$extrarules = <<<RULES

DEB_CONFIGURE_PREFIX := /opt/kde3

DEB_SHLIBDEPS_INCLUDE := /opt/kde3/lib

$(patsubst %,binary-install/%,$(DEB_ALL_PACKAGES)) ::
	test ! -d debian/$(cdbs_curpkg)/usr/share/man || mkdir -p debian/$(cdbs_curpkg)/opt/kde3/share
	test ! -d debian/$(cdbs_curpkg)/usr/share/man || cp -a debian/$(cdbs_curpkg)/usr/share/man debian/$(cdbs_curpkg)/opt/kde3/share
	test ! -d debian/$(cdbs_curpkg)/usr/share/man || rm -r debian/$(cdbs_curpkg)/usr/share/man
	test ! -d debian/$(cdbs_curpkg)/opt/kde3/share/doc || mkdir -p debian/$(cdbs_curpkg)/usr/share/doc
	test ! -d debian/$(cdbs_curpkg)/opt/kde3/share/doc || cp -a debian/$(cdbs_curpkg)/opt/kde3/share/doc debian/$(cdbs_curpkg)/usr/share
	test ! -d debian/$(cdbs_curpkg)/opt/kde3/share/doc || rm -r debian/$(cdbs_curpkg)/opt/kde3/share/doc
	test ! -d debian/$(cdbs_curpkg)/usr/share/doc || for f in debian/$(cdbs_curpkg)/usr/share/doc/*; do \
		if ! echo $$f | grep ^debian/$(cdbs_curpkg)/usr/share/doc/kde3-  >/dev/null; then \
			mv $$f debian/$(cdbs_curpkg)/usr/share/doc/kde3-`basename $$f`; \
		fi; \
	done


RULES;

file_put_contents("$targetDirectory/debian/rules", file_get_contents("$targetDirectory/debian/rules") . $extrarules);


function parseControl($control)
{
	$control = str_replace("\t", " ", str_replace("\r", "", $control));
	
	$logicalLines = array();
	$currentLine = null;
	foreach(explode("\n", $control) as $line) {
		if(substr($line, 0, 1) == "#") {
			continue;
		}
		if((substr($line, 0, 1) != " ") || (trim($line) == "")) {
			if($currentLine !== null) {
				$logicalLines[] = trim($currentLine);
			}
			$currentLine = "";
		}
		$currentLine .= $line . "\n";
	}
	$logicalLines[] = trim($currentLine);
	
	$paragraphs = array();
	$paragraph = array();
	foreach($logicalLines as $line) {
		if(trim($line) == "") {
			if(count($paragraph) > 0) {
				$paragraphs[] = $paragraph;
			}
			$paragraph = array();
		} else {
			$pos = strpos($line, ":");
			if($pos === false) {
				die("Syntax error in control file\n");
			}
			$key = strtolower(trim(substr($line, 0, $pos)));
			$value = trim(substr($line, $pos + 1));
			if(isset($paragraph[$key])) {
				die("Double key '$key' in control file\n");
			}
			$paragraph[$key] = $value;
		}
	}
	if(count($paragraph) > 0) {
		$paragraphs[] = $paragraph;
	}
	return $paragraphs;
}

function renderControl($paragraphs)
{
	$output = "";
	foreach($paragraphs as $paragraph) {
		foreach($paragraph as $key=>$value) {
			$parts = array();
			foreach(explode("-", $key) as $part) {
				$parts[] = strtoupper($part[0]) . substr($part, 1);
			}
			$ckey = implode("-", $parts);
			$output .= "$ckey: $value\n";
		}
		$output .= "\n";
	}
	return $output;
}

function replacePackageNames($dependencies)
{
	$binaries = array();
	foreach(explode("\n", file_get_contents("kde-binaries")) as $package) {
		if($package == "") {
			continue;
		}
		$binaries[$package] = true;
	}
	
	$output = array();
	foreach(explode(",", $dependencies) as $dependency) {
		$out = array();
		foreach(explode("|", $dependency) as $dep) {
			$dep = str_replace("\n", " ", trim($dep));
			$pos = strpos($dep, " ");
			if($pos === false) {
				$name = $dep;
				$postfix = "";
			} else {
				$name = substr($dep, 0, $pos);
				$postfix = substr($dep, $pos);
			}
			if(isset($binaries[$name])) {
				$out[] = "kde3-$name$postfix";
			} else {
				$out[] = "$name$postfix";
			}
		}
		$output[] = implode(" | ", $out);
	}
	return implode(", ", $output);
}

function removeOld($dependencies)
{
	$output = array();
	foreach(explode(",", $dependencies) as $dependency) {
		if(strpos($dependency, "<") !== false) {
			continue;
		}
		$output[] = trim($dependency);
	}
	return implode(", ", $output);
}
