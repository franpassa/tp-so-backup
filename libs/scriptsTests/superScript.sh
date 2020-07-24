#!/bin/sh
bash scriptLibs.sh
terminator -T "TEAM1" -e "bash scriptTeam.sh" &
terminator -T "BROKER" -e "bash scriptBroker1.sh" &
terminator -T "GAMEBOY" -e "bash scriptGameboy1.sh" &
terminator -T "GAMECARD" -e "bash scriptGameCard1.sh" &
terminator -T "GAMECARD" -e "bash scriptLibs.sh" 
