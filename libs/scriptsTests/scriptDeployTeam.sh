#!/bin/sh
cp /home/utnso/workspace/tp-2020-1c-Cuarenteam/team/Configs/team1.config /home/utnso/workspace/tp-2020-1c-Cuarenteam/team/Default
cd /home/utnso/workspace/tp-2020-1c-Cuarenteam/team/Default
rm team.config
mv team1.config team.config
./team
