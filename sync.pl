#!/bin/perl

$dev_target = `mount | grep sdb1`;

if(length($dev_target) == 0){
	print "You forgot to plug the disk in.\n";
	exit -1;
}

if($dev_target =~ /rw/){
	@target_slices = split(/ +/, $dev_target);
	if(-e @target_slices[2] . '/.ds-sync'){
		`cp -r /home/dousha/workbench/ns @target_slices[2]`;
		print "Done.\n";
		exit 0;
	}else{
		print "It's not your disk!\n";
		exit -1;
	}
}else{
	print "Disk is read-only\n";
	exit -1;
}
