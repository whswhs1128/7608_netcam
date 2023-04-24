#dir="./CUS_SZ_MB_A1_IMX222 ./CUS_JA_SENSOR_IMX222 ./CUS_SENSOR_AR0130 ./CUS_JA_SENSOR_IMX222_WIFI ./CUS_HZD_OJTV_V10_OV2710 ./CUS_SENSOR_SC1045 ./CUS_JA_SENSOR_OV2710 ./CUS_SENSOR_SC1035 ./CUS_HZD_OJTV_V10_JXH42 ./CUS_HSL_SENSOR_JXH42"

dir=$(ls -l | awk '/^d/ {print $NF}')


file_name=gk_system_cfg.cjson
#old_valu="\"enable\":\t1"

old_valu="languageTypeProperty\":\t{\"type\":\"S32\",\"mode\":\"rw\",\"min\":0"
new_valu="languageTypeProperty\":\t{\"type\":\"S32\",\"mode\":\"rw\",\"min\":-1"

do_help()
{
    echo "$0 old_string new_string"
    exit
}

if [ ! $old_valu -a ! $new_valu ]; then
if [ $# -eq 0 ]; then
    do_help
fi
old_valu=$1
new_valu=$2
fi
echo "old_valu:$old_valu"
echo "new_valu:$new_valu"

for tmpdir in $dir; do
    echo "dir--:$tmpdir"
    find $tmpdir -name $file_name |xargs perl -pi -e 's|'${old_valu}'|'${new_valu}'|g'
done

