#!/bin/sh
terminator -T "HTOP" -e htop -p hold &
sleep 3 
terminator -T "BROKER" -e "bash scriptDeployBroker.sh" -p hold &
sleep 3 
terminator -T "GAMECARD" -e "bash scriptDeployGamecard.sh" -p hold &
sleep 3 
terminator -T "GAMEBOY" -e "bash scriptDeployGameboy.sh" -p hold &
sleep 3 
terminator -T "TEAM1" -e "bash scriptDeployTeam.sh" -p hold &
sleep 3 
terminator -T "TEAM2" -e "bash scriptDeployTeam2.sh" -p hold &
sleep 3
cd /home/utnso/Descargas
gnome-mplayer pokemon.mp3

