#!/bin/sh

echo "sensordetect cfg"
sensor=`readlink /tmp/sensor_hw_ch0.bin  | cut -c 24-29`
echo "sensor: $sensor"
if [ -h "/opt/custom/cfg/gk_video_cfg.cjson" ]; then
	echo "check cfg matches"
	sensorcfg=`readlink /opt/custom/cfg/gk_video_cfg.cjson | cut -c 30-35`
	if [ "$sensor"x != "$sensorcfg"x ]; then
		echo "not $sensor $sensorcfg"
		rm /opt/custom/cfg/gk_video_cfg.cjson
		ln -s /opt/custom/cfg/gk_video_cfg_$sensor".cjson" /opt/custom/cfg/gk_video_cfg.cjson
	fi

else
	echo "use defcfg"
	cp /usr/local/defcfg/*.cjson /opt/custom/cfg/
	rm /opt/custom/cfg/gk_video_cfg.cjson
	ln -s /opt/custom/cfg/gk_video_cfg_$sensor".cjson" /opt/custom/cfg/gk_video_cfg.cjson
fi
